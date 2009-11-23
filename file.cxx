//-< FILE.CXX >------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 21-Jan-99    K.A. Knizhnik  * GARRET *
//                                                                   *  /  \  *
// Port to AIX done by XaoYao 13-Apr-97                              *        *
// Also thanks to Wolfgang Hendriks and Antonio Corrao for reporting
// bugs in this module.
//-------------------------------------------------------------------*--------*
// Mapped on memory file class implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include <time.h>
#include "storage.h"

BEGIN_POST_NAMESPACE

post_file* post_file::chain;
char* program_compilation_time;

void post_file::set_file_name(const char* file_name)
{
    size_t len = strlen(file_name);
   
    name = new char[len+5];
    log_name = new char[len+5];
    sav_name = new char[len+5];
    tmp_name = new char[len+5];
    strcpy(name, file_name);
    if (len < 4 || file_name[len-4] != '.') { 
	strcat(name, ".odb");
    }
    strcat(strcpy(log_name, file_name), ".log");
    strcat(strcpy(tmp_name, file_name), ".tmp");
    strcat(strcpy(sav_name, file_name), ".sav");
}

#ifdef _WIN32

#ifndef _WIN64
bool post_file::handle_page_access(DWORD* params) 
#else
bool post_file::handle_page_access(ULONG_PTR* params) 
#endif
{
    for (post_file* fp = chain; fp != NULL; fp = fp->next) { 
	if (params[1] - (ptrdiff_t)fp->base < fp->mapped_size) { 
	    return fp->prot != read_only 
		&& fp->create_shadow_page(params[0], (void*)params[1]);
	}
    }  
    return false;
} 

static LONG WINAPI AccessViolationHandler(LPEXCEPTION_POINTERS ep)
{
    return ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION
	&& post_file::handle_page_access(ep->ExceptionRecord->ExceptionInformation)
	   ? EXCEPTION_CONTINUE_EXECUTION
	   : EXCEPTION_CONTINUE_SEARCH;
}


post_file::post_file(const char* name, size_t max_file_size, void* default_base_address, size_t max_locked_pages)
{
    SYSTEM_INFO sysinfo;
    MEMORYSTATUS memstat;

    OSVERSIONINFO osinfo;
    osinfo.dwOSVersionInfoSize = sizeof osinfo;
    GetVersionEx(&osinfo);
    platform = osinfo.dwPlatformId;
    
    GetSystemInfo(&sysinfo);
    allocation_granularity = sysinfo.dwAllocationGranularity;
    page_size = sysinfo.dwPageSize;
    base = (char*)default_base_address;

    if (platform == VER_PLATFORM_WIN32_NT) {     
	SIZE_T MinimumWorkingSetSize, MaximumWorkingSetSize;
	GetProcessWorkingSetSize(GetCurrentProcess(),
				 &MinimumWorkingSetSize, 
				 &MaximumWorkingSetSize);

	if (max_locked_pages > MinimumWorkingSetSize/page_size) { 
	    MinimumWorkingSetSize = max_locked_pages*2*page_size;
	    if (MaximumWorkingSetSize < MinimumWorkingSetSize) { 
		MaximumWorkingSetSize = MinimumWorkingSetSize;
	    }
	    if (!SetProcessWorkingSetSize(GetCurrentProcess(),
					  MinimumWorkingSetSize,
					  MaximumWorkingSetSize)) 
	    {
		const size_t max_nt_locked_pages = 30;
		TRACE_MSG(("post_file::file: failed to extend process working set "
			   "size to %ld bytes, set max_locked_pages to %ld\n", 
			   MinimumWorkingSetSize, max_nt_locked_pages));
		max_locked_pages = max_nt_locked_pages;
	    } else { 
		TRACE_MSG(("post_file::file: extend process working set size "
			   "to %ld bytes\n", MinimumWorkingSetSize));
	    }
	}
    } else { 
	max_locked_pages = 0;
    }
    memstat.dwLength = sizeof(memstat);
    GlobalMemoryStatus(&memstat);
    if (memstat.dwAvailVirtual < max_file_size) { 
	max_file_size = memstat.dwAvailVirtual & ~(allocation_granularity-1);
	TRACE_MSG(("post_file::file: set max_file_size to %ld bytes\n",
		   max_file_size));
    }

    set_file_name(name);
    this->max_file_size = ALIGN(max_file_size, allocation_granularity);
    this->max_locked_pages = max_locked_pages;

    log_buffer = new char[page_size+sizeof(ptrdiff_t)];
    locked_page = new char*[max_locked_pages];
    
    SetUnhandledExceptionFilter(AccessViolationHandler);
    error_code = ok;
}

post_file::~post_file() 
{
    delete[] name;
    delete[] log_name;
    delete[] tmp_name;
    delete[] sav_name;
    delete[] log_buffer;
    delete[] locked_page;
}


inline bool post_file::recover_file()
{
    DWORD read_bytes;
    size_t trans_size = page_size + sizeof(ptrdiff_t);
    int n_recovered_pages = 0;
    TRACE_MSG(("post_file::recover_file: recover data file from transaction log\n"));
    recovery = true;
    while (ReadFile(log, log_buffer, trans_size, &read_bytes, NULL)) { 
	if (read_bytes != trans_size) { 
	    recovery = false;
	    if (read_bytes == 0) { 
		size = ALIGN(((file_header*)base)->file_size, page_size);
		TRACE_MSG(("recover::file: recover %d pages, set file size "
			   "to %ld\n", n_recovered_pages, size));
		error_code = ok;
		return true;	    
	    } else { 
		error_code = end_of_file;
		TRACE_MSG(("post_file::recover_file: read %ld bytes from log "
			   "instead of %ld\n", 
			   read_bytes, trans_size));
		return false;
	    }
	}    
	size_t offs = *(ptrdiff_t*)log_buffer;
	assert(offs < size);
	memcpy(base + offs, log_buffer+sizeof(ptrdiff_t), page_size);
	n_recovered_pages += 1;
    }
    error_code = GetLastError();
    TRACE_MSG(("post_file::recover_file: failed to read log file: %d\n", 
	       error_code));
    recovery = false;
    return false;
}

inline bool post_file::flush_log_buffer()
{
    if (!FlushFileBuffers(log)) { 
	error_code = GetLastError();
	TRACE_MSG(("post_file::flush_log_buffer: FlushFileBuffers failed: %d\n", 
		   error_code));
	return false;
    }
    for (size_t i = n_locked_pages; i-- > 0;) { 
	if (!VirtualUnlock(locked_page[i], page_size)) { 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::flush_log_buffer: VirtualUnlock %p failed: %d\n",
		       locked_page[i], error_code));
	    return false;
	}
    }
    n_locked_pages = 0;
    return true;
}


