//-< TEXTOBJ.H >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     12-Mar-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 12-Mar-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Text object: example of POST++ class definition
//-------------------------------------------------------------------*--------*

#ifndef __TXTOBJ_H__
#define __TXTOBJ_H__

#include "object.h"

BEGIN_POST_NAMESPACE

class POST_DLL_ENTRY search_ctx { 
    friend class text;
  protected:
    enum { max_patterns = 32 };

    int    shift[256];
    int    min_length;    // length of shortest pattern - 1
    int    diff;          // difference between longest and shortest length
    int    pattern_length[max_patterns];

  public:  
    char** const pattern;
    int const n_patterns; 
    int const exact_mask; // mask to specify which patterns should not compared
                          // without ignoring case of character 
    int const and_mask;   // mask specifing which patterns should be all 
                          // present in text. If more than 1 bit is set in this
                          // mask search function will not set position field. 


    int    position;      // match position, -1 if not found

    bool   search(const char* p_text, int text_length);

    void   reset() { // start search from the beginning
	position = -1;
    }

    search_ctx(char* patterns[], // patterns will not be copied and will be 
	       // modified if case insensetive comparison is used
	       int n_patterns = 1,
	       int and_mask   = 1,
	       int exact_mask = 1);
};


class POST_DLL_ENTRY text : public object { 
  protected:
    text(const char* txt, size_t len);
  public:
    int  length;
    char data[1];

    //
    // Create text object of 'len' size by coping data from 'txt' buffer
    //
    static text* create(storage& store, const char* buf, size_t len);    
    //
    // Extract text from file starting from 'offs' and at most 'len' bytes long
    //
    static text* create(storage& store, FILE* f, size_t offs, size_t len);
    //
    // BM-search of ctx.pattern in the text starting from ctx.position.
    // If pattern is found, ctx.position is set to position of first matched
    // character in text. If pattern is not found, ctx.position is set to -1 
    // and false is returned,
    //
    bool search(search_ctx& ctx) const { 
	return ctx.search(data, length);
    }

    CLASSINFO(text, NO_REFS);
};

END_POST_NAMESPACE

#endif
