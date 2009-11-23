//-< ITERATOR.H >----------------------------------------------------*--------*
//                          Created:     05-Apr-03    Johan Sorensen * / [] \ *
//                          Last update: 05-Apr-03    Johan Sorensen * GARRET *
//-------------------------------------------------------------------*--------*
// Iterator implementation
//-------------------------------------------------------------------*--------*

#ifndef __ITERATOR_H__
#define __ITERATOR_H__

#include "ttree.h"
#include "dnmarr.h"

BEGIN_POST_NAMESPACE

// it should be possible (and quite easy) to make iterators for any kind of container
// using this generic "interface"

template<class T>
class basic_iterator
{
  public:
    virtual bool IsValid () const = 0;

    virtual T* operator* () = 0;
    virtual T* operator++ () = 0;
    virtual T* operator-- () = 0;
};


// here is a concrete instantiation for the Ttree container

class Ttree_iterator : public basic_iterator<object>
{
  public:
    Ttree_iterator(dbTtree *t, 
                   bool bReverseOrder = false); // for backwards traversal
    virtual~Ttree_iterator();

    bool IsValid () const;

    object* operator* ();
    object* operator++ ();
    object* operator-- ();

  private:
    dbTtreeNode*  m_CurrNode;
    long          m_CurrPos;      // -1 --> "invalid"
    dnm_array<dbTtreeNode*> m_NodeStack;
    dnm_array<long>         m_PositionStack;
};

END_POST_NAMESPACE

#endif  // __ITERATOR_H

