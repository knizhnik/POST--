//-< CLASSINFO.CXX >-------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  2-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Implementation of class descriptor methods
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "classinfo.h"

BEGIN_POST_NAMESPACE

const unsigned class_hash_table_size = 117;

class_descriptor* class_descriptor::hash_table[class_hash_table_size];
class_descriptor* class_descriptor::list;
int               class_descriptor::count;

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

class_descriptor::class_descriptor(const char* class_name,
				   size_t class_size,
				   void (*default_constructor)(object*)) 
: id(count++), size(class_size)
{ 
    assert(!find_class(class_name));
    name = class_name;
    constructor = default_constructor;
    next = list;
    list = this;
    unsigned h = string_hash_function(class_name) % class_hash_table_size;
    collision_chain = hash_table[h];
    hash_table[h] = this;
}


class_descriptor* class_descriptor::find_class(char const* class_name)
{
    unsigned h = string_hash_function(class_name) % class_hash_table_size;
    class_descriptor* cp;
    for (cp = hash_table[h]; 
	 cp != NULL && strcmp(cp->name, class_name) != 0; 
	 cp = cp->collision_chain);
    return cp;
}

END_POST_NAMESPACE
