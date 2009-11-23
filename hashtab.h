//-< HASHTAB.H >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      7-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  7-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Hash table
//-------------------------------------------------------------------*--------*

#ifndef __HASHTAB_H__
#define __HASHTAB_H__

#include "object.h"

BEGIN_POST_NAMESPACE

class POST_DLL_ENTRY hash_item : public object { 
  public: 
    hash_item* next;
    object*    obj;
    char       name[1];

    CLASSINFO(hash_item, REF(next) REF(obj));

    hash_item(const char* name, object* obj, hash_item* chain) { 
	next = chain;
	this->obj = obj;
	strcpy(this->name, name);
    }
};

class POST_DLL_ENTRY hash_table : public object { 
  protected: 
    size_t     size;
    hash_item* table[1];

    hash_table(size_t hash_size) { 
	size = hash_size;
	memset(table, 0, size*sizeof(hash_item*));
    }
  public: 
    //
    // Add to hash table association of object with specified name.
    //
    void    put(const char* name, object* obj);
    //
    // Search for object with specified name in hash table.
    // If object is not found NULL is returned.
    //
    object* get(const char* name) const;
    //
    // Remove object with specified name from hash table.
    // If there are several objects with the same name the one last inserted
    // is removed. If such name was not found 'false' is returned.
    //
    bool    del(const char* name);
    //
    // Remove concrete object with specified name from hash table.
    // If such name was not found or it is associated with different object
    // 'false' is returned. If there are several objects with the same name, 
    // all of them are compared with specified object.
    //    
    bool    del(const char* name, object* obj);

    void    purge(bool delete_objects = false);

    ~hash_table() { purge(); }

    CLASSINFO(hash_table, VREFS(table));

    static hash_table* create(storage& store, size_t size) { 
	return new (self_class, store, (size-1)*sizeof(hash_item*)) 
	           hash_table(size);
    }
}; 

END_POST_NAMESPACE

#endif


