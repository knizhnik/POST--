//-< POSTNEW.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     16-Nov-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 16-Nov-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Redefined global operators new and delete
//-------------------------------------------------------------------*--------*

#include "object.h"

BEGIN_POST_NAMESPACE

#ifdef DO_NOT_REDEFINE_MALLOC

void* operator new(size_t size) { 
    storage* store = storage::get_current_storage();
    if (store == NULL) { 
	return malloc(size);
    } else { 
	return post_raw_object::create(*store, size);
    }
}

void* operator new[](size_t size) { 
    storage* store = storage::get_current_storage();
    if (store == NULL) { 
	return malloc(size);
    } else { 
	return post_raw_object::create(*store, size);
    }
}

void operator delete(void* ptr) 
throw() 
{
    storage* store = storage::find_storage((object*)ptr);
    if (store != NULL) { 
	store->free((object*)ptr);
    } else { 
	free(ptr);
    }
}

void operator delete[](void* ptr) 
throw() 
{
    storage* store = storage::find_storage((object*)ptr);
    if (store != NULL) { 
	store->free((object*)ptr);
    } else { 
	free(ptr);
    }
}

#else

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <string.h>

extern "C" void* malloc(size_t size)
{
    storage* store = storage::get_current_storage();
    if (store == NULL) { 
#ifdef _WIN32
	return VirtualAlloc(NULL, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
#else
	return sbrk(size);
#endif
    } else { 
	return post_raw_object::create(*store, size);
    }
}

extern "C" void free(void* ptr) 
{
    storage* store = storage::find_storage((object*)ptr);
    if (store != NULL) { 
	store->free((object*)ptr);
    } else { 
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#endif
    }
}

extern "C" void* realloc(void* ptr, size_t size)
{
    void* new_ptr = malloc(size);
    object* obj = (object*)ptr;
    memcpy(new_ptr, ptr, storage::find_storage(obj)->get_header(obj)->size);
    free(ptr);
    return new_ptr;
}

extern "C" void* calloc(size_t nmemb, size_t size)
{
    void* ptr = malloc(nmemb*size);
    memset(ptr, 0, nmemb*size);
    return ptr;
}
#ifdef _WIN32
extern "C" void* _nh_malloc(size_t size)
{
    return malloc(size);
}
#endif

#endif

END_POST_NAMESPACE

