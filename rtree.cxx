//-< RTREE.CXX >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     15-Mar-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 15-Mar-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// R-tree class implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "rtree.h"

BEGIN_POST_NAMESPACE

void R_tree::insert(rectangle const& r, object* obj)
{
    if (root == NULL) { 
	root = new_in(*get_storage(), R_page)(r, obj);
	height = 1;
    } else { 
	R_page* p = root->insert(r, obj, height);
	if (p != NULL) { 
	    // root splitted
	    root = new_in(*get_storage(), R_page)(root, p);
	    height += 1;
	}
    }
    n_records += 1;
}


bool R_tree::remove(rectangle const& r, object* obj)
{
    if (height != 0) { 
	R_page::reinsert_list rlist;
	if (root->remove(r, obj, height, rlist)) { 
	    R_page* pg = rlist.chain;
	    int level = rlist.level;
	    while (pg != NULL) {
		for (int i = 0, n = pg->n; i < n; i++) { 
		    R_page* p = root->insert(pg->b[i].rect, 
					     pg->b[i].p, height-level);
		    if (p != NULL) { 
			// root splitted
			root = new_in(*get_storage(), R_page)(root, p);
			height += 1;
		    }
		}
		level -= 1;
		R_page* next = pg->next_reinsert_page();
		delete pg;
		pg = next;
	    }
	    if (root->n == 1 && height > 1) { 
		R_page* new_root = root->b[0].p;
		delete root;
		root = new_root;
		height -= 1;
	    }
	    n_records -= 1;
	    return true;
	}
    }
    return false;
}

int R_tree::search(rectangle const& r, callback& cb) const
{
    return (n_records != 0) ? root->search(r, cb, height) : 0;
}

int R_tree::search(rectangle const& r, search_buffer& sb) const
{
    return (n_records != 0) ? root->search(r, sb, height) : 0;
}

void R_tree::purge(bool delete_leaves)
{
    if (root != NULL) { 
	root->purge(height, delete_leaves);
	root = NULL;
	n_records = 0;
    }
}

REGISTER(R_tree);

//-------------------------------------------------------------------------
// R-tree page methods
//-------------------------------------------------------------------------

//
// Search for objects overlapped with specified rectangle and call
// callback method for all such objects.
//
int R_page::search(rectangle const& r, callback& cb, int level) const
{
    assert(level >= 0);
    int hit_count = 0;
    if (--level != 0) { /* this is an internal node in the tree */
	for (int i=0; i < n; i++) { 
	    if ((r & b[i].rect) != 0) {
		R_page* child = b[i].p;
		hit_count += child->search(r, cb, level);
	    }
	}
    } else { /* this is a leaf node */
	for (int i=0; i < n; i++) { 
	    if ((r & b[i].rect) != 0) {
		cb.apply(b[i].p);
		hit_count += 1;
	    }
	}
    }
    return hit_count;
}

//
// Search for objects overlapped with specified rectangle and place
// all such objects in search buffer.
//
int R_page::search(rectangle const& r, search_buffer& sb, int level) const
{
    assert(level >= 0);
    int hit_count = 0;
    if (--level != 0) { /* this is an internal node in the tree */
	for (int i=0; i < n; i++) { 
	    if ((r & b[i].rect) != 0) {
		R_page* child = b[i].p;
		hit_count += child->search(r, sb, level);
	    }
	}
    } else { /* this is a leaf node */
	for (int i=0; i < n; i++) { 
	    if ((r & b[i].rect) != 0) {
		sb.push(b[i].p);
		hit_count += 1;
	    }
	}
    }
    return hit_count;
}

//
// Create root page
//
R_page::R_page(rectangle const& r, object* obj) 
{
    n = 1;
    b[0].rect = r;
    b[0].p = (R_page*)obj;
}

//
// Create new root page (root splitting)
//
R_page::R_page(R_page* old_root, R_page* new_page)
{
    n = 2;
    b[0].rect = old_root->cover();
    b[0].p = old_root;
    b[1].rect = new_page->cover();
    b[1].p = new_page;
}

//
// Calculate cover of all rectangles at page
//
rectangle R_page::cover() const 
{
    rectangle r = b[0].rect;
    for (int i = 1; i < n; i++) { 
	r += b[i].rect;
    }
    return r;
}

