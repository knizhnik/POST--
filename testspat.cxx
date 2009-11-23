//-< TESTSPAT.CXX >--------------------------------------------------*--------*
// GOODS                     Version 1.0         (c) 1997  GARRET    *     ?  *
// (Generic Object Oriented Database System)                         *   /\|  *
//                                                                   *  /  \  *
//                          Created:      7-Jun-97    K.A. Knizhnik  * / [] \ *
//                          Last update: 17-Oct-97    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test program for R-tree class 
//-------------------------------------------------------------------*--------*

#include "object.h"
#include "rtree.h"
#include "hashtab.h"
#include <time.h>

USE_POST_NAMESPACE

class spatial_object : public object { 
  public:
    rectangle rect;
    int       id;

    void print() const { 
	printf("Object '%s' (%d,%d,%d,%d)\n", name, 
	       rect.boundary[0], rect.boundary[1], 
	       rect.boundary[2], rect.boundary[3]);
    }
    static spatial_object* create(storage& store,
				  int x0, int y0, int x1, int y1,
				  const char* name, int id) 
    {
	return new (self_class, store, strlen(name)) spatial_object(x0, y0, 
								    x1, y1, 
								    id, name);
    }
    bool is(const char* id) const { return strcmp(name, id) == 0; }

    CLASSINFO(spatial_object, NO_REFS);
  protected:
    char      name[1];

    spatial_object(int x0, int y0, int x1, int y1,
		   int ident, const char* so_name)
    {
	id = ident;
	rect.boundary[0] = x0;
	rect.boundary[1] = y0;
	rect.boundary[2] = x1;
	rect.boundary[3] = y1;
	strcpy(name, so_name);
    }
};

REGISTER(spatial_object);


class spatial_base : public R_tree { 
  protected:
    hash_table* hash;

    class spatial_object_callback : public callback { 
	virtual void apply(object* obj) { 
	    ((spatial_object*)obj)->print();
	}
    };
    static bool print_object(const char*, object* obj) { 
	((spatial_object*)obj)->print();
	return false;
    }
	
  public:
    void find(int x0, int y0, int x1, int y1) const {
	rectangle rect;
	spatial_object_callback cb;
	rect.boundary[0] = x0;
	rect.boundary[1] = y0;
	rect.boundary[2] = x1;
	rect.boundary[3] = y1;
	printf("Found %d objects\n", search(rect, cb));
    }
    void find(const char* name) const { 
	spatial_object* obj = (spatial_object*)hash->get(name);
	if (obj == NULL) { 
	    printf("No object found\n");
	} else { 
	    obj->print();
	}
    }
    void insert(int x0, int y0, int x1, int y1, const char* name, int id = 0) {
	spatial_object* obj = spatial_object::create(*get_storage(), 
						     x0, y0, x1, y1, 
						     name, id);
	R_tree::insert(obj->rect, obj);
	hash->put(name, obj);
    }
    bool remove(const char* name) { 
	spatial_object* obj = (spatial_object*)hash->get(name);
	if (obj != NULL) { 
	    hash->del(name);
	    R_tree::remove(obj->rect, obj);
	    delete obj;
	    return true;
	}
	return false;
    }
    void clear() { 
	purge(true);
        hash->purge();
    }
    spatial_base(size_t hash_size) { 
	hash = hash_table::create(*get_storage(), hash_size);
    }
    CLASSINFO(spatial_base, REF(hash));
};



REGISTER(spatial_base);

static void input(char* prompt, char* buf, size_t buf_size)
{
    char* p;
    do { 
	printf(prompt);
	*buf = '\0';
	fgets(buf, buf_size, stdin);
	p = buf + strlen(buf);
    } while (p <= buf+1); 
    
    if (*(p-1) == '\n') {
	*--p = '\0';
    }
}


struct test_record { 
    union { 
	test_record* next_free;
	struct { 
	    int x0;
	    int y0;
	    int x1;
	    int y1;
	} coord;
    };
    unsigned data[2];
};

inline unsigned random(unsigned mod) { return rand() % mod; }