bool post_file::create_shadow_page(int modify, void* addr)
{
    DWORD old_prot;
    DWORD written_bytes;
    char* page = (char*)(ptrdiff_t(addr) & ~(page_size-1));
    size_t offs = page - base;

    if (!VirtualProtect(page, page_size, 
			modify ? PAGE_READWRITE : PAGE_READONLY, &old_prot)) 
    { 
	TRACE_MSG(("post_file::create_shadow_page: VirtualProtect %p failed: %ld\n",
		   page, GetLastError()));
	return false;
    }
    if (platform != VER_PLATFORM_WIN32_NT) {     
	if (modify) { 
	    size_t page_no = offs / page_size;
	    dirty_page_map[page_no >> 5] |= 1 << (page_no & 0x1F);
	    if (recovery) { 
		return true;
	    }
	}
	if (old_prot == PAGE_NOACCESS) { 
	    if (!modify) { 
		if (!VirtualProtect(page,page_size,PAGE_READWRITE,&old_prot)){
		    TRACE_MSG(("post_file::create_shadow_page: "
			       "VirtualProtect2 %p failed: %ld\n", 
			       page, GetLastError()));
		    return false;
		}
	    }
	    DWORD read_bytes;
	    if (SetFilePointer(fd, offs, NULL, FILE_BEGIN) != offs
		|| !ReadFile(fd, page, page_size, &read_bytes, NULL))
	    {
		return false;
	    }
	    if (!modify) { 
		if (!VirtualProtect(page,page_size,PAGE_READONLY,&old_prot)) {
		    TRACE_MSG(("post_file::create_shadow_page: "
			       "VirtualProtect3 %p failed: %ld\n", 
			       page, GetLastError()));
		}		    
		return true;
	    }
	}
    }

    *(ptrdiff_t*)log_buffer = offs;
    memcpy(log_buffer+sizeof(ptrdiff_t), page, page_size);
    size_t trans_size = page_size + sizeof(ptrdiff_t);
    if (!WriteFile(log, log_buffer, trans_size, &written_bytes, NULL)
	|| written_bytes != trans_size)
    {
	TRACE_MSG(("post_file::create_shadow_page: WriteFile failed: %d\n", 
		   GetLastError()));
	return false;
    }
    if (max_locked_pages != 0) { 
	if (n_locked_pages >= max_locked_pages) { 
	    return flush_log_buffer();
	} 
	if (!VirtualLock(page, page_size)) { 
	    TRACE_MSG(("post_file::create_shadow_page: VirtualLock %p failed,"
		       "number of locked pages %ld: %ld\n",
		       page, n_locked_pages, GetLastError()));
	    if (n_locked_pages != 0) { 
		max_locked_pages = n_locked_pages;
	    }
	    return flush_log_buffer();
	} 
	locked_page[n_locked_pages++] = page;
    }
    return true;
}


bool post_file::read_file_in_memory()
{
#if DEBUG_LEVEL >= DEBUG_TRACE
    msg_buf buf;
#endif
    DWORD read_bytes;
    vmem = (char*)VirtualAlloc(base, mapped_size, 
			       MEM_RESERVE, PAGE_READWRITE);
    if (vmem == NULL) { 
	error_code = GetLastError();
	TRACE_MSG(("post_file::read_file_in_memory: failed to virtual alloc at "
		   "address %p: %s\n", base, get_error_text(buf, sizeof buf)));
	vmem = (char*)VirtualAlloc(NULL, mapped_size, 
				   MEM_RESERVE, PAGE_READWRITE);
    }
    if (vmem == NULL) { 
	error_code = GetLastError();
	TRACE_MSG(("post_file::read_file_in_memory: failed to virtual alloc: %s\n", 
		   get_error_text(buf, sizeof buf)));
	CloseHandle(fd);
	return false;
    }
    base = vmem;
    if (size != 0) { 
	TRACE_MSG(("post_file::read_file_in_memory: virtual alloc: address=%p, "
		   "mapped_size=%ld, size=%ld\n", vmem, mapped_size, size));
	if (base != (char*)VirtualAlloc(base, size, MEM_COMMIT, 
					PAGE_READWRITE) 
	    || SetFilePointer(fd, 0, NULL, FILE_BEGIN) != 0
	    || ReadFile(fd, base, size, &read_bytes, NULL) == false)
	{
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::read_file_in_memory: failed to read file in "
		       "memory: %s\n", get_error_text(buf, sizeof buf)));
	    CloseHandle(fd);
	    VirtualFree(vmem, 0, MEM_RELEASE);
	    return false;
	} else if (read_bytes != size) { 
	    error_code = file_size_not_aligned;
	    TRACE_MSG(("post_file::read_file_in_memory:read %ld bytes instead "
		       "of %ld\n", read_bytes, size));
	    CloseHandle(fd);
	    VirtualFree(vmem, 0, MEM_RELEASE);
	    return false;
	}	    
    }
    return true;
}

bool post_file::write_dirty_pages_in_file()
{
    size_t offs = 0; 
    unsigned page_no;
    for (page_no = 0; offs < size; page_no += 1) { 
	int mask = 1 << (page_no & 0x1F);
	if (dirty_page_map[page_no >> 5] & mask) {  
	    dirty_page_map[page_no >> 5] &= ~mask;
	    DWORD written_bytes;
	    if (SetFilePointer(fd, offs, NULL, FILE_BEGIN) != offs
		|| !WriteFile(fd, base+offs, page_size, &written_bytes, 0)
		|| written_bytes != page_size)
	    {
		error_code = GetLastError();
		TRACE_MSG(("post_file::write_dirty_pages_in_file: failed to write"
			   " page in file: %d\n", error_code));
		return false;
	    }
	    DWORD old_prot;
	    if (!VirtualProtect(base+offs, page_size, PAGE_READONLY, &old_prot)) {
		error_code = GetLastError();
		TRACE_MSG(("post_file::write_dirty_pages_in_file: failed to change protection"
			   " to read-only: %d\n", error_code));
		return false;
	    }
	}
	offs += page_size;
    }
    if (!FlushFileBuffers(fd)) { 
	error_code = GetLastError();
	TRACE_MSG(("post_file::write_dirty_pages_in_file: FlushFileBuffers "
		   "failed: %d\n", error_code));
	return false;
    }
    return true;
}

