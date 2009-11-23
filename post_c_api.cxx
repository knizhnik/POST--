/*-< OBJECT.H >------------------------------------------------------*--------*
 * POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
 * (Persistent Object Storage)                                       *   /\|  *
 *                                                                   *  /  \  *
 *                          Created:      4-Jun-2002  K.A. Knizhnik  * / [] \ *
 *                          Last update:  4-Jun-2002  K.A. Knizhnik  * GARRET *
 *-------------------------------------------------------------------*--------*
 * C API for POST++
 *-------------------------------------------------------------------*--------*
 */

#define INSIDE_POST

#include "object.h"
#include "post_c_api.h"

USE_POST_NAMESPACE

post_storage* post_create_storage(char const* name, size_t max_file_size)
{
    return (post_storage*)new storage(name, max_file_size);
}


post_bool post_open(post_storage* s, int flags)
{
    return ((storage*)s)->open(flags | storage::fixed);
}

void post_commit(post_storage* s) { 
    ((storage*)s)->commit();
}

void post_rollback(post_storage* s) { 
    ((storage*)s)->rollback();
}

void post_flush(post_storage* s) { 
    ((storage*)s)->flush();
}
    
void post_close(post_storage* s) { 
    ((storage*)s)->close();
}

void post_set_root_object(post_storage* s, void* obj) {
    ((storage*)s)->set_root_object(obj);
}

void* post_get_root_object(post_storage* s) { 
    return ((storage*)s)->get_root_object();
}
   
void* post_alloc(post_storage* s, size_t size) { 
    return post_raw_object::create(*(storage*)s, size);
}
    
void  post_free(post_storage* s, void* obj) { 
    ((storage*)s)->free((object*)obj);
}

void post_delete_storage(post_storage* s) {
    delete (storage*)s;
}
