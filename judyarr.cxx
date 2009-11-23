//-< JUDY.CPP >------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     15-Mar-2009  K.A. Knizhnik  * / [] \ *
//                          Last update: 15-Mar-2009  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Judy array implementation
//-------------------------------------------------------------------*--------*

#include "object.h"
#include "Judy.h"
#include "judyarr.h"

BEGIN_POST_NAMESPACE

template<>
object* dictionary::get(const char* index)
{
    Pvoid_t pvalue;
    JSLG(pvalue, root, (uint8_t*)index);
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
object* sparse_array::get(long index)
{
    Pvoid_t pvalue;
    JLG(pvalue, root, (Word_t)index);
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
bool bit_array::get(long index)
{
    int value;
    J1T(value, root, (Word_t)index);
    return value != 0;
}

template<>
object* dictionary::set(const char* index, object* value)
{
    Pvoid_t pvalue;
    JSLI(pvalue, root, (uint8_t*)index);
    object* prev = *(object**)pvalue;
    *(object**)pvalue = value;
    return prev;
}

template<>
object* sparse_array::set(long index, object* value)
{
    Pvoid_t pvalue;
    JLI(pvalue, root, (Word_t)index);
    object* prev = *(object**)pvalue;
    *(object**)pvalue = value;
    return prev;
}

template<>
bool bit_array::set(long index, bool value)
{
    int rc;
    if (value) { 
        J1S(rc, root, (Word_t)index);
    } else {
        J1U(rc, root, (Word_t)index);
    }
    return rc != 0;
}

template<>
bool dictionary::remove(const char* index)
{
    int rc;
    JSLD(rc, root, (uint8_t*)index);
    return rc != 0;
}

template<>
bool sparse_array::remove(long index)
{
    int rc;
    JLD(rc, root, (Word_t)index);
    return rc != 0;
}

template<>
bool bit_array::remove(long index)
{
    int rc;
    J1U(rc, root, (Word_t)index);
    return rc != 0;
}

template<>
object* dictionary::first(char* index)
{
    Pvoid_t pvalue;
    JSLF(pvalue, root, (uint8_t*)index);
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
object* sparse_array::first(long& index)
{
    Pvoid_t pvalue;
    Word_t word = (Word_t)index;
    JLF(pvalue, root, word);
    index = (long)word;
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
bool bit_array::first(long& index)
{
    int value;
    Word_t word = (Word_t)index;
    J1F(value, root, word);
    index = (long)word;
    return value != 0;
}

template<>
object* dictionary::last(char* index)
{
    Pvoid_t pvalue;
    JSLL(pvalue, root, (uint8_t*)index);
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
object* sparse_array::last(long& index)
{
    Pvoid_t pvalue;
    Word_t word = (Word_t)index;
    JLL(pvalue, root, word);
    index = (long)word;
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
bool bit_array::last(long& index)
{
    int value;
    Word_t word = (Word_t)index;
    J1L(value, root, word);
    index = (long)word;
    return value != 0;
}

template<>
object* dictionary::next(char* index)
{
    Pvoid_t pvalue;
    JSLN(pvalue, root, (uint8_t*)index);
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
object* sparse_array::next(long& index)
{
    Pvoid_t pvalue;
    Word_t word = (Word_t)index;
    JLN(pvalue, root, word);
    index = (long)word;
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
bool bit_array::next(long& index)
{
    int value;
    Word_t word = (Word_t)index;
    J1N(value, root, word);
    index = (long)word;
    return value != 0;
}

template<>
object* dictionary::prev(char* index)
{
    Pvoid_t pvalue;
    JSLP(pvalue, root, (uint8_t*)index);
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
object* sparse_array::prev(long& index)
{
    Pvoid_t pvalue;
    Word_t word = (Word_t)index;
    JLP(pvalue, root, word);
    index = (long)word;
    return pvalue == NULL ? NULL : *(object**)pvalue;
}

template<>
bool bit_array::prev(long& index)
{
    int value;
    Word_t word = (Word_t)index;
    J1P(value, root, word);
    index = (long)word;
    return value != 0;
}

template<>
dictionary::~judy_array()
{
    int rc;
    JSLFA(rc, root);
}

template<>
sparse_array::~judy_array()
{
    int rc;
    JLFA(rc, root);
}

template<>
bit_array::~judy_array()
{
    int rc;
    J1FA(rc, root);
}

Word_t JudyMalloc(Word_t Words)
{
    storage* store = storage::get_current_storage();
    if (store == NULL) { 
	return (Word_t)malloc(Words*sizeof(Word_t));
    } else { 
        return (Word_t)post_raw_object::create(*store, Words*sizeof(Word_t));
    }
}

void JudyFree(void * PWord, Word_t Words)
{
    storage* store = storage::find_storage((object*)PWord);
    if (store != NULL) { 
	store->free((object*)PWord);
    } else { 
	free(PWord);
    }
}    
