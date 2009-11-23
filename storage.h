//-< STORAGE.H >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 21-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Storage interface
//-------------------------------------------------------------------*--------*

#ifndef __STORAGE_H__
#define __STORAGE_H__

#define POST_VERSION 147 // 1.47

#include "file.h"

BEGIN_POST_NAMESPACE

const size_t defaultMaxDatabaseSize = 8*1024*1024;

const int    maxStaticData = 32;

#if defined(_WIN32)
    class POST_DLL_ENTRY simple_mutex { 
	CRITICAL_SECTION cs;
      public:
	void enter() { 
	    EnterCriticalSection(&cs); 
	}
	void leave() { 
	    LeaveCriticalSection(&cs); 
	}
	simple_mutex() { 
	    InitializeCriticalSection(&cs);
	}
	~simple_mutex() { 
	    DeleteCriticalSection(&cs);
	}	
    };
#elif defined(_REENTRANT)
    #include <pthread.h>
    class simple_mutex { 
	pthread_mutex_t cs;
      public:
	void enter() { 
	    pthread_mutex_lock(&cs);
	}
	void leave() { 
	    pthread_mutex_unlock(&cs);
	}
	simple_mutex() { 
	    pthread_mutex_init(&cs, NULL);
	}
	~simple_mutex() { 
	    pthread_mutex_destroy(&cs);
	}	
    };
#else
    class simple_mutex { 
      public:
	void enter() {}
	void leave() {}
    };
#endif

class critical_section { 
   simple_mutex& mutex;
 public:
   critical_section(simple_mutex& cs) : mutex(cs) { 
       cs.enter();
   }
   ~critical_section() { 
       mutex.leave();
    }
};

#define CS(mutex) critical_section __cs(mutex)
#define CS2(mutex1, mutex2) critical_section __cs1(mutex1), __cs2(mutex2)

class object;
class class_descriptor;

struct object_header { 
    enum { 
	free_object   = 0,
	metaclass_cid = 1,
	page_map_cid  = 2,
	first_cid     = 3,
	gc_marked     = 0x80000000 
    };
    int      cid;
    unsigned size;
};  

struct storage_free_block : public object_header { 
    storage_free_block* next;
};       

class storage_class_descriptor;

class storage_page_map : public object_header { 
  public: 
    //
    // All pages except first occupied by large object are marked with non-zero
    // bit in this bitmap. Bits correspond to all other pages are zero.
    //
    char bits[1];
};

struct POST_DLL_ENTRY storage_header : public file_header { 
    object* root_object;

    unsigned n_classes;
    storage_class_descriptor* class_dictionary;
    int     version;

    storage_page_map* page_map;

    storage_free_block* free_page_chain;
#ifdef LARGE_OBJECTS
    storage_free_block* free_block_chain[18];
#else
    storage_free_block* free_block_chain[13];
#endif
    char    timestamp[32];

    int     n_static_data;
    object* static_data[maxStaticData];

    void    adjust_references(ptrdiff_t shift);
};

struct gc_segment { 
    object** ptr;
    size_t   len;
};

class POST_DLL_ENTRY storage { 
    friend class object;
  public:
    static ptrdiff_t base_address_shift;
    static gc_segment* gc_stack;

    enum open_flags { 
	use_transaction_log       = 0x01, // apply all changes as transactions
	no_file_mapping           = 0x02, // do not use file mapping mechanism
	fault_tolerant            = 0x04, // preserve storage consistency
	read_only                 = 0x08, // read only file view
	support_virtual_functions = 0x10, // storage objects contain virt.func.
	do_garbage_collection     = 0x20, // perform GC on loading
	fixed                     = 0x40  // always map to the same address
    };
    virtual bool open(int flags = 0
#ifdef LARGE_OBJECTS
                      , int dummy = 0 // this parameter is added only to ensure that both POST library and user application 
                                      // were compiled with equal state of LARGE_OBJECTS macro
#endif
                      );
    //
    // Commit current transaction: save all modified pages in data file and
    // truncate transaction log. This function works only in 
    // in transaction mode (when flag use_transaction_log is set).
    // Commit is implicitly called by close().
    //
    virtual void commit(); 
    //
    // Rollback current transaction. This function works only in 
    // in transaction mode (when flag use_transaction_log is set).
    //
    virtual void rollback(); 
    //
    // Flush all changes on disk. 
    // Behavior of this function depends on storage mode:
    // 1. In transaction mode this function is equivalent to commit(). 
    // 2. In other modes this functions performs atomic update of storage file 
    //    by flushing data to temporary file and then renaming it to original 
    //    one. All modified objects will not be written in data file until 
    //    flush().
    //
    virtual void flush(); 

    //
    // Close storage. To store modified objects you should first call flush(),
    // otherwise all changes will be lost.
    //
    virtual void close();
    //
    // Free all objects unreachable from the storage root object.
    // This function will be called from open() if 'do_garbage_collection'
    // attribute is specified. It is also possible to call this function 
    // explicitly, but you should be sure that no program variable points 
    // to object unreachable from the storage root. 
    // Method returns number of deallocated objects. Even if you are using
    // explicit memory deallocation you can call this method to search for
    // memory leak and check reference consisteny.
    //
    virtual size_t garbage_collection();

