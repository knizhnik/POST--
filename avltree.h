//-< AVLTREE.H >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  4-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// AVL-tree: example of POST++ class definition
//-------------------------------------------------------------------*--------*

#ifndef __AVLTREE_H__
#define __AVLTREE_H__

#include "object.h"

BEGIN_POST_NAMESPACE

class POST_DLL_ENTRY l2elem {
  public:
    l2elem* next;
    l2elem* prev;

    void link_after(l2elem* after) {
	(next = after->next)->prev = this;
	(prev = after)->next = this;
    }
    void link_before(l2elem* before) {
	(prev = before->prev)->next = this;
	(next = before)->prev = this;
    }
    void unlink() {
        prev->next = next;
        next->prev = prev;
    }
    void prune() {
	next = prev = this;
    }
    bool empty() const {
	return next == this;
    };

    CLASSINFO(l2elem, REF(next) REF(prev));
};

class POST_DLL_ENTRY avl_tree_key : public l2elem { 
  public:  
    //
    // Returns negative if this key is less than key passed as parameter,
    // positive if this key is greater, and 0 - if equal.
    //
    virtual int compare(avl_tree_key const* key) const = 0;
};


class POST_DLL_ENTRY avl_tree_node : public object { 
  public: 
    avl_tree_key*  key;
    avl_tree_node* left;
    avl_tree_node* right;
    int            balance;

    CLASSINFO(avl_tree_node, REF(key) REF(left) REF(right));

    //
    // This method returns true if height of branch increased after new 
    // key insertion. If there is a key with the same value in the tree, 
    // then new key is not inserted, and reference to the old key is 
    // returned through 'ins_key' parameter.
    //
    bool insert(avl_tree_node*& p, avl_tree_key*& ins_key);

    //
    // Method returns 1 if height of branch decreased, 
    // 0 if height not changed, -1 if key was not found in the tree
    //
    int  remove(avl_tree_node*& p, avl_tree_key* del_key); 

    int  remove(avl_tree_node*& p, avl_tree_node*& q);
    int  balance_left_branch(avl_tree_node*& p);
    int  balance_right_branch(avl_tree_node*& p);

    avl_tree_node(avl_tree_key* new_key) { 
	key = new_key;
	left = right = NULL;
	balance = 0;
    }
};

class POST_DLL_ENTRY avl_tree : public object, public l2elem { 
  protected: 
    avl_tree_node* root;
  public: 
    //
    // Insert key in AVL tree. 
    // If there is no such key in AVL tree then new key is inserted and
    // returned as the result of the method, otherwise pointer to the key
    // in AVL tree with the same value is returned (new key is not inserted).
    //
    avl_tree_key* insert(avl_tree_key* key); 
    //
    // Remove key from AVL tree.
    // Return false if there is no such key in AVL tree, true otherwise.
    //
    bool          remove(avl_tree_key* key); 

    //
    // Looks for a key with the same value in AVL tree.
    //
    avl_tree_key* search(avl_tree_key* key) const; 

    CLASSINFO(avl_tree, REF(root));

    avl_tree(int) { // dummy parameter to distinguish from default constructor
	prune();
	root = NULL;
    }
};

END_POST_NAMESPACE

#endif
