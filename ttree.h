//-< TTREE.H >-------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-tree class interface
//-------------------------------------------------------------------*--------*

#ifndef __TTREE_H__
#define __TTREE_H__

#include "object.h"
#include "dnmarr.h"

BEGIN_POST_NAMESPACE

class POST_DLL_ENTRY dbRelation {
  public:
    virtual int compare(object* o1, object* o2) = 0;
};

class POST_DLL_ENTRY dbSearchContext { 
  public:
    void*          firstKey;
    void*          lastKey;
    int            firstKeyInclusion; // true/false
    int            lastKeyInclusion;  // true/false
    search_buffer* buffer;         // set to NULL if buffer not needed 
    size_t         selectionLimit; // maximal number of selected objects 
                                   // (0 - no limit)
    size_t         nMatches;
    object*        lastMatch;

    virtual bool   predicate(object*) { return true; }
    virtual int    compare(void const* key, object* obj) = 0;
};


class POST_DLL_ENTRY dbTtreeNode : public object { 
    enum { 
	pageSize = 125,
	minItems = pageSize - 2 // minimal number of items in internal node
    };

  public:
    dbTtreeNode* left;
    dbTtreeNode* right;
    int1         balance;
    nat2         nItems;
    object*      item[pageSize];

    CLASSINFO(dbTtreeNode, REF(left) REF(right) REFS(item));

    dbTtreeNode(object* obj) { 
	nItems = 1;
	item[0] = obj;
	left = right = NULL;
	balance = 0;
    }
    
    bool  insert(dbTtreeNode* &node, object* obj, dbRelation& relation);
    int   remove(dbTtreeNode* &node, object* obj, dbRelation& relation);

    int   balanceRightBranch(dbTtreeNode* &node);
    int   balanceLeftBranch(dbTtreeNode* &node);
    
    bool  find(dbSearchContext& sc);

    ~dbTtreeNode();
};

class POST_DLL_ENTRY dbTtree : public object { 
  protected:
    dbTtreeNode* root;

  public:
    CLASSINFO(dbTtree, REF(root));

    int   find(dbSearchContext& sc);

    void  insert(object* obj, dbRelation& relation);
    void  remove(object* obj, dbRelation& relation);

    void  purge();

    friend class Ttree_iterator;
};


END_POST_NAMESPACE

#endif
