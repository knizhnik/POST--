//-< AVLTREE.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  4-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Implementation of AVL tree
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "avltree.h"

BEGIN_POST_NAMESPACE

avl_tree_key* avl_tree::insert(avl_tree_key* key)
{
    if (root == NULL) { 
	root = new_in(*get_storage(),avl_tree_node)(key);
	key->link_after(this);
	return key;
    } else { 
	root->insert(root, key);
	return key;
    }
}

avl_tree_key* avl_tree::search(avl_tree_key* key) const
{
    avl_tree_node* np = root;
    while (np != NULL) { 
	int diff = key->compare(np->key);
	if (diff < 0) np = np->left;
	else if (diff > 0) np = np->right;
	else return np->key;
    }
    return NULL;
}

bool avl_tree::remove(avl_tree_key* key)
{
    return root ? root->remove(root, key) >= 0 : false;
}

REGISTER(avl_tree);

bool avl_tree_node::insert(avl_tree_node*& p, avl_tree_key*& ins_key)
{
    int diff = ins_key->compare(key);
    if (diff < 0) {
	if (left == NULL) { 
	    left = new_in(*get_storage(),avl_tree_node)(ins_key);
	    ins_key->link_before(key);
	} else { 
	    if (!left->insert(left, ins_key)) { 
		return false;
	    }
	}
	if (balance > 0) { 
	    balance = 0;
	    return false;
	} else if (balance == 0) { 
	    balance = -1;
	    return true;
	} else { 
	    avl_tree_node* lp = left;
	    if (lp->balance < 0) { // single LL turn
		left = lp->right;
		lp->right = this;
		balance = 0;
		lp->balance = 0;
		p = lp;
	    } else { // double LR turn
		avl_tree_node* rp = lp->right;
		lp->right = rp->left;
		rp->left = lp;
		left = rp->right;
		rp->right = this;
		balance = (rp->balance < 0) ? 1 : 0;
		lp->balance = (rp->balance > 0) ? -1 : 0;
		rp->balance = 0;
		p = rp;
	    }
	    return false;
	}
    } else if (diff > 0) { 
	if (right == NULL) { 
	    right = new_in(*get_storage(),avl_tree_node)(ins_key);
	    ins_key->link_after(key);
	} else { 
	    if (!right->insert(right, ins_key)) { 
		return false;
	    }
	}
	if (balance < 0) { 
	    balance = 0;
	    return false;
	} else if (balance == 0) { 
	    balance = 1;
	    return true;
	} else { 
	    avl_tree_node* rp = right;
	    if (rp->balance > 0) { // single RR turn
		right = rp->left;
		rp->left = this;
		balance = 0;
		rp->balance = 0;
		p = rp;
	    } else { // double RL turn
		avl_tree_node* lp = rp->left;
		rp->left = lp->right;
		lp->right = rp;
		right = lp->left;
		lp->left = this;
		balance = (lp->balance > 0) ? -1 : 0;
		rp->balance = (lp->balance < 0) ? 1 : 0;
		lp->balance = 0;
		p = lp;
	    }
	    return false;
	}
    } else { 
	ins_key = key;
	return false;
    }
}

inline int avl_tree_node::balance_left_branch(avl_tree_node*& p)
{
    if (balance < 0) { 
	balance = 0;
	return 1;
    } else if (balance == 0) { 
	balance = 1;
	return 0;
    } else { 
	avl_tree_node* rp = right;
	if (rp->balance >= 0) { // single RR turn
	    right = rp->left;
	    rp->left = this;
	    if (rp->balance == 0) { 
		balance = 1;
		rp->balance = -1;
		p = rp;
		return 0;
	    } else { 
		balance = 0;
		rp->balance = 0;
		p = rp;
		return 1;
	    }
	} else { // double RL turn
	    avl_tree_node* lp = rp->left;
	    rp->left = lp->right;
	    lp->right = rp;
	    right = lp->left;
	    lp->left = this;
	    balance = lp->balance > 0 ? -1 : 0;
	    rp->balance = lp->balance < 0 ? 1 : 0;
	    lp->balance = 0;
	    p = lp;
	    return 1;
	}
    }
}


inline int avl_tree_node::balance_right_branch(avl_tree_node*& p)
{
    if (balance > 0) { 
	balance = 0;
	return 1;
    } else if (balance == 0) { 
	balance = -1;
	return 0;
    } else { 
	avl_tree_node* lp = left;
	if (lp->balance <= 0) { // single LL turn
	    left = lp->right;
	    lp->right = this;
	    if (lp->balance == 0) { 
		balance = -1;
		lp->balance = 1;
		p = lp;
		return 0;
	    } else { 
		balance = 0;
		lp->balance = 0;
		p = lp;
		return 1;
	    }
	} else { // double LR turn
	    avl_tree_node* rp = lp->right;
	    lp->right = rp->left;
	    rp->left = lp;
	    left = rp->right;
	    rp->right = this;
	    balance = rp->balance < 0 ? 1 : 0;
	    lp->balance = rp->balance > 0 ? -1 : 0;
	    rp->balance = 0;
	    p = rp;
	    return 1;
	}
    }
}

int avl_tree_node::remove(avl_tree_node*& p, avl_tree_node*& q)
{
    if (right != NULL) { 
	return right->remove(right, q) ? balance_right_branch(p) : 0;
    } else { 
	q->key = key;
	q = this;
	p = left;
	return 1;
    } 
}

int avl_tree_node::remove(avl_tree_node*& p, avl_tree_key* del_key)
{
    int diff = del_key->compare(key);
    if (diff < 0) { 
	if (left == NULL) { 
	    return -1;
	} else { 
	    int h = left->remove(left, del_key);
	    if (h > 0) { 
		return balance_left_branch(p);
	    }
	    return h;
	} 
    } else if (diff > 0) { 
	if (right == NULL) { 
	    return -1;
	} else { 
	    int h = right->remove(right, del_key);
	    if (h > 0) { 
		return balance_right_branch(p);
	    }
	    return h;
	} 
    } else { 
	if (key != del_key) { 
	    return -1; // key not found
	} else { 
	    int h = 1;
	    avl_tree_node* q = this;
	    if (right == NULL) { 
		p = left;
	    } else if (left == NULL) { 
		p = right;
	    } else { 
		h = left->remove(left, q) && balance_left_branch(p);
	    }
	    del_key->unlink();
	    delete q;
	    return h;
	}
    }
}

REGISTER(avl_tree_node);

END_POST_NAMESPACE
