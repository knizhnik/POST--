/*-< OBJECT.H >------------------------------------------------------*--------*
 * POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
 * (Persistent Object Storage)                                       *   /\|  *
 *                                                                   *  /  \  *
 *                          Created:      4-Jun-2002  K.A. Knizhnik  * / [] \ *
 *                          Last update:  4-Jun-2002  K.A. Knizhnik  * GARRET *
 *-------------------------------------------------------------------*--------*
 * C API for POST++
 *-------------------------------------------------------------------*--------*/

#ifndef __POST_C_API_H__
#define __POST_C_API_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" { 
#endif

#ifdef POST_DLL
#ifdef INSIDE_POST
#define POST_DLL_ENTRY __declspec(dllexport)
#else
#define POST_DLL_ENTRY __declspec(dllimport)
#endif
#else
#define POST_DLL_ENTRY
#endif


typedef struct post_storage post_storage;
    
typedef int post_bool;

/**
 * Create instance of POST++ storage
 */
POST_DLL_ENTRY post_storage* post_create_storage(char const* name, size_t max_file_size);


/**
 * Open flags
 */
enum post_storage_open_flags {
    use_transaction_log       = 0x01, /* apply all changes as transactions */
    no_file_mapping           = 0x02, /* do not use file mapping mechanism */
    fault_tolerant            = 0x04, /* preserve storage consistency */
    read_only                 = 0x08  /* read only file view */
};
    
/**
 * Open POST storage.
 */
post_bool POST_DLL_ENTRY post_open(post_storage* storage, int flags);

/**
 * Commit current transaction: save all modified pages in data file and
 * truncate transaction log. This function works only in 
 * in transaction mode (when flag use_transaction_log is set).
 * Commit is implicitly called by close().
 */
void POST_DLL_ENTRY post_commit(post_storage* storage);

/**
 * Rollback current transaction. This function works only in 
 * in transaction mode (when flag use_transaction_log is set).
 */
void POST_DLL_ENTRY post_rollback(post_storage* storage);

/**
 * Flush all changes on disk. 
 * Behavior of this function depends on storage mode:
 * 1. In transaction mode this function is equivalent to commit(). 
 * 2. In other modes this functions performs atomic update of storage file 
 *    by flushing data to temporary file and then renaming it to original 
 *    one. All modified objects will not be written in data file until 
 *    flush().
 */
void POST_DLL_ENTRY post_flush(post_storage* storage);

/**
 * Close storage. To store modified objects you should first call flush(),
 * otherwise all changes will be lost.
 */
void POST_DLL_ENTRY post_close(post_storage* storage);

/**
 * Set storage root object
 */
void POST_DLL_ENTRY post_set_root_object(post_storage* storage, void* obj); 

/**
 * Get storage root object
 */
POST_DLL_ENTRY void* post_get_root_object(post_storage* storage);


/**
 * Allocate new object in storage
 */
POST_DLL_ENTRY void* post_alloc(post_storage* storage, size_t size);

/**
 * Deallocate new object in storage
 */
void POST_DLL_ENTRY  post_free(post_storage* storage, void* obj);

/**
 * Delete POST storage
 */
void POST_DLL_ENTRY post_delete_storage(post_storage* storage);

#ifdef __cplusplus
}
#endif

#endif
