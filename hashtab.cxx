//-< HASHTAB.H >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      7-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  7-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Hash table: example of POST++ class definition
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "hashtab.h"

BEGIN_POST_NAMESPACE

inline unsigned string_hash_function(const char* name)
{ 
    unsigned h = 0, g;
    while(*name) { 
	h = (h << 4) + *name++;
	if ((g = h & 0xF0000000) != 0) { 
	    h ^= g >> 24;
	}
	h &= ~g;
    }
    return h;
}

void hash_table::put(const char* name, object* obj)
{
    unsigned h = string_hash_function(name) % size;
    table[h] = new (hash_item::self_class, *get_storage(), strlen(name))
	           hash_item(name, obj, table[h]);
}

object* hash_table::get(const char* name) const
{
    for (hash_item* ip = table[string_hash_function(name) % size];
	 ip != NULL; 
	 ip = ip->next)
    {
	if (strcmp(ip->name, name) == 0) { 
	    return ip->obj;
	}
    }
    return NULL;
}

bool hash_table::del(const char* name)
{
    hash_item *ip, **ipp;
    ipp = &table[string_hash_function(name) % size];
    while ((ip = *ipp) != NULL && strcmp(ip->name, name) != 0) { 
	ipp = &ip->next;
    }
    if (ip != NULL) { 
	*ipp = ip->next;
	delete ip;
	return true;
    }
    return false;
}

bool hash_table::del(const char* name, object* obj)
{
    hash_item *ip, **ipp;
    ipp = &table[string_hash_function(name) % size];
    while ((ip=*ipp) != NULL && (ip->obj != obj || strcmp(ip->name,name) != 0))
    {
	ipp = &ip->next;
    }
    if (ip != NULL) { 
	*ipp = ip->next;
	delete ip;
	return true;
    }
    return false;
}

void hash_table::purge(bool delete_objects)
{
    for (int i = size; --i >= 0;) { 
	hash_item* ip = table[i];
	while (ip != NULL) { 
	    hash_item* next = ip->next;
	    if (delete_objects) { 
		delete ip->obj;
	    }
	    delete ip;
	    ip = next;
	}
	table[i] = NULL;
    }
}

REGISTER(hash_item);
REGISTER(hash_table);

END_POST_NAMESPACE