bool post_file::open(open_mode mode, access_prot prot) 
{
#if DEBUG_LEVEL >= DEBUG_TRACE
    msg_buf buf;
#endif
    DWORD old_prot;
    assert(name != NULL);
    if (prot == read_only) { 
	mode = map_file;
    } else if (mode == copy_on_write_map && platform != VER_PLATFORM_WIN32_NT){
	mode = load_in_memory; // copy on write not working in Windows 95
    }

    this->mode = mode;
    this->prot = prot;
    vmem = NULL;
    md = NULL;
    recovery = false;

    if (mode == shadow_pages_transaction) { 
	int flags = (platform == VER_PLATFORM_WIN32_NT)
	    ? FILE_FLAG_WRITE_THROUGH|FILE_FLAG_RANDOM_ACCESS
	    : FILE_FLAG_RANDOM_ACCESS;
	fd = CreateFileA(name, GENERIC_READ|GENERIC_WRITE, 0, 
			NULL, OPEN_ALWAYS, flags, NULL);
	if (fd == INVALID_HANDLE_VALUE) { 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::open: failed to open file '%s': %s\n", 
		       name, get_error_text(buf, sizeof buf)));
	    return false;
	}
	DWORD read_bytes;
	file_header hdr;
	hdr.base_address = base;
	hdr.file_size = 0;
        if (!ReadFile(fd, &hdr, sizeof hdr, &read_bytes, NULL)
	    || (read_bytes != 0 && read_bytes != sizeof hdr))
	{
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::open: failed to read file: %s\n", 
		       get_error_text(buf, sizeof buf)));
	    CloseHandle(fd);
	    return false;
	}
	base = (char*)hdr.base_address;
	allocated_size = size = ALIGN(hdr.file_size, page_size);
	mapped_size = size > max_file_size ? size : max_file_size;
	if (platform == VER_PLATFORM_WIN32_NT) {     
	    md = CreateFileMapping(fd,NULL,PAGE_READWRITE,0,mapped_size,NULL);
	    if (md == NULL) { 
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: failed to create file mapping: "
			   "base=%p, size=%ld: %s\n", 
			   base, mapped_size, 
			   get_error_text(buf, sizeof buf)));
		CloseHandle(fd);
		return false;
	    }
	    void* p = MapViewOfFileEx(md, FILE_MAP_ALL_ACCESS, 0, 0, 0, base);
	    if (p == NULL) { 
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: failed to map view of file on address "
			   "%p: %s\n", base, get_error_text(buf, sizeof buf)));
		p = MapViewOfFileEx(md, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);
		if (p == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to map view of file: %s\n",
			       get_error_text(buf, sizeof buf)));
		    CloseHandle(fd);
		    CloseHandle(md);
		    return false;
		}
	    }
	    base = (char*)p;
	    dirty_page_map = NULL;
	} else { 
	    //
	    // VirtualProtect doesn't work in Winfows 95 with 
	    // mapped on file memory. We have to use VirtualAlloc instead 
	    // to handle page faults and read pages from the file ourself.
	    //
	    vmem = (char*)VirtualAlloc(base, mapped_size, 
				       MEM_RESERVE, PAGE_READWRITE);
	    if (vmem == NULL) { 
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: failed to virtual alloc at address"
			   " %p: %s\n", base, get_error_text(buf,sizeof buf)));
		vmem = (char*)VirtualAlloc(NULL, mapped_size, 
					   MEM_RESERVE, PAGE_READWRITE);
	    }
	    if (vmem == NULL) { 
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: failed to virtual alloc: %s\n", 
			   get_error_text(buf, sizeof buf)));
		CloseHandle(fd);
		return false;
	    }
	    base = vmem;
	    if (size != 0) { 
		TRACE_MSG(("post_file::open: virtual alloc: address=%p, "
			   "mapped_size=%ld, size=%ld\n",
			   vmem, mapped_size, size));
		if (base != (char*)VirtualAlloc(base, size, MEM_COMMIT, 
						PAGE_NOACCESS)) 
		{
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to read file in memory: "
			       "%s\n", get_error_text(buf, sizeof buf)));
		    CloseHandle(fd);
		    VirtualFree(vmem, 0, MEM_RELEASE);
		    return false;
		}
	    }
	    size_t page_map_size = ((mapped_size / page_size) + 31) >> 5;
	    dirty_page_map = new int[page_map_size];
	    memset(dirty_page_map, 0, sizeof(int)*page_map_size);
	}
	n_locked_pages = 0;
	if (max_locked_pages != 0) {
	    // check if we can use VirtualLock()
	    if (VirtualLock(base, page_size)) { 
		VirtualUnlock(base, page_size);
	    } else { 
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: VirtualLock test failed: %s\n",
			   get_error_text(buf, sizeof buf)));
		max_locked_pages = 0;
	    }		
	}
	int log_flags = FILE_FLAG_SEQUENTIAL_SCAN;
	if (max_locked_pages == 0 && platform == VER_PLATFORM_WIN32_NT) { 
            log_flags |= FILE_FLAG_WRITE_THROUGH;
	}
	log = CreateFileA(log_name, GENERIC_READ|GENERIC_WRITE, 0, NULL, 
			 OPEN_EXISTING, log_flags, NULL); 
	if (log == INVALID_HANDLE_VALUE) { 
	    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
		log = CreateFileA(log_name, GENERIC_READ|GENERIC_WRITE, 0, NULL,
				 CREATE_ALWAYS, log_flags, NULL); 
	    }
	    if (log == INVALID_HANDLE_VALUE) { 
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: failed to create log file '%': %s\n", 
			   log_name, get_error_text(buf, sizeof buf)));
	      return_error:
		if (md != NULL) { 
		    UnmapViewOfFile(base);
		    CloseHandle(md);
		} else { 
		    VirtualFree(base, 0, MEM_RELEASE);
		}
		CloseHandle(fd);
		CloseHandle(log);
		delete[] dirty_page_map;
		return false;
	    }
	    if (size != 0 
		&& platform == VER_PLATFORM_WIN32_NT
		&& !VirtualProtect(base, size, PAGE_READONLY, &old_prot))
            {
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: VirtualProtect failed for base=%p, "
			   "size=%ld: %s\n", 
			   base, size, get_error_text(buf, sizeof buf)));
		goto return_error;
	    }
	    next = chain;
	    chain = this;
	} else { 
	    next = chain;
	    chain = this;
	    recover_file();	
	    if (!commit()) { 
		chain = next;
		goto return_error;
	    }
	}
    } else { // non-shadow_pages_transaction mode
	int access_flags = (mode == map_file)
	    ? GENERIC_READ|GENERIC_WRITE : GENERIC_READ;
	int create_flags = (mode == map_file) 
	    ? OPEN_ALWAYS : OPEN_EXISTING;
	int hint_flags = (mode == load_in_memory) 
	    ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_RANDOM_ACCESS; 
	int share_mode = (prot == read_only) ? FILE_SHARE_READ|FILE_SHARE_WRITE : FILE_SHARE_READ;
	fd = CreateFileA(name, access_flags, share_mode, NULL, 
			create_flags, hint_flags, NULL); 
	size = 0;
	if (fd != INVALID_HANDLE_VALUE) { 
	    DWORD read_bytes;
	    if (!ReadFile(fd, &base, sizeof base, &read_bytes, NULL)
		|| (read_bytes != 0 && read_bytes != sizeof base))
	    {
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: failed to read file '%s': %s\n", 
			   name, get_error_text(buf, sizeof buf)));
		CloseHandle(fd);
		return false;
	    }
	    size = GetFileSize(fd, NULL);
	    if ((size & (allocation_granularity-1)) != 0
		&& prot != read_only 
		&& mode == copy_on_write_map 
		&& size < max_file_size)
	    {
		error_code = file_size_not_aligned;
		TRACE_MSG(("post_file::open: size of file '%s' is not aligned "
			   "on %ld\n", name, allocation_granularity));
		return false;
	    }
	    if (mode == copy_on_write_map) { 
		mapped_size = size;
		md = CreateFileMapping(fd, NULL, PAGE_WRITECOPY, 0, size,NULL);
		if (md == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to create file mapping: "
			       "%s\n", get_error_text(buf, sizeof buf)));
		    CloseHandle(fd);
		    return false;
		}
		TRACE_MSG(("post_file::open: create file mapping: size=%ld\n",
			   size));
		void* p = MapViewOfFileEx(md, prot == read_only 
					  && platform == VER_PLATFORM_WIN32_NT 
					  ? FILE_MAP_READ : FILE_MAP_COPY, 
					  0, 0, size, base);
		if (p == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to map view of file on "
			       "address %p: %s\n", 
			       base, get_error_text(buf, sizeof buf)));
		    p = MapViewOfFileEx(md, prot == read_only  
					&& platform == VER_PLATFORM_WIN32_NT
					? FILE_MAP_READ : FILE_MAP_COPY,
					0, 0, size, NULL);
		}
		if (p == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to map view of file: %s\n", 
			       get_error_text(buf, sizeof buf)));
		    CloseHandle(md);
		    CloseHandle(fd);
		    return false;
		}
		base = (char*)p;
		TRACE_MSG(("post_file::open: map view of file on %p\n", base));
		if (prot != read_only && size < max_file_size) { 
		    mapped_size = max_file_size;
		    vmem = (char*)VirtualAlloc(base + size, mapped_size - size,
					       MEM_RESERVE, PAGE_READWRITE);
		    if (vmem == NULL) { 
			error_code = GetLastError();
			TRACE_MSG(("post_file::open: failed to allocate end of "
				   "region: base=%p, size=%ld: %s\n", 
				   base+size, mapped_size-size, 
				   get_error_text(buf, sizeof(buf))));
			UnmapViewOfFile(p);
			CloseHandle(md);
			CloseHandle(fd);
			return false;
		    }
		    TRACE_MSG(("post_file::open: virtual alloc: address=%p, "
			       "size=%ld\n", vmem, mapped_size));
		    assert(vmem == base + size);
		}  
	    } else if (mode == map_file) { 
		mapped_size = (prot == read_only || size > max_file_size)
		    ? size : max_file_size;
		md = CreateFileMapping(fd, NULL, PAGE_READWRITE, 0, 
				       mapped_size, NULL);
		if (md == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to create file mapping: "
			       "%s\n", get_error_text(buf, sizeof buf)));
		    CloseHandle(fd);
		    return false;
		}
		TRACE_MSG(("post_file::open: create file mapping: size=%ld\n", 
			   size));
		void* p = MapViewOfFileEx(md, prot == read_only 
					  ?FILE_MAP_READ :FILE_MAP_ALL_ACCESS, 
					  0, 0, mapped_size, base);
		if (p == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to map view of file on "
			       "address %p: %s\n", 
			       base, get_error_text(buf, sizeof buf)));
		    p = MapViewOfFileEx(md, prot == read_only  
					? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
					0, 0, mapped_size, NULL);
		}
		if (p == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::open: failed to map view of file: %s\n", 
			       get_error_text(buf, sizeof buf)));
		    CloseHandle(md);
		    CloseHandle(fd);
		    return false;
		}
		base = (char*)p;
		TRACE_MSG(("post_file::open: map view of file on %p\n", base));
	    } else { // read file to memory
		mapped_size = (prot == read_only || size > max_file_size)
		    ? size : max_file_size;
		if (!read_file_in_memory()) { 
		    return false;
		}
		CloseHandle(fd);
		fd = INVALID_HANDLE_VALUE;
	    }
	} else { 
	    error_code = GetLastError();
	    if (error_code != ERROR_FILE_NOT_FOUND) { 
		TRACE_MSG(("post_file::open: failed to open file '%s': %s\n", 
		       name, get_error_text(buf, sizeof buf)));
		return false;
	    }
	    TRACE_MSG(("post_file::open: file '%s' not found\n", name));
	    if (prot == read_only) {
		TRACE_MSG(("post_file::open: failed to open in read only mode "
			   "unexisted file '%s'\n", name));
		return false;
	    }
	    mapped_size = max_file_size;
	    base = vmem = (char*)VirtualAlloc(NULL, mapped_size, 
					      MEM_RESERVE, PAGE_READWRITE);
	    if (base == NULL) {
		error_code = GetLastError();
		TRACE_MSG(("post_file::open: failed to virtual alloc: %s\n", 
			   get_error_text(buf, sizeof buf)));
		return false;
	    }
	}
    }
    error_code = ok;
    return true;
}

