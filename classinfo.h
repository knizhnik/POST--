//-< CLASSINFO.H >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Class information maintaining
//-------------------------------------------------------------------*--------*

#ifndef __CLASSINFO_H__
#define __CLASSINFO_H__

#include "storage.h"

BEGIN_POST_NAMESPACE

class object;

class POST_DLL_ENTRY class_descriptor { 
    friend class storage;
  protected:
    static class_descriptor* hash_table[];
    static int count;
    class_descriptor* collision_chain;
    static class_descriptor* list;
    class_descriptor* next;

    void (*constructor)(object*);
    
  public:
    const char*  name;
    const int    id;
    const size_t size; // size of fixed part of the class

    static class_descriptor* find_class(char const* class_name);

    class_descriptor(const char* name, size_t size, void (*cons)(object*));
};

#define REF(x) \
   if (gc_stack) { gc_stack[sp].ptr=(object**)&x; gc_stack[sp].len=1; sp+=1; }\
   else if (x != NULL) { *(char**)&x += shift; }
	         
#define REFS(x) \
   if (gc_stack) { \
       gc_stack[sp].ptr = (object**)x; gc_stack[sp].len = items(x); sp += 1; \
   } else { size_t len = items(x); char** p = (char**)x; \
       do { if (*p != NULL) { *p += shift; } p += 1; } while (--len != 0); \
   }

#define VREFS(x) { \
   size_t len = \
       (object**)((char*)this+((object_header*)this-1)->size) - (object**)x; \
   if (gc_stack) { \
       gc_stack[sp].ptr = (object**)x; gc_stack[sp].len = len; sp += 1; \
   } else { \
       char** p = (char**)x; \
       while (len != 0) { if (*p != NULL) { *p += shift; } len -= 1; p += 1; } \
   } \
}

#define NO_REFS  ;

#define CLASSINFO(NAME, FIELD_LIST) \
    NAME() { \
	int sp = 0; \
	gc_segment* const gc_stack = storage::gc_stack; \
	ptrdiff_t const shift = storage::base_address_shift; \
	if (gc_stack != NULL) { FIELD_LIST storage::gc_stack=&gc_stack[sp]; } \
	else if (shift != 0) { FIELD_LIST } \
    } \
    static void constructor(object* ptr); \
    static class_descriptor self_class

#define REGISTER(NAME) \
    void  NAME::constructor(object* ptr) { new (ptr) NAME; } \
    class_descriptor NAME::self_class(#NAME, sizeof(NAME), &NAME::constructor)

// ---------------------------------------------------------------------------
//	REGISTER_WITH_VARIABLE_ARRAY_OF_OBJECTS
//		- a macro which is can be used instead of 'REGISTER' when a class
//		  has an variable length array of objects (not object pointers!)
//		- calls default constructor on all elements of the variable length 
//		  array (except the first).
//
#define	REGISTER_WITH_VARIABLE_ARRAY_OF_OBJECTS(NAME, ARRAY_TYPE, ARRAY)	\
    void NAME::constructor(object* ptr) {			                \
        NAME* p = new (ptr) NAME;				                \
        ARRAY_TYPE* end = (ARRAY_TYPE*)((char*)p + ((object_header*)p-1)->size);\
        ARRAY_TYPE* start = p->ARRAY;			                        \
        while (++start < end) {  						\
	    new (start) ARRAY_TYPE;               				\
        }	                                                                \
    }									        \
    class_descriptor NAME::self_class(#NAME, sizeof(NAME), &NAME::constructor)

#define REGISTER_TEMPLATE(NAME) \
    template<> void NAME::constructor(object* ptr) { new (ptr) NAME; } \
    template<> class_descriptor NAME::self_class(#NAME, sizeof(NAME), &NAME::constructor)

END_POST_NAMESPACE

#endif
