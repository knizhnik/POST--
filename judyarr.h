//-< JUDY.H >--------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     15-Mar-2009  K.A. Knizhnik  * / [] \ *
//                          Last update: 15-Mar-2009  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Judy array interface 
//-------------------------------------------------------------------*--------*

#ifndef __JUDYARR_H__
#define __JUDYARR_H__

#include "object.h"

BEGIN_POST_NAMESPACE

template<class Index, class Value, class Position>
class POST_DLL_ENTRY judy_array : public object { 
  private:
    void* root;

  public:
    /**
     * Get element at specified index 
     * @param index index in the array
     * @return element value
     */
    Value get(Index index);
    
    /**
     * Set element at specified index
     * @param index array element index
     * @param value array element value
     * @return previous array element value
     */     
    Value set(Index index, Value value);

    /**
     * Put element at specified index
     * @param index array element index
     * @return true if element was successfully deleted, false if it is not present in array
     */     
    bool remove(Index index);

    /**
     * Search (inclusive) for the first index present that is equal to or greater than the passed index.
     * @param index in: index to start search, out: index of the first element 
     * @return first element value or 0 if array is empty
     */
    Value first(Position index);

    /**
     * Search (inclusive) for the last index present that is equal to or less than the passed index.
     * @param index in: index to start search, out: index of the last element 
     * @return last element value or 0 if array is empty
     */
    Value last(Position index);

    /**
     * Search (exclusive) for the next index present that is greater than the passed index.
     * @param index in: index to continue search, out: index of the next element 
     * @return next  element value or 0 if end of array is reached
     */     
    Value next(Position index);

    /**
     * Search (exclusive) for the previous index present that is less than the passed index.
     * @param index in: index to continue search, out: index of the previous element 
     * @return next  element value or 0 if beginning of array is reached
     */     
    Value prev(Position index);

    judy_array() { 
        root = 0;
    }
    
    ~judy_array();

    static judy_array* create() { 
        return (judy_array*)post_raw_object::create(*storage::get_current_storage(), sizeof(judy_array));
    }
};

typedef judy_array<const char*,object*,char*> dictionary;
typedef judy_array<long,object*,long&> sparse_array;
typedef judy_array<long,bool,long&>    bit_array;

END_POST_NAMESPACE

#endif
