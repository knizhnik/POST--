//-< TTREE.CXX >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-tree class implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "ttree.h"

BEGIN_POST_NAMESPACE

REGISTER(dbTtree);
REGISTER(dbTtreeNode);


int dbTtree::find(dbSearchContext& sc)
{
    sc.lastMatch = NULL;
    sc.nMatches = 0;
    if (sc.buffer != NULL) { 
	sc.buffer->set_size(0);
    }
    if (root != NULL) { 
	root->find(sc);
    }
    return sc.nMatches;
}

void dbTtree::insert(object* obj, dbRelation& relation)
{
    if (root == NULL) {
	root = new_in(*get_storage(), dbTtreeNode)(obj);
    } else { 
	root->insert(root, obj, relation);
    }
}


void dbTtree::remove(object* obj, dbRelation& relation)
{
    int h = root->remove(root, obj, relation);
    assert(h >= 0);
}   


void dbTtree::purge() 
{
    if (root != NULL) { 
	delete root;
	root = NULL;
    } 
}


bool dbTtreeNode::find(dbSearchContext& sc)
{
    int l, r, m, n = nItems;

    if (sc.firstKey != NULL) { 
	if (sc.compare(sc.firstKey, item[0]) >= sc.firstKeyInclusion) {	    
	    if (sc.compare(sc.firstKey, item[n-1]) >= sc.firstKeyInclusion) {
		if (right != 0) { 
		    return right->find(sc); 
		} 
		return true;
	    }
	    for (l = 0, r = n; l < r;) { 
		m = (l + r) >> 1;
		if (sc.compare(sc.firstKey, item[m]) >= sc.firstKeyInclusion) {
		    l = m+1;
		} else { 
		    r = m;
		}
	    }
	    while (r < n) { 
		if (sc.lastKey != NULL 
		    && -sc.compare(sc.lastKey, item[r]) >= sc.lastKeyInclusion)
		{ 
		    return false;
		}
		if (sc.predicate(item[r])) {
		    sc.lastMatch = item[r];
		    if (sc.buffer != NULL) { 
			sc.buffer->push(item[r]);
		    }
		    if (++sc.nMatches == sc.selectionLimit) { 
			return false;
		    }
		}
		r += 1;
	    }
	    if (right != 0) { 
		return right->find(sc); 
	    } 
	    return true;	
	}
    }	
    if (left != 0) { 
	if (!left->find(sc)) { 
	    return false;
	}
    }
    for (l = 0; l < n; l++) { 
	if (sc.lastKey != NULL 
	    && -sc.compare(sc.lastKey, item[l]) >= sc.lastKeyInclusion) 
	{
	    return false;
	}
	if (sc.predicate(item[l])) {
	    sc.lastMatch = item[l];
	    if (sc.buffer != NULL) { 
		sc.buffer->push(item[l]);
	    }
	    if (++sc.nMatches == sc.selectionLimit) { 
		return false;
	    }
	}
    }
    if (right != 0) { 
	return right->find(sc);
    } 
    return true;
}