    //
    // Report error in storage method "func". Applciation can override this
    // method to perform application dependent error handling.
    // Default implementation of this method out put message and abort 
    // application
    // 
    virtual void handle_error(const char* func);

    void set_root_object(void* obj) { 
	hdr->root_object = (object*)obj; 
    }

    void* get_root_object() const { 
	return hdr->root_object; 
    }

    void set_version(int version) { 
	hdr->version = version; 
    }

    int get_version() const { 
	return hdr->version; 
    }

    void begin_static_data() { 
	static_data = true;
	static_data_index = 0;
    }

    void end_static_data() { 
	static_data = false;
    }
    
    void* get_static_data() { 
	if (static_data && static_data_index < hdr->n_static_data) { 
	    return hdr->static_data[static_data_index++];
	}
	return NULL;
    }
    
    void set_static_data(void* obj) { 
	if (static_data) { 
	    assert(hdr->n_static_data < maxStaticData);
	    hdr->static_data[static_data_index] = (object*)obj;
	    hdr->n_static_data = ++static_data_index;
	}
    }

    bool is_static_data(void* obj) { 
	for (int i = hdr->n_static_data; --i >= 0; ) { 
	    if (hdr->static_data[i] == obj) { 
		return true;
	    }
	}
	return false;
    }


    static storage* find_storage(object const* obj) { 
	for (storage* sp = chain; sp != NULL; sp = sp->next) { 
	    if (size_t((char*)obj - (char*)sp->hdr) < sp->hdr->file_size) { 
		return sp;
	    }
	} 
	return NULL;
    }
    
    static storage* get_current_storage() { 
	assert(chain == NULL || chain->next == NULL);
	return chain;
    }

    class_descriptor* get_object_class(int cid) const { 
	return dbs_class_mapping[cid];
    }

    bool has_object(object const* obj) const { 
	return size_t((char*)obj - (char*)hdr) < hdr->file_size;
    }

    char* get_error_text(char* buf, size_t buf_size);

    object_header* get_header(object const* obj);

    //
    // Allocate object in storage.
    //
    void* allocate(int app_class_id, unsigned size) { 
	CS(mutex);
	return _allocate(app_class_id, size);
    }

    //
    // Explicit deallocation of object in storage.
    //
    void  free(object* obj) { 
	CS(mutex);
	object_header* hp = get_header(obj);
	if (hp->size + sizeof(object_header) <= page_size/2) {
	    int n = block_chain[((hp->size+sizeof(object_header)+7) >> 3) - 1];
	    storage_free_block* bp = (storage_free_block*)hp;
	    bp->cid = object_header::free_object;
	    bp->next = hdr->free_block_chain[n];
	    hdr->free_block_chain[n] = bp;
	} else { 
	    free_page(hp);
	}
    }

    //
    // Parameter max_file_size specifies maximal data file extension for 
    // this storage.
    // Parameter max_locked_pages is used only in transaction mode and 
    // specifies number of pages which can be locked in memory to provide
    // buffering sadow pages writes to the log file. For Windows-NT number
    // of locked pages by default should not be greater then 30. 
    // If larger number of pages is specified, POST++ will try to extend 
    // process working set.
    //
    storage(const char* name, 
	    size_t max_file_size = defaultMaxDatabaseSize,
            void* default_base_address = NULL, 
	    size_t max_locked_pages = 0)
    : data_file(name, max_file_size, default_base_address, max_locked_pages) 
    {
	static_data = false;
    }

    virtual~storage();

  protected:
    post_file data_file;
    storage_header* hdr;
    storage_free_block* curr_free_page;

    storage* next;
    static storage* chain; // chain of opened storages

    class_descriptor** dbs_class_mapping;
    int*               app_class_mapping;
    
    simple_mutex mutex;
    static simple_mutex global_mutex;

    enum { 
#ifdef LARGE_OBJECTS
	page_size = 1024,       // bytes
#else
	page_size = 512,        // bytes
#endif
	init_page_map_size = 8  // pages
    };

    bool static_data;
    int  static_data_index;

    static const int block_size[page_size/16];
    static const int block_chain[page_size/16];

    void* alloc_page(int cid, unsigned size);
    void  free_page(object_header* hp);

    virtual void adjust_references(ptrdiff_t shift);
    virtual size_t garbage_collection(ptrdiff_t shift);
    virtual void load_class_dictionary();
    virtual bool update_class_dictionary();


    void* _allocate(int app_class_id, unsigned size) { 
	int cid = app_class_mapping[app_class_id];
	assert(cid != 0);
	if (size + sizeof(object_header) <= page_size/2) { 
	    int n = block_chain[((size + sizeof(object_header) + 7) >> 3) - 1];
	    storage_free_block* bp = hdr->free_block_chain[n];
	    if (bp != NULL) { 
		hdr->free_block_chain[n] = bp->next;
 		bp->size = size;
		bp->cid = cid;
		return (object_header*)bp+1;
	    }
	}
	return alloc_page(cid, size);
    }

    void  call_constructor(object_header* hp); 
};

END_POST_NAMESPACE

#endif
