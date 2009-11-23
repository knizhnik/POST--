//-< FILE.H >--------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 18-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Mapped on memory file interface
//-------------------------------------------------------------------*--------*

#ifndef __FILE_H__
#define __FILE_H__

#include "stdtp.h"

BEGIN_POST_NAMESPACE

struct file_header { // first two words of file
    void*  base_address;
    size_t file_size;
};

class POST_DLL_ENTRY post_file { 
  public:
    typedef char msg_buf[256]; // /buffer for get_error_text

    size_t get_size() const { return size; }
    char*  get_base() const { return base; }

    enum open_mode { 
	shadow_pages_transaction, 
	copy_on_write_map, 
	load_in_memory,
	map_file
    };
    enum access_prot { 
	read_only, 
	read_write 
    };

    bool   open(open_mode mode, access_prot prot);
    bool   set_size(size_t new_size);
    bool   set_protection(access_prot prot);
    bool   flush();
    bool   commit();
    bool   rollback();
    bool   close();
    char*  get_error_text(char* buf, size_t buf_size);

    static char* get_program_timestamp();
    
    post_file(const char* name, size_t max_file_size, void* default_base_address, size_t max_locked_pages);
    ~post_file();

  protected:
    static post_file*  chain;
    post_file*         next; // chain of opened files

    char*         name;
    char*         log_name;
    char*         tmp_name;
    char*         sav_name;
    
    char*         base;
    size_t        size; // size of file
    size_t        mapped_size; // size of mapped segment
    size_t        allocation_granularity;
    size_t        page_size;
    int           error_code;
    
    size_t        max_file_size; // file extension limitiation 
    //
    // Maximal number of locked in memory pages, Locking of pages in memory is
    // used to provide buffering pf shadow pages writes to transaction log.
    // 
    size_t        max_locked_pages; 

    size_t        allocated_size;

    char*         log_buffer;
    char**        locked_page;
    size_t        n_locked_pages;
    
    open_mode     mode;
    access_prot   prot;

    enum app_error_codes { 
	ok = 0,  
	file_size_not_aligned  = -1, 
	file_mapping_size_exceeded = -2,
	not_in_transaction = -3,
	end_of_file = -4
    };

    bool create_shadow_page(int modify, void* addr);
    bool flush_log_buffer();
    bool recover_file();

    void set_file_name(const char* name);

#ifdef _WIN32
    HANDLE fd;
    HANDLE md;
    HANDLE log;
    bool   recovery;
    char*  vmem;
    int    platform;
    int*   dirty_page_map;

    bool read_file_in_memory();
    bool write_dirty_pages_in_file();

  public:
    // this method have to be public because it is called from system 
    // dependent signal handler
#ifndef _WIN64
    static bool handle_page_access(DWORD* params);
#else
    static bool handle_page_access(ULONG_PTR* params);
#endif

#else // Unix
    int    fd;
    int    log;
    size_t file_extension_granularity;

  public:
    // this method have to be public because it is called from system 
    // dependent signal handler
    static bool handle_page_modification(void* addr);
#endif
};

//
// In transaction mode access violation exception is used to implement
// shadow page transactions. By default this exception is catched by
// unhandled exception filter. But if you want to use debugger, you 
// shoud use structured exception handling. You should always use
// structured exception handling with Borland C++, because Unhandled Exception 
// Filter is not correctly called in Borland. 
// Please use two following macros to enclose body of main (or WinMain)
// function:

#if defined(_WIN32) && !defined(__MINGW32__)
#define SEN_TRY __try
#define SEN_ACCESS_VIOLATION_HANDLER() \
__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION \
  && post_file::handle_page_access((GetExceptionInformation())-> \
			      ExceptionRecord->ExceptionInformation) \
  ? EXCEPTION_CONTINUE_EXECUTION \
  : EXCEPTION_CONTINUE_SEARCH) {}

#if 0 && defined(__BORLANDC__)
#define __try    try
#define __except except
#endif

#else // Unix
#define SEN_TRY
#define SEN_ACCESS_VIOLATION_HANDLER()
#endif

END_POST_NAMESPACE

#endif