bool dbTtreeNode::insert(dbTtreeNode* &p, object* obj, dbRelation& relation)
{
    int n = nItems;
    int diff = relation.compare(obj, item[0]);
    if (diff <= 0) { 
	if ((left == NULL || diff == 0) && n != pageSize) { 
	    for (int i = n; i > 0; i--) item[i] = item[i-1];
	    item[0] = obj;
	    nItems += 1;
	    return false;
	} 
	if (left == NULL) { 
	    left = new_in(*get_storage(), dbTtreeNode)(obj);
	} else {
	    if (!left->insert(left, obj, relation)) { 
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
	    dbTtreeNode* lp = left;
	    if (lp->balance < 0) { // single LL turn
		left = lp->right;
		lp->right = this;
		balance = 0;
		lp->balance = 0;
		p = lp;
	    } else { // double LR turn
		dbTtreeNode* rp = lp->right;
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
    } 
    diff = relation.compare(obj, item[n-1]);
    if (diff >= 0) { 
	if ((right == NULL || diff == 0) && n != pageSize) { 
	    item[n] = obj;
	    nItems += 1;
	    return false;
	}
	if (right == NULL) { 
	    right = new_in(*get_storage(), dbTtreeNode)(obj);
	} else { 
	    if (!right->insert(right, obj, relation)) { 
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
	    dbTtreeNode* rp = right;
	    if (rp->balance > 0) { // single RR turn
		right = rp->left;
		rp->left = this;
		balance = 0;
		rp->balance = 0;
		p = rp;
	    } else { // double RL turn
		dbTtreeNode* lp = rp->left;
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
    }
    int l = 1, r = n-1;
    while (l < r)  {
	int i = (l+r) >> 1;
	diff = relation.compare(obj, item[i]);
	if (diff > 0) { 
	    l = i + 1;
	} else { 
	    r = i;
	    if (diff == 0) { 
		break;
	    }
	}
    }
    // Insert before item[r]
    if (n != pageSize) {
	for (int i = n; i > r; i--) item[i] = item[i-1]; 
	item[r] = obj;
	nItems += 1;
	return false;
    } else { 
	object* reinsert;
	if (balance >= 0) { 
	    reinsert = item[0];
	    for (int i = 1; i < r; i++) item[i-1] = item[i]; 
	    item[r-1] = obj;
	} else { 
	    reinsert = item[n-1];
	    for (int i = n-1; i > r; i--) item[i] = item[i-1]; 
	    item[r] = obj;
	}
	return insert(p, reinsert, relation);
    }
}

inline int dbTtreeNode::balanceLeftBranch(dbTtreeNode* &p)
{
    if (balance < 0) { 
	balance = 0;
	return 1;
    } else if (balance == 0) { 
	balance = 1;
	return 0;
    } else { 
	dbTtreeNode* rp = right;
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
	    dbTtreeNode* lp = rp->left;
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


inline int dbTtreeNode::balanceRightBranch(dbTtreeNode* &p)
{
    if (balance > 0) { 
	balance = 0;
	return 1;
    } else if (balance == 0) { 
	balance = -1;
	return 0;
    } else { 
	dbTtreeNode* lp = left;
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
	    dbTtreeNode* rp = lp->right;
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

int dbTtreeNode::remove(dbTtreeNode* &p, object* obj, dbRelation& relation)
{
    int n = nItems;
    int diff = relation.compare(obj, item[0]);
    if (diff <= 0) { 
	if (left != NULL) { 
	    int h = left->remove(left, obj, relation);
	    if (h > 0) { 
		return balanceLeftBranch(p);
	    } else if (h == 0) { 
		return 0;
	    }
	}
	assert (diff == 0);
    }
    diff = relation.compare(obj, item[n-1]);
    if (diff <= 0) {	    
	for (int i = 0; i < n; i++) { 
	    if (item[i] == obj) { 
		if (n == 1) { 
		    if (right == NULL) { 
			p = left;
			left = NULL;
			delete this;
			return 1;
		    } else if (left == NULL) { 
			p = right;
			right = NULL;
			delete this;
			return 1;
		    } 
		} 
		if (n <= minItems) { 
		    if (left != NULL && balance <= 0) {  
			dbTtreeNode* lp = left;
			while (lp->right != 0) { 
			    lp = lp->right;
			}
			while (--i >= 0) { 
			    item[i+1] = item[i];
			}
			item[0] = lp->item[lp->nItems-1];
			int h = left->remove(left, item[0], relation); 
			if (h > 0) {
			    h = balanceLeftBranch(p);
			}
			return h;
		    } else if (right != NULL) { 
			dbTtreeNode* rp = right;
			while (rp->left != 0) { 
			    rp = rp->left;
			}
			while (++i < n) { 
			    item[i-1] = item[i];
			}
		        item[n-1] = rp->item[0];
			int h = right->remove(right, item[n-1], relation);
			if (h > 0) {
			    h = balanceRightBranch(p);
			}
			return h;
		    }
		}
		while (++i < n) { 
		    item[i-1] = item[i];
		}
		nItems -= 1;
		return 0;
	    }
	}
    }
    if (right != NULL) { 
	int h = right->remove(right, obj, relation);
	if (h > 0) { 
	    return balanceRightBranch(p);
	} else { 
	    return h;
	}
    }
    return -1;
}

dbTtreeNode::~dbTtreeNode()
{
    delete left;
    delete right;
}

END_POST_NAMESPACE