bool post_file::set_size(size_t new_size)
{
    if (new_size > mapped_size) { 
	error_code = file_mapping_size_exceeded;
	return false;
    } 
    if (mode == shadow_pages_transaction) {
	new_size = ALIGN(new_size, page_size);
	if (platform != VER_PLATFORM_WIN32_NT) { // Windows 95, uhh...
	    if (new_size > allocated_size) { 
		allocated_size = ALIGN(new_size, allocation_granularity);
		if (!VirtualAlloc(base+size, allocated_size-size, MEM_COMMIT,
				  PAGE_READWRITE))
		{
		    error_code = GetLastError();
		    return false;
		}
		size_t page_no = size / page_size;
		size_t offs = size; 
		while (offs < allocated_size) {
		    dirty_page_map[page_no >> 5] |= 1 << (page_no & 0x1F);
		    offs += page_size;
		    page_no += 1;
		} 
	    }
	}
    } else { 
	if (mode != map_file) { 
	    new_size = ALIGN(new_size, allocation_granularity);
	    if (new_size > size) { 
		if (!VirtualAlloc(base+size, new_size-size, MEM_COMMIT,
				  PAGE_READWRITE))
		{
		    error_code = GetLastError();
		    return false;
		}
	    }
	}
    }
    size = new_size;
    error_code = ok;
    return true;
}

bool post_file::set_protection(access_prot prot)
{
    DWORD old_prot;
    if (platform == VER_PLATFORM_WIN32_NT || mode == map_file) { 
	if (!VirtualProtect(base, mapped_size, 
			    prot == post_file::read_only 
			    ? PAGE_READONLY 
			    : mode == copy_on_write_map 
			      ? PAGE_WRITECOPY : PAGE_READWRITE,
			    &old_prot))
	{
	    error_code = GetLastError();
	    return false;
	}
    }
    this->prot = prot;
    error_code = ok;
    return true;
}


bool post_file::commit()
{
    DWORD old_prot;
    if (mode != shadow_pages_transaction) { 
	error_code = not_in_transaction;
	return false;
    }
    if (n_locked_pages != 0 || platform != VER_PLATFORM_WIN32_NT) { 
	if (!flush_log_buffer()) { 
	    return false;
	}
    }
    if (platform == VER_PLATFORM_WIN32_NT) { 
	if (size > 0 && !FlushViewOfFile(base, size)) { 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::commit: FlushViewOfFile(%p, %ld) failed: %d\n",
		       base, size, error_code));
	    return false;
	}
    } else { 
	if (!write_dirty_pages_in_file()) { 
	    return false;
	}
    } 
    if (SetFilePointer(log, 0, NULL, FILE_BEGIN) != 0 || !SetEndOfFile(log)) { 
	error_code = GetLastError();
	TRACE_MSG(("post_file::commit: failed to truncate lof file: %d\n", 
		   error_code));
	return false;
    }
    if (size > 0 && platform == VER_PLATFORM_WIN32_NT && !VirtualProtect(base, size, PAGE_READONLY, &old_prot)){
	error_code = GetLastError();
	TRACE_MSG(("post_file::commit: VirtualProtect(%p, %ld) failed: %d\n",
		   base, size, error_code));
	return false;
    }
    error_code = ok;
    return true;
}

bool post_file::rollback()
{
    if (mode != shadow_pages_transaction) { 
	error_code = not_in_transaction;
	return false;
    }
    SetFilePointer(log, 0, NULL, FILE_BEGIN);
    return recover_file();
}