static void random_test(spatial_base* base, long n_iterations) 
{ 
    const int max_records = 1024*1024;
    const int max_x = 32*1024;
    const int max_y = 32*1024;
    const int max_width = 1024;
    const int max_height = 1024;
    
    test_record* record = new test_record[max_records];
    test_record* free_chain;
    int i;
    int n_inserts = 0, n_searches = 0, n_removes = 0;
    test_record* rp;
    int x0, y0, x1, y1;
    rectangle rect;
    search_buffer sb;
    char buf[64];

    srand(time(NULL));
    for (i = 0; i < max_records; i++) { 
	record[i].data[0] = 0;
	record[i].next_free = &record[i+1];
    }
    record[max_records-1].next_free = NULL;
    free_chain = record;
    base->clear();
    time_t start_time = time(NULL);

    while (--n_iterations >= 0) { 
	switch (random(8)) { 
	  default: // insert
	    if (free_chain != NULL) { 
		rp = free_chain;
		free_chain = free_chain->next_free;
		rp->coord.x0 = x0 = random(max_x*2) - max_x; 
		rp->coord.x1 = x1 = x0 + random(max_width); 
		rp->coord.y0 = y0 = random(max_y*2) - max_y; 
		rp->coord.y1 = y1 = y0 + random(max_height); 
		rp->data[0] = rand();
		if (rp->data[0] == 0) { 
		    rp->data[0] += 1;
		}
		rp->data[1] = rand();
		i = rp - record;
		sprintf(buf, "%x%x.%x", rp->data[0], rp->data[1], i);
		base->insert(x0, y0, x1, y1, buf, i);
		n_inserts += 1;
	    }
	    break;
	  case 0: // remove
	    i = random(max_records);
	    rp = &record[i];
	    if (rp->data[0]) { 
		sprintf(buf, "%x%x.%x", rp->data[0], rp->data[1], i);
		bool removed = base->remove(buf);
		assert(removed);
		rp->data[0] = 0;
		rp->next_free = free_chain;
		free_chain = rp;
		n_removes += 1;
	    } 
	    break;
	  case 1: // search
	    x0 = random(max_x*2) - max_x; 
	    x1 = x0 + random(max_width); 
	    y0 = random(max_y*2) - max_y; 
	    y1 = y0 + random(max_height); 
	    rect.boundary[0] = x0;
	    rect.boundary[1] = y0;
	    rect.boundary[2] = x1;
	    rect.boundary[3] = y1;
	    sb.set_size(0);
	    int found = base->search(rect, sb);
	    rp = record;
	    for (i = 0; i < max_records; i++, rp++) { 
		if (rp->data[0]) { 
		    if (x0 <= rp->coord.x1 && rp->coord.x0 <= x1
			&& y0 <= rp->coord.y1 && rp->coord.y0 <= y1) 
		    {
			int j = sb.get_size(); 
			while (--j >= 0 && ((spatial_object*)sb[j])->id != i);
			assert(j >= 0);
			spatial_object* obj = (spatial_object*)sb[j];
			sprintf(buf, "%x%x.%x", 
				rp->data[0], rp->data[1], i);
			assert(obj->rect.boundary[0] == rp->coord.x0 &&
			       obj->rect.boundary[1] == rp->coord.y0 &&
			       obj->rect.boundary[2] == rp->coord.x1 &&
			       obj->rect.boundary[3] == rp->coord.y1 &&
			       obj->is(buf));
			found -= 1;
		    }
		}
	    }
	    assert (found == 0); 
	    n_searches += 1;
	}
	printf("Proceed %d inserts, %d removes, %d searches\r", 
			n_inserts, n_removes, n_searches);
    }
    printf("\nElapsed time: %ld\n", time(NULL) - start_time);
    delete[] record;
}

int main() 
{ 
    storage db("rtree");
    if (db.open(storage::fault_tolerant)) { 
	spatial_base* root = (spatial_base*)db.get_root_object();
	if (root == NULL) { 
	    root = new_in (db,spatial_base)(9719);
	    db.set_root_object(root);
	}
	
	while (true) { 
	    char buf[256], name[256];
	    int  action = 0;
	    int  x0, y0, x1, y1;
	    int  n_iterations;

	    printf("\nSpatial database menu:\n"
		   "\t1. Insert object\n"
		   "\t2. Remove object\n"
		   "\t3. Find object by name\n"
		   "\t4. Find object by coordinates\n"
		   "\t5. Random test\n"
		   "\t6. Exit\n");
	    input(">>", buf, sizeof buf);
	    sscanf(buf, "%d", &action);
	    switch (action) { 
	      case 1:
		input("Please specify object name: ", name, sizeof name);
		while (true) { 
		    input("Please specify object coordinates x0 y0 x1 y1: ",
			  buf,  sizeof buf);
		    if (sscanf(buf, "%d %d %d %d", &x0, &y0, &x1, &y1) == 4) {
			root->insert(x0, y0, x1, y1, name);
			break;
		    }
		}
		continue;
	      case 2:
		input("Please specify removed object name: ", 
		      name, sizeof name);
		if (!root->remove(name)) { 
		    printf("Object is not in database\n");
		}
		continue;
	      case 3:
		input("Please specify object name: ", name, sizeof name);
		root->find(name);
		continue;
	      case 4:
		while (true) { 
		    input("Please specify object coordinates x0 y0 x1 y1: ",
			  buf, sizeof buf);
		    if (sscanf(buf, "%d %d %d %d", &x0, &y0, &x1, &y1) == 4) {
			root->find(x0, y0, x1, y1);
			break;
		    }
		}
		continue;
	      case 5:
		while (true) {
		    input("Please specify number of iterations: ", 
			  buf, sizeof buf);
		    if (sscanf(buf, "%d", &n_iterations) == 1) {
			random_test(root, n_iterations);
			break;
		    }
		}
		continue;
	      case 6:
		db.flush();
		db.close();
		printf("Session is closed\n");
		return EXIT_SUCCESS;
	      default:
		printf("Please chose action 1..7\n");
	    }
	}
    } else { 
	printf("Failed to open database\n");
	return EXIT_FAILURE;
    }
}








