//-< RTREE.H >-------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     15-Mar-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 15-Mar-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// R-tree: example of POST++ class definition
//-------------------------------------------------------------------*--------*

#ifndef __RTREE_H__
#define __RTREE_H__

#include "object.h"
#include "dnmarr.h"

BEGIN_POST_NAMESPACE

class POST_DLL_ENTRY rectangle
{
  public:
    enum { dim = 2 };
    int boundary[dim*2];
    friend size_t area(rectangle const& r) { 
	size_t area = 1;
	for (int i = dim; --i >= 0; area *= r.boundary[i+dim] - r.boundary[i]);
	return area;
    }
    void operator +=(rectangle const& r) { 
        int i = dim; 
	while (--i >= 0) { 
	    boundary[i] = (boundary[i] <= r.boundary[i]) 
		? boundary[i] : r.boundary[i];
	    boundary[i+dim] = (boundary[i+dim] >= r.boundary[i+dim]) 
		? boundary[i+dim] : r.boundary[i+dim];
	}
    }
    rectangle operator + (rectangle const& r) const { 
	rectangle res;
        int i = dim; 
	while (--i >= 0) { 
	    res.boundary[i] = (boundary[i] <= r.boundary[i]) 
		? boundary[i] : r.boundary[i];
	    res.boundary[i+dim] = (boundary[i+dim] >= r.boundary[i+dim]) 
		? boundary[i+dim] : r.boundary[i+dim];
	}
	return res;
    }
    bool operator& (rectangle const& r) const {
	int i = dim; 
	while (--i >= 0) { 
	    if (boundary[i] > r.boundary[i+dim] ||
		r.boundary[i] > boundary[i+dim])
	    {
		return false;
	    }
	}
	return true;
    }
    bool operator <= (rectangle const& r) const { 
	int i = dim; 
	while (--i >= 0) { 
	    if (boundary[i] < r.boundary[i] ||
		boundary[i+dim] > r.boundary[i+dim])
	    {
		return false;
	    }
	}
	return true;
    }
};

class callback { 
  public:
    virtual void apply(object* obj) = 0;
};

class POST_DLL_ENTRY R_page : public object { 
  public:
    enum { 
	card = (4096-4)/(6+4*4), // maximal number of branches at page
	min_fill = card/2        // minimal number of branches at non-root page
    };

    struct POST_DLL_ENTRY branch { 
	rectangle rect;
	R_page*   p;

	CLASSINFO(branch, REF(p));
    };
    
    struct reinsert_list { 
	R_page* chain;
	int     level;
	reinsert_list() { chain = NULL; }
    };

    int search(rectangle const& r, callback& cb, int level) const;
    int search(rectangle const& r, search_buffer& sbuf, int level) const;

    R_page* insert(rectangle const& r, object* obj, int level);

    bool remove(rectangle const& r, object* obj, int level,
		reinsert_list& rlist);

    rectangle cover() const;

    R_page* split_page(branch const& br);

    R_page* add_branch(branch const& br) { 
	if (n < card) { 
	    b[n++] = br;
	    return NULL;
 	} else { 
	    return split_page(br);
	}
    }
    void remove_branch(int i);

    void purge(int level, bool remove_leaves);

    R_page* next_reinsert_page() const { return (R_page*)b[card-1].p; }

    R_page(rectangle const& rect, object* obj);
    R_page(R_page* old_root, R_page* new_page);

    int    n; // number of branches at page
    branch b[card];

    CLASSINFO(R_page, NO_REFS);
};

class POST_DLL_ENTRY R_tree : public object { 
  public: 
    int  search(rectangle const& r, callback& cb) const;	
    int  search(rectangle const& r, search_buffer& sbuf) const;       
    void insert(rectangle const& r, object* obj);
    bool remove(rectangle const& r, object* obj);

    CLASSINFO(R_tree, REF(root));

    void purge(bool delete_leaves = false);
    ~R_tree() { purge(); }

  protected:
    unsigned n_records;
    unsigned height;
    R_page*  root;
};

END_POST_NAMESPACE

#endif