bool post_file::flush() 
{
    if (prot == read_only) {
	return true;
    }
    if (mode == shadow_pages_transaction) { 
	return commit();
    } else if (mode == map_file) { 
	if (!FlushViewOfFile(base, size)) {
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::flush: failed to flush file mapping: %d\n",
		       error_code));
	    return false;
	}
	error_code = ok;
	return true;
    } else { // non-shadow_pages_transaction mode
	HANDLE tmp_fd = CreateFileA(tmp_name, GENERIC_WRITE, 0, NULL, 
				   CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 
				   NULL);
	if (tmp_fd == INVALID_HANDLE_VALUE) { 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::flush: failed to create temporary file '%s': "
		       "%d\n", tmp_name, error_code));
	    return false;
	}
	DWORD written_bytes;
	if (!WriteFile(tmp_fd, base, size, &written_bytes, NULL)
	    || written_bytes != size)
	{
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::flush: failed to write %ld bytes to file '%s': "
		       "%d\n", size, tmp_name, error_code));
	    CloseHandle(tmp_fd);
	    return false;
	}
	CloseHandle(tmp_fd);
	if (platform == VER_PLATFORM_WIN32_NT) { 
	    if (fd != INVALID_HANDLE_VALUE) { 
		assert(md != NULL);
		if (!UnmapViewOfFile(base)) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to unmap %p: %d\n", 
			       base, error_code));
		    return false;
		}
		if (!CloseHandle(md)) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to close file mapping: "
			       "%d\n", error_code));
		    return false;
		}	    
		if (!CloseHandle(fd)) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to close file: %d\n",
			       error_code));
		    return false;
		}	    		    
		if (!MoveFileExA(tmp_name, name, MOVEFILE_REPLACE_EXISTING|
				MOVEFILE_WRITE_THROUGH) != 0)
		{ 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to reanme fileL %d\n",
			       error_code));
		    return false;
		}
		fd = CreateFileA(name, GENERIC_READ, FILE_SHARE_READ, 
				NULL, OPEN_EXISTING, 
				FILE_FLAG_SEQUENTIAL_SCAN, NULL); 
		if (fd == INVALID_HANDLE_VALUE) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to open file: %d\n", 
			       error_code));
		    return false;
		}
		assert(vmem != NULL);
		md = CreateFileMapping(fd, NULL, PAGE_WRITECOPY, 0, 
				       vmem - base, NULL);
		if (md == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to create file mapping: "
			       "base=%p, size=%ld: %d\n", 
			       base, vmem - base, error_code));
		    CloseHandle(fd);
		    return false;
		}
		void* p = MapViewOfFileEx(md, FILE_MAP_COPY, 
					  0, 0, vmem - base, base);
		if (p == NULL) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to map view of file on "
			       "address %p size %d: %d\n", 
			       base, vmem-base, error_code));
		    CloseHandle(fd);
		    CloseHandle(md);
		    return false;
		}
		assert(p == base);
	    } else { // just rename
		if (!MoveFileExA(tmp_name, name, MOVEFILE_REPLACE_EXISTING|
				MOVEFILE_WRITE_THROUGH) != 0)
		{ 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to reanme fileL %d\n",
			       error_code));
		    return false;
		}
	    }
	} else { // windows 95
	    if (fd != INVALID_HANDLE_VALUE) { 
		CloseHandle(fd);
		fd = INVALID_HANDLE_VALUE;
	    }
	    BOOL backup = MoveFileA(name, sav_name);// may be exists, may be not
	    if (!backup && GetLastError() == ERROR_ALREADY_EXISTS) {
		DeleteFileA(sav_name); // remove previous backup file
		backup = MoveFileA(name, sav_name);
		if (!backup) { 
		    error_code = GetLastError();
		    TRACE_MSG(("post_file::flush: failed to make backup copy of "
			       "file '%s'\n", name));
		    return false;
		}
	    }
	    if (!MoveFileA(tmp_name, name)) { 
		error_code = GetLastError();
		TRACE_MSG(("post_file::flush: failed to rename file '%s' to '%s'\n",
			   tmp_name, name));
		return false;
	    }
	    if (backup) { 
		DeleteFileA(sav_name); 
	    }
	}
	error_code = ok;
	return true;
    }
}

bool post_file::close()
{
    if (mode == shadow_pages_transaction) { 
	post_file *fp, **fpp = &chain;
	while ((fp = *fpp) != this) { 
	    fpp = &fp->next;
	}
	*fpp = fp->next;
	    
	if (n_locked_pages != 0 || platform != VER_PLATFORM_WIN32_NT) { 
	    if (!flush_log_buffer()) { 
		return false;
	    }
	}
	if (platform != VER_PLATFORM_WIN32_NT) { 
	    if (!write_dirty_pages_in_file()) {
		return false;
	    }
	    delete[] dirty_page_map;
	}
    }
    if (vmem != NULL) { 
	if (!VirtualFree(vmem, 0, MEM_RELEASE)) { 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::close: failed to free %p: %d\n", 
		       vmem, error_code));
	    return false;
	}
    }
    if (md != NULL) { 
        if (!UnmapViewOfFile(base)) { 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::close: failed to unmap %p: %d\n", 
		       base, error_code));
	    return false;
	}
	if (!CloseHandle(md)) { 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::close: failed to close file mapping: %d\n",
		       error_code));
	    return false;
	}	    
    }
    if ((mode == shadow_pages_transaction 
	 && platform == VER_PLATFORM_WIN32_NT) 
	|| mode == map_file)
    {
	if (SetFilePointer(fd, size, NULL, FILE_BEGIN) != size ||
	    !SetEndOfFile(fd)) 
	{ 
	    error_code = GetLastError();
	    TRACE_MSG(("post_file::close: failed to change size of file to %ld: "
		       "%d\n", size, error_code));
	    return false;
	}	    	    
    }
    if (fd != INVALID_HANDLE_VALUE) { 
	if (!CloseHandle(fd)) { 	    
 	    error_code = GetLastError();
	    TRACE_MSG(("post_file::close: failed to close file: %d\n", error_code));
	    return false;
	}
    }
    if (mode == shadow_pages_transaction) { 
	if (!CloseHandle(log)) { 
	    TRACE_MSG(("post_file::close: CloseHandle for log failed: %d\n", 
		       GetLastError()));
	}
	if (!DeleteFileA(log_name)) { 
	    TRACE_MSG(("post_file::close: failed to remove log file: %d\n", 
		       GetLastError()));
	}
    }
    error_code = ok;
    return true;
}

char* post_file::get_error_text(char* buf, size_t buf_size)
{
    char* err_txt;
    char errbuf[64];

    switch (error_code) {
      case ok: 
	err_txt = "no error";
	break;
      case file_size_not_aligned:
	err_txt = "size of file is not aligned on segment boundary";
	break;
      case file_mapping_size_exceeded:
	err_txt = "attempt to exceed the limit on mapped object size";
	break;
      case not_in_transaction:
	err_txt = "operation is possible only inside transaction"; 
	break;
      case end_of_file:
	err_txt = "unexpected end of file"; 
	break;
      default: 
	if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
			  NULL, error_code, 0,
			  buf, buf_size, NULL) > 0)
	{
	    return buf;
	} else { 
	    sprintf(errbuf, "unknown error code %u", error_code);
	    err_txt = errbuf;
	}
    }
    return strncpy(buf, err_txt, buf_size);
}