R_page* R_page::split_page(branch const& br)
{
    int i, j, seed[2];
    int rect_area[card+1], waste, worst_waste = INT_MIN;
    //
    // As the seeds for the two groups, find two rectangles which waste 
    // the most area if covered by a single rectangle.
    //
    rect_area[0] = area(br.rect);
    for (i = 0; i < card; i++) { 
	rect_area[i+1] = area(b[i].rect);
    }
    branch const* bp = &br;
    for (i = 0; i < card; i++) { 
	for (j = i+1; j <= card; j++) { 
	    waste = area(bp->rect + b[j-1].rect) - rect_area[i] - rect_area[j];
	    if (waste > worst_waste) {
		worst_waste = waste;
		seed[0] = i;
		seed[1] = j;
	    }
	}
	bp = &b[i];
    }	    
    char taken[card];
    rectangle group[2];
    int group_area[2];
    int group_card[2];
    R_page* p;
    
    memset(taken, 0, sizeof taken);
    taken[seed[1]-1] = 2;
    group[1] = b[seed[1]-1].rect;
    
    if (seed[0] == 0) { 
	group[0] = br.rect;
	p = new_in(*get_storage(), R_page)(br.rect, br.p);
    } else { 
	group[0] = b[seed[0]-1].rect;
	p = new_in(*get_storage(), R_page)(group[0], b[seed[0]-1].p);
	b[seed[0]-1] = br;
    }
    group_card[0] = group_card[1] = 1;
    group_area[0] = rect_area[seed[0]];
    group_area[1] = rect_area[seed[1]];
    //
    // Split remaining rectangles between two groups.
    // The one chosen is the one with the greatest difference in area 
    // expansion depending on which group - the rect most strongly 
    // attracted to one group and repelled from the other.
    //
    while (group_card[0] + group_card[1] < card + 1 
	   && group_card[0] < card + 1 - min_fill
	   && group_card[1] < card + 1 - min_fill)
    {
	int better_group = -1, chosen = -1, biggest_diff = -1;
	for (i = 0; i < card; i++) { 
	    if (!taken[i]) { 
		int diff = (area(group[0] + b[i].rect) - group_area[0])
		         - (area(group[1] + b[i].rect) - group_area[1]);
		if (diff > biggest_diff || -diff > biggest_diff) { 
		    chosen = i;
		    if (diff < 0) { 
			better_group = 0;
			biggest_diff = -diff;
		    } else { 
			better_group = 1;
			biggest_diff = diff;
		    }
		}
	    }
	}
	assert(chosen >= 0);
	group_card[better_group] += 1;
	group[better_group] += b[chosen].rect;
	group_area[better_group] = area(group[better_group]);
	taken[chosen] = better_group+1;
	if (better_group == 0) { 
	    p->b[group_card[0]-1] = b[chosen];
	}
    }
    //
    // If one group gets too full, then remaining rectangle are
    // split between two groups in such way to balance cards of two groups.
    //
    if (group_card[0] + group_card[1] < card + 1) { 
	for (i = 0; i < card; i++) { 
	    if (!taken[i]) { 
		if (group_card[0] >= group_card[1]) { 
		    taken[i] = 2;
		    group_card[1] += 1;
		} else { 
		    taken[i] = 1;
		    p->b[group_card[0]++] = b[i];		
		}
	    }
	}
    }
    p->n = group_card[0];
    n = group_card[1];
    for (i = 0, j = 0; i < n; j++) { 
	if (taken[j] == 2) {
	    b[i++] = b[j];
	}
    }
    while (i < card) { 
	b[i++].p = NULL;
    }
    return p;
}

void R_page::remove_branch(int i)
{
    n -= 1;
    memcpy(&b[i], &b[i+1], (n-i)*sizeof(branch));
}
    
R_page* R_page::insert(rectangle const& r, object* obj, int level) 
{
    branch br;
    if (--level != 0) { 
	// not leaf page
	int i, mini = 0;
	size_t min_incr = INT_MAX;
	size_t best_area = INT_MAX;
	for (i = 0; i < n; i++) { 
            size_t r_area = area(b[i].rect);
	    size_t incr = area(b[i].rect + r) - r_area;
	    if (incr < min_incr) { 
                best_area = r_area;
		min_incr = incr;
		mini = i;
	    } else if (incr == min_incr && r_area < best_area) { 
                best_area = r_area;
                mini = i;
            } 
	}
	R_page* p = b[mini].p;
	R_page* q = p->insert(r, obj, level);
	if (q == NULL) { 
	    // child was not split
	    b[mini].rect += r;
	    return NULL;
	} else { 
	    // child was split
	    b[mini].rect = p->cover();
	    br.p = q;
	    br.rect = q->cover();
	    return add_branch(br);
	}
    } else { 
	br.p = (R_page*)obj;
	br.rect = r;
	return add_branch(br);
    }
}

bool R_page::remove(rectangle const& r, object* obj, 
		    int level, reinsert_list& rlist)
{
    if (--level != 0) { 
	for (int i = 0; i < n; i++) { 
	    if (b[i].rect & r) { 
		R_page* p = b[i].p;
		if (p->remove(r, obj, level, rlist)) { 
		    if (p->n >= min_fill) { 
			b[i].rect = p->cover();
		    } else { 
			// not enough entries in child
			p->b[card-1].p = rlist.chain;
			rlist.chain = p;
			rlist.level = level - 1; 
			remove_branch(i);
		    }
		    return true;
		}
	    }
	}
    } else {
	for (int i = 0; i < n; i++) { 
	    if (b[i].p == obj) { 
		remove_branch(i);
		return true;
	    }
	}
    }
    return false;
}

void R_page::purge(int level, bool remove_leaves)
{
    if (--level != 0) { /* this is an internal node in the tree */
	for (int i=0; i < n; i++) { 
	    b[i].p->purge(level, remove_leaves);
	}
    } else { /* this is a leaf node */
	if (remove_leaves) { 
	    for (int i=0; i < n; i++) { 
		delete b[i].p;
	    }
	}
    }
    delete this;
}

REGISTER(R_page);

END_POST_NAMESPACE


