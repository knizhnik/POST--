//-< OBJECT.H >------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 19-Apr-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Main header file. Persistent object interface.
//-------------------------------------------------------------------*--------*

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "classinfo.h"

BEGIN_POST_NAMESPACE

#define new_in(STORAGE, CLASS) new (CLASS::self_class, STORAGE) CLASS 

#ifndef UNUSED_ALWAYS
#define UNUSED_ALWAYS(x)
#endif 

class POST_DLL_ENTRY object { 
  public:
    size_t get_size() const { 
	storage* sp = storage::find_storage(this);
	CS(sp->mutex);
	return sp->get_header(this)->size; 
    }
    class_descriptor* get_classinfo() const {     
	storage* sp = storage::find_storage(this);
	CS(sp->mutex);
	return sp->get_object_class(sp->get_header(this)->cid);
    }
    storage* get_storage() const { 
	return storage::find_storage(this);
    }

    void* operator new(size_t size, class_descriptor& desc, storage& sp, 
		       size_t varying_size = 0) 
    { 
	void* p = sp.allocate(desc.id, (unsigned)(size + varying_size));
	memset(p, 0, size + varying_size);
	return p;
    }

    void* operator new(size_t, object* ptr) { return ptr; } 
    
#ifndef __BORLANDC__
    void operator delete(void* /*p*/, object* /*ptr*/) {}
 
    void operator delete(void* p, class_descriptor& /*desc*/, storage& /*sp*/, 
#if defined(__SPROV5__)
			 size_t varying_size = 0) 
#else
			 size_t varying_size) 
#endif
    {
        UNUSED_ALWAYS(varying_size);
        if (p != NULL) { 
	    object* obj = (object*)p;
   	    storage::find_storage(obj)->free(obj);
	}
    }
#endif

    void operator delete(void* p) 
    {
        if (p != NULL) { 
	    object* obj = (object*)p;
            storage* sto = storage::find_storage(obj);
            if (sto != NULL) { 
                sto->free(obj);
            }
	}
    }
};

//
// Class used for objects with unknown structure
//
class POST_DLL_ENTRY post_raw_object : public object { 
   public:
    CLASSINFO(post_raw_object, NO_REFS);
    
    static post_raw_object* create(storage& store, size_t size) 
    { 
        return new (self_class, store, size - sizeof(post_raw_object)) post_raw_object();
    }
};
    
END_POST_NAMESPACE

#endif


