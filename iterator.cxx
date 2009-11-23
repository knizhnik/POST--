//-< ITERATOR.CXX >--------------------------------------------------*--------*
//                          Created:     05-Apr-03    Johan Sorensen * / [] \ *
//                          Last update: 05-Apr-03    Johan Sorensen * GARRET *
//-------------------------------------------------------------------*--------*
// Iterator class implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "iterator.h"
#include "assert.h"

BEGIN_POST_NAMESPACE


Ttree_iterator::Ttree_iterator(dbTtree *t, bool bReverseOrder)
  : m_CurrPos(-1)
{
  m_CurrNode = (t ? t->root : NULL);

  if (m_CurrNode)
  {
    if (bReverseOrder)
    {
      // start by going down to the right-most leaf node in the tree
      m_CurrPos = m_CurrNode->nItems - 1;
      while (m_CurrNode->right != NULL)
      {
        m_NodeStack.push(m_CurrNode);
        m_PositionStack.push(m_CurrPos);

        m_CurrNode = m_CurrNode->right;
        m_CurrPos = m_CurrNode->nItems - 1;
      }
    }
    else
    {
      // start by going down to the left-most leaf node in the tree
      m_CurrPos = 0;
      while (m_CurrNode->left != NULL)
      {
        m_NodeStack.push(m_CurrNode);
        m_PositionStack.push(m_CurrPos);

        m_CurrNode = m_CurrNode->left;
      }
    }
    if (m_CurrNode->nItems == 0)
      m_CurrPos = -1;   // abnormal case to encounter an empty node?!...
  }
}

Ttree_iterator::~Ttree_iterator()
{
  // m_NodeStack and m_PositionStack will delete themselves :-)
}


bool Ttree_iterator::IsValid () const
{
  return m_CurrPos != -1;
}

object* Ttree_iterator::operator* ()
{
  if (IsValid())
  {
    return m_CurrNode->item[m_CurrPos];
  }
  return NULL;
}

object* Ttree_iterator::operator++ ()
{
  if (!IsValid())
    return NULL;

  // have we traversed the "current" node yet?
  if (++m_CurrPos >= m_CurrNode->nItems)
  {
    // if so, see if there's a child on the right
    if (m_CurrNode->right != NULL)
    {
      m_NodeStack.push(m_CurrNode);
      m_PositionStack.push(m_CurrPos);
      m_CurrNode = m_CurrNode->right;
      m_CurrPos = 0;

      // and walk down to the left-most leaf of that branch
      while (m_CurrNode->left != NULL)
      {
        m_NodeStack.push(m_CurrNode);
        m_PositionStack.push(m_CurrPos);

        m_CurrNode = m_CurrNode->left;
      }
    }
    else
    {
      // walk up the tree until we find a node we haven't traversed internally yet
      size_t s = m_NodeStack.get_size();
      while (m_CurrPos >= m_CurrNode->nItems && s > 0)
      {
        s--;
        m_CurrNode = m_NodeStack[s];
        m_CurrPos  = m_PositionStack[s];

        m_NodeStack.set_size(s);
        m_PositionStack.set_size(s);
      }

      if (m_CurrPos >= m_CurrNode->nItems)
        m_CurrPos = -1;   // we're actually finished!
    }
    assert(m_CurrNode->nItems > 0);
  }

  return operator*();
}

object* Ttree_iterator::operator-- ()
{
  if (!IsValid())
    return NULL;

  // have we traversed the "current" node yet?
  if (--m_CurrPos < 0)
  {
    // if so, see if there's a child on the left
    if (m_CurrNode->left != NULL)
    {
      m_NodeStack.push(m_CurrNode);
      m_PositionStack.push(m_CurrPos);
      m_CurrNode = m_CurrNode->left;
      m_CurrPos = m_CurrNode->nItems - 1;

      // and walk down to the right-most leaf of that branch
      while (m_CurrNode->right != NULL)
      {
        m_NodeStack.push(m_CurrNode);
        m_PositionStack.push(m_CurrPos);

        m_CurrNode = m_CurrNode->right;
        m_CurrPos = m_CurrNode->nItems - 1;
      }
    }
    else
    {
      // walk up the tree until we find a node we haven't traversed internally yet
      size_t s = m_NodeStack.get_size();
      while (m_CurrPos < 0 && s > 0)
      {
        s--;
        m_CurrNode = m_NodeStack[s];
        m_CurrPos  = m_PositionStack[s];

        m_NodeStack.set_size(s);
        m_PositionStack.set_size(s);
      }

      // we're finihsed when we come out here with m_CurrPos == -1
    }
    assert(m_CurrNode->nItems > 0);
  }

  return operator*();
}

END_POST_NAMESPACE