char* post_file::get_program_timestamp()
{
    static char buf[32];
    char program_name[MAX_PATH];
    GetModuleFileNameA(NULL, program_name, sizeof program_name);
    FILETIME mftime;
    SYSTEMTIME mstime;
    HANDLE program = CreateFileA(program_name, GENERIC_READ, 
				FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    
    if (program != INVALID_HANDLE_VALUE) { 
	if (GetFileTime(program, NULL, NULL, &mftime)
	    && FileTimeToSystemTime(&mftime, &mstime))
	{
	    sprintf(buf, "%02d.%02d.%04d %02d:%02d.%02d", 
		    mstime.wDay, mstime.wMonth, mstime.wYear, 		    
		    mstime.wHour, mstime.wMinute, mstime.wSecond);
	} 
	CloseHandle(program);
    } 
    assert(*buf != '\0'); 
    return buf;
}

#else // Unix

END_POST_NAMESPACE

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/resource.h>

#ifndef SA_SIGINFO
#define SA_SIGINFO 0
#endif

#ifndef SA_RESTART
#define SA_RESTART 0
#endif

#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#ifndef MAP_VARIABLE
#define MAP_VARIABLE 0
#endif

#ifndef MAP_NORESERVE
// This attrbute used in Solaris to avoid reservation space in swap file
#define MAP_NORESERVE 0
#endif

#define USE_WRITEV 1 // most modern Unix-s have this function
#ifdef USE_WRITEV
#include <sys/uio.h>
#endif

BEGIN_POST_NAMESPACE

#define USE_FDATASYNC 0 // more efficient version of fsync()

#ifndef O_SYNC
#define O_SYNC O_FSYNC
#endif

// minimal number of pages appended to the file at each file extensin
#define FILE_EXTENSION_QUANTUM 256

//
// mlock(), munlock() functions can be used in transaction mode to 
// provide buffering of shadow pages writes to transaction log.
// In Unix unly superuser can use this functions. Method post_file::open()
// will check if it is possible to use mlock() function.
// 
#if !defined(__osf__) && !defined(_AIX) && !defined(__hpux__)
// These systems doesn't support mlock() 
#define USE_MLOCK 1
#endif

#ifndef USE_MLOCK
#undef mlock
#undef munlock
#define mlock(p, s) -1
#define munlock(p, s) -1
#endif

#ifndef MAP_ANONYMOUS 
int dev_zero = open("/dev/zero", O_RDONLY, 0);
#endif

#if defined(__sun__)
extern "C" int getpagesize(void);
#endif

bool post_file::handle_page_modification(void* addr) 
{
    for (post_file* fp = chain; fp != NULL; fp = fp->next) { 
	if (size_t((char*)addr - fp->base) < fp->mapped_size) { 
	    return fp->prot != read_only && fp->create_shadow_page(true, addr);
	}
    }  
    return false;
} 

//
// It will takes a lot of time and disk space to produce core file
// for program with huge memory mapped segment. This option allows you
// to automatically start debugger when some exception is happened in program.
//
#define CATCH_SIGNALS 1

#ifdef CATCH_SIGNALS
END_POST_NAMESPACE
#include <signal.h>
BEGIN_POST_NAMESPACE

#define WAIT_DEBUGGER_TIMEOUT 10

static void fatal_error_handler(int signo) { 
    char  buf[64];
    char* exe_name = getenv("_"); // defined by BASH
    if (exe_name == NULL) { 
	exe_name = "a.out";
    }
    fprintf(stderr, "\
Program catch signal %d.\n\
Input path to executable file to debug or press Ctrl/C to terminate program:\n\
[%s] ", 
	    signo, exe_name);
    int pid = getpid();
    if (fgets(buf, sizeof buf, stdin)) { 
	char pid_str[16];
	int len = strlen(buf);
	if (len > 1) { 
	    buf[len-1] = '\0'; // truncate '\n'
	    exe_name = buf;
	}
	sprintf(pid_str, "%d", pid); 
	if (fork()) { 
	    sleep(WAIT_DEBUGGER_TIMEOUT);
	} else { 
	    execlp("gdb", "gdb", "-q", "-s", exe_name, exe_name, pid_str, NULL);
	}
    } else { 
	kill(pid, SIGKILL);
    }
}
#endif


inline bool post_file::flush_log_buffer()
{
#if USE_FDATASYNC
    if (fdatasync(log) != ok) { 
#else
    if (fsync(log) != ok) { 
#endif
	error_code = errno;
	TRACE_MSG(("post_file::flush_log_buffer: fsync failed: %s\n", 
		   strerror(error_code)));
	return false;
    }
    for (int i = n_locked_pages; --i >= 0;) { 
	if (munlock(locked_page[i], page_size) != ok) { 
	    error_code = errno;
	    TRACE_MSG(("post_file::flush_log_buffer: munlock %p failed: %s\n",
		       locked_page[i], strerror(error_code)));
	    return false;
	}
    }
    n_locked_pages = 0;
    return true;
}

bool post_file::create_shadow_page(int, void* addr)
{
    char* page = (char*)(ptrdiff_t(addr) & ~(page_size-1));
    if (mprotect(page, page_size, PROT_READ|PROT_WRITE) != ok) { 
	TRACE_MSG(("post_file::create_shadow_page: mprotect failed for %p: %s\n", 
		   page, strerror(errno)));
	return false;
    }
#if USE_WRITEV
    iovec iov[2];
    ptrdiff_t offs = page - base;
    iov[0].iov_base = (char*)&offs;
    iov[0].iov_len = sizeof offs;
    iov[1].iov_base = page;
    iov[1].iov_len = page_size;
    if ((size_t)writev(log, iov, 2) != sizeof(ptrdiff_t) + page_size) { 
	TRACE_MSG(("post_file::create_shadow_page: writev failed: %s\n",
		   strerror(errno)));
	return false;
    }
#else
    *(ptrdiff_t*)log_buffer = page - base;
    memcpy(log_buffer + sizeof(ptrdiff_t), page, page_size);
    size_t trans_size = page_size + sizeof(ptrdiff_t);
    if (write(log, log_buffer, trans_size) != trans_size) { 
	TRACE_MSG(("post_file::create_shadow_page: failed to write log file: %s\n", 
		   strerror(errno)));
	return false;
    }
#endif
    if (max_locked_pages != 0) { 
	if (n_locked_pages >= max_locked_pages) { 
	    return flush_log_buffer();
	}  
	if (mlock(page, page_size) != ok) { 
	    TRACE_MSG(("post_file::create_shadow_page: mlock %p failed: %s\n", 
		       page, strerror(errno)));
	    if (n_locked_pages != 0) { 
		max_locked_pages = n_locked_pages;
	    }
	    return flush_log_buffer();
	}  
	locked_page[n_locked_pages++] = page;
    }
    return true;
}


#if defined(_AIX) || defined(__FreeBSD__)
static void sigsegv_handler(int, int, struct sigcontext *scp)
#else
static void sigsegv_handler(int, siginfo_t *info)
#endif
{
    char* fault_addr;
#if defined(_AIX)
    fault_addr = (char*)scp->sc_jmpbuf.jmp_context.o_vaddr;
#elif defined(__FreeBSD__)
    fault_addr = (char*)scp->sc_err;
#else
    fault_addr = (char*)info->si_addr;
#endif

    if (!post_file::handle_page_modification(fault_addr)) {
	kill(getpid(), SIGABRT);
    }
}

post_file::post_file(const char* name, size_t max_file_size, void* default_base_address, size_t max_locked_pages)
{
    // reserver one char for backup/log file names construction
    set_file_name(name);
    base = (char*)default_base_address;
#ifndef USE_MLOCK
    max_locked_pages = 0;
#else
    if (max_locked_pages != 0) { 
	// Check if we can use mlock()
	if (mlock((char*)this, page_size) != ok) { 
	    max_locked_pages = 0;
	    TRACE_MSG(("post_file::open: mlock test failed: %s\n",
		       strerror(errno)));
	} else { 
	    munlock((char*)this, page_size);
	}
    }
#endif
    allocation_granularity = page_size = getpagesize();

    struct rlimit mem_limit;
    if (getrlimit(RLIMIT_DATA, &mem_limit) == ok) { 
	if (max_file_size > size_t(mem_limit.rlim_cur)) { 
	    max_file_size = mem_limit.rlim_cur & ~(allocation_granularity-1);
	    TRACE_MSG(("post_file::file: set max_file_size to %ld bytes\n", 
		       max_file_size));
	}
    } else {
	TRACE_MSG(("post_file::file: getrlimit failed: %s\n", strerror(errno)));
    }

    this->max_file_size = ALIGN(max_file_size, allocation_granularity);
    this->max_locked_pages = max_locked_pages;

    log_buffer = new char[page_size+sizeof(ptrdiff_t)];
    locked_page = new char*[max_locked_pages];
    file_extension_granularity = FILE_EXTENSION_QUANTUM*page_size;

    static struct sigaction sigact; 
    sigact.sa_handler = (void(*)(int))sigsegv_handler;
    sigact.sa_flags = SA_RESTART|SA_SIGINFO;
    sigaction(SIGSEGV, &sigact, NULL);
#if defined(__FreeBSD__)
    sigaction(SIGBUS, &sigact, NULL);
#endif
#ifdef CATCH_SIGNALS
    sigact.sa_flags = 0;
    sigact.sa_handler = fatal_error_handler;
#if !defined(__FreeBSD__)
    sigaction(SIGBUS, &sigact, NULL);
#endif
    sigaction(SIGILL, &sigact, NULL);
    sigaction(SIGABRT, &sigact, NULL);
#endif
    error_code = ok;
}

post_file::~post_file() 
{
    delete[] name;
    delete[] log_name;
    delete[] tmp_name;
    delete[] sav_name;
    delete[] log_buffer;
    delete[] locked_page;
}

char* post_file::get_error_text(char* buf, size_t buf_size)
{
    char* err_txt;
    switch (error_code) {
      case ok: 
	err_txt = "no error";
	break;
      case file_size_not_aligned:
	err_txt = "size of file is not aligned on page boundary";
	break;
      case file_mapping_size_exceeded:
	err_txt = "attempt to exceed the limit on mapped object size";
	break;
      case not_in_transaction:
	err_txt = "operation is possible only inside transaction"; 
	break;
      case end_of_file:
	err_txt = "unexpected end of file"; 
	break;
      default: 
	err_txt = strerror(error_code);
    }
    return strncpy(buf, err_txt, buf_size);
}

inline bool post_file::recover_file()
{
    ssize_t rc, trans_size = page_size + sizeof(ptrdiff_t);
    int n_recovered_pages = 0;
    TRACE_MSG(("post_file::recover_file: recover data file from transaction log\n"));
    while ((rc = read(log, log_buffer, trans_size)) == trans_size) { 
	memcpy(base + *(ptrdiff_t*)log_buffer, log_buffer+sizeof(ptrdiff_t), page_size);
	n_recovered_pages += 1;
    }
    if (rc != 0) {
	if (rc < 0) { 
	    error_code = errno;
	    TRACE_MSG(("post_file::recover: log read failed: %s\n", 
		       strerror(error_code)));
	} else { 
	    error_code = end_of_file;	
	    TRACE_MSG(("post_file::recover: expected end of log\n"));
	}
	return false;
    }
    allocated_size = size = ALIGN(((file_header*)base)->file_size, page_size);
    TRACE_MSG(("recover::file: recover %d pages, set file size to %ld\n", 
	       n_recovered_pages, size));
    error_code = ok;
    return true;
}


bool post_file::open(open_mode mode, access_prot prot) 
{
    assert(name != NULL);
    if (prot == read_only) { 
	mode = map_file;
    } 
    this->mode = mode;
    this->prot = prot;

    if (mode == shadow_pages_transaction) {
	fd = ::open(name, O_RDWR|O_CREAT|O_SYNC, 0777);
	if (fd < 0) { 
	    error_code = errno;
	    TRACE_MSG(("post_file::open: failed to open file '%s': %s\n",
		       name, strerror(error_code)));
	    return false;
	}
	file_header hdr;
	hdr.base_address = base;
	hdr.file_size = 0;
	read(fd, &hdr, sizeof hdr);
	base = (char*)hdr.base_address;
	allocated_size = size = ALIGN(hdr.file_size, page_size);
	mapped_size = size > max_file_size ? size : max_file_size;
	void* p = mmap(base, mapped_size, PROT_READ|PROT_WRITE,
		       MAP_VARIABLE|MAP_SHARED|MAP_FILE, fd, 0);
	if (p == (char*)MAP_FAILED) { 
	    error_code = errno;
	    TRACE_MSG(("post_file::open: mmap failed: base=%p, size=%ld: %s\n",
		       base, mapped_size, strerror(error_code)));
	    ::close(fd);
	    return false;
	}
	base = (char*)p;
	n_locked_pages = 0;

	int log_flags = O_RDWR;
	if (max_locked_pages == 0) { 
	    log_flags |= O_SYNC;
	}
	log = ::open(log_name, log_flags, 0); 
	if (log < 0) { 
	    if (errno != ENOENT || 
		(log = ::open(log_name, log_flags|O_CREAT, 0777)) < 0)
	    {
		error_code = errno;
		TRACE_MSG(("post_file::open: failed to open log file '%s': %s\n",
			   log_name, strerror(error_code)));
		munmap(base, mapped_size);
		::close(fd);
		return false;
	    }
	    if (size != 0 && mprotect(base, size, PROT_READ) != ok) {
		error_code = errno;
		TRACE_MSG(("post_file::open: mprotect failed: address=%p, size=%ld:"
			   " %s\n", base, size, strerror(error_code)));
		munmap(base, mapped_size);
		::close(fd);
		::close(log);
		return false;
	    }
	    next = chain;
	    chain = this;
	} else { 
	    next = chain;
	    chain = this;
	    recover_file();
	    if (!commit()) { 
		chain = next;
		munmap(base, mapped_size);
		::close(fd);
		::close(log);
		return false;
	    }
	}
    } else { // mapping file in non-transaction mode
	int open_mode = O_RDONLY;
	if (prot != read_only) { 
	    if (mode == map_file) { 
		open_mode = O_RDWR|O_CREAT;
	    } else if (mode == copy_on_write_map) {
		open_mode = O_RDWR;
	    }
	} else { 
	    open_mode = O_RDWR; // to make it possible to change memory object protection
	}

	fd = ::open(name, open_mode, 0777);
	int d;
	if (fd >= 0) { 
	    size_t rc = read(fd, &base, sizeof base);
	    if (rc != 0 && rc != sizeof base) { 
		error_code = errno;
		TRACE_MSG(("post_file::open: failed to read from file '%s': %s\n",
			   name, strerror(error_code)));
		::close(fd);
		return false;
	    }
	    size = ALIGN(lseek(fd, 0, SEEK_END), allocation_granularity);
	    TRACE_MSG(("post_file::open: file '%s' exists: size=%ld, base=%p\n", 
		       name, size, base));
	} else { 
	    error_code = errno;
	    TRACE_MSG(("post_file::open: can't open file '%s', %s\n", 
		       name, strerror(error_code)));
	    if (error_code != ENOENT || mode == map_file) { 
		return false;
	    }
	    if (prot == read_only) {
		TRACE_MSG(("post_file::open: failed to open storage in read_only "
			   "mode because file '%s' doesn't exist\n", name));
		return false;
	    }
	    size = 0;
	}
	mapped_size = (prot == read_only || size > max_file_size) 
	    ? size : max_file_size;
	allocated_size = size;

	int mmap_attr = (mode == map_file) 
	    ? MAP_VARIABLE|MAP_SHARED : MAP_VARIABLE|MAP_PRIVATE;

	if (fd >= 0 && mode != load_in_memory) {
	    mmap_attr |= MAP_FILE;
	    d = fd;
	} else {
#ifndef MAP_ANONYMOUS
	    d = dev_zero;
#else
	    d = -1;
	    mmap_attr |= MAP_ANONYMOUS;
#endif
	}
	int mmap_prot = 
	    (prot == read_only) ? PROT_READ : PROT_READ|PROT_WRITE;
	void* p = mmap(base, mapped_size, mmap_prot, mmap_attr, d, 0);
	if (p == (void*)MAP_FAILED) { 
	    error_code = errno;
	    TRACE_MSG(("post_file::open: mmap to %p failed: %s\n", 
		       base, strerror(error_code)));
	    if (fd >= 0) { 
		::close(fd);
	    }
	    return false;
	}
	TRACE_MSG(("post_file::open: map file to address %p\n", p));
	base = (char*)p;
	
	if (fd >= 0 && mode == load_in_memory) { 
	    //
	    // Read file to memory
	    //
	    lseek(fd, 0, SEEK_SET);
	    if ((size_t)read(fd, base, size) != size) { 
		error_code = errno;
		TRACE_MSG(("post_file::open: failed to read file in memory: %s\n", 
			   strerror(error_code)));
		munmap(base, mapped_size);
		::close(fd);
		return false;
	    }
	    ::close(fd); // file is nor more needed
	    fd = -1;
	}
    } 
    error_code = ok;
    return true;
}

bool post_file::set_size(size_t new_size)
{
    new_size = ALIGN(new_size, allocation_granularity);
    if (new_size > mapped_size) { 
	error_code = file_mapping_size_exceeded;
	return false;
    } 
    if (fd >= 0 && new_size > allocated_size) { 
	allocated_size = ALIGN(new_size, file_extension_granularity);
	if (ftruncate(fd, allocated_size) != ok) { 
	    error_code = errno;
	    return false;
	}
    }
    error_code = ok;
    size = new_size;
    return true;
}

bool post_file::set_protection(access_prot prot) 
{
    if (mprotect(base, mapped_size, 
		 prot == read_only ? PROT_READ : PROT_READ|PROT_WRITE) != ok)
    {
	error_code = errno;
	return false;
    } else { 
	error_code = ok;
	this->prot = prot;
	return true;
    }
}

bool post_file::commit()
{
    if (mode != shadow_pages_transaction) { 
	error_code = not_in_transaction;
	return false;
    }
    if (n_locked_pages != 0 && !flush_log_buffer()) { 
	return false;
    }
    if (size > 0 && msync(base, size, MS_SYNC) != ok) { 
	error_code = errno;
	TRACE_MSG(("post_file::commit: msync failed for address %p size %ld: %s\n", 
		   base, size, strerror(error_code)));
	return false;
    }
    if (lseek(log, 0, SEEK_SET) != 0 ||
	ftruncate(log, 0) != ok) 
    {
	error_code = errno;
	TRACE_MSG(("post_file::commit: failed to truncate log file: %s\n",
		   strerror(error_code)));
	return false;
    }
    if (size > 0 && mprotect(base, size, PROT_READ) != ok) {
	error_code = errno;
	TRACE_MSG(("post_file::commit: mprotect failed for address %p size %ld: "
		   "%s\n", base, size, strerror(error_code)));
	return false;
    }
    error_code = ok;
    return true;
}

bool post_file::rollback()
{
    if (mode != shadow_pages_transaction) { 
	error_code = not_in_transaction;
	return false;
    }
    if (lseek(log, 0, SEEK_SET) != 0) { 
	error_code = errno;
	TRACE_MSG(("post_file::rollback; failed to set position in log file: %s\n", 
		   strerror(error_code)));
	return false;
    }
    return recover_file();
}

bool post_file::flush() 
{
    if (prot == read_only) {
	return true;
    }
    if (mode == shadow_pages_transaction) { 
	return commit();
    } else if (mode == map_file) { 
	if (size > 0 && msync(base, size, MS_ASYNC) != ok) { 
	    TRACE_MSG(("post_file::flush: failed to flush file: %s\n",
		       strerror(error_code)));
	    return false;
	}
	error_code = ok;
	return true;
    } else { 
	int new_fd = ::open(tmp_name, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (new_fd < 0) { 
	    error_code = errno;
	    return false;
	}
	if ((size_t)write(new_fd, base, size) != size) { 
	    error_code = errno;
	    ::close(new_fd);
	    return false;
	}
	::close(new_fd);
	if (rename(tmp_name, name) != ok) { 
	    error_code = errno;
	    return false;
	}
	error_code = ok;
	return true;
    }
}

bool post_file::close()
{
    if (base == NULL) { 
	if (fd >= 0 && ::close(fd) != ok) { 
	    error_code = errno;
	    TRACE_MSG(("post_file::close: failed to close file: %s\n",
		       strerror(error_code)));
	    return false;
	}
	return true;
    }
    if (mode == shadow_pages_transaction) { 
	post_file *fp, **fpp = &chain;
	while ((fp = *fpp) != this) { 
	    fpp = &fp->next;
	}
	*fpp = fp->next;
	    
	if (n_locked_pages != 0 && !flush_log_buffer()) { 
	    return false;
	}
    }
    if (munmap(base, mapped_size) != ok) { 
	error_code = errno;
	TRACE_MSG(("post_file::close: failed to unmap memory segment: %s\n",
		   strerror(error_code)));
	return false;
    }
    if (fd >= 0) {
	if (size != allocated_size) { 
	    if (ftruncate(fd, size) != ok) {
		error_code = errno;
		TRACE_MSG(("post_file::close: failed to truncate file: %s\n",
			   strerror(error_code)));
		return false;
	    }
	}
	if (::close(fd) != ok) {
	    error_code = errno;
	    TRACE_MSG(("post_file::close: failed to close file: %s\n",
		       strerror(error_code)));
	    return false;
	}
    }
    if (mode == shadow_pages_transaction) { 
	if (::close(log) != ok) { 
	    TRACE_MSG(("post_file::close: failed to close transaction log file: "
		       "%s\n", strerror(errno)));
	}
	if (unlink(log_name) != ok) {
	    TRACE_MSG(("post_file::close: failed to remove transaction log file: "
		       "%s\n", strerror(errno)));
	}
    }
    error_code = ok;
    return true;	
}

char* post_file::get_program_timestamp()
{
    static char buf[32];
    if (program_compilation_time == NULL) { 
	char* image = getenv("_");
	assert(image != NULL);
	struct stat fs;
	int rc = stat(image, &fs);
	assert(rc == 0);
	struct tm* fmtime = localtime(&fs.st_mtime);
	sprintf(buf, "%02d.%02d.%04d %02d:%02d.%02d", 
		fmtime->tm_mday, fmtime->tm_mon, fmtime->tm_year+1900,
		fmtime->tm_hour, fmtime->tm_min, fmtime->tm_sec);
	return buf;
    }
    return program_compilation_time;
}

#endif // Unix


#if 0 && DEBUG_LEVEL >= DEBUG_CHECK
//
// Sometimes it is more convenient to catch SIGSEGV instead of SIGABRT. 
// For example in Digital Unix debugger failed to unroll stack after
// assertion failure. And in Windows NT assertion failure will not cause
// invocation of debugger if program is not started from MSDEV. That is
// why this "strange" version of abort() was implemented. 
//
#ifndef _DEBUG
extern "C" 
void abort() { while(1) *(int*)0 = 0; /* do not return */ }
#endif

#endif

END_POST_NAMESPACE



