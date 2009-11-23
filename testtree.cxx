//-< TESTTREE.CXX >--------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  8-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example: test for AVL-tree and hash table
//-------------------------------------------------------------------*--------*

#include "avltree.h"
#include "hashtab.h"
#include <time.h>

USE_POST_NAMESPACE

storage test_storage("testtree");


class record : public object, public avl_tree_key { 
  public:
    int  hits; 
    char name[1];

    virtual int compare(avl_tree_key const* key) const { 
	return strcmp(name, ((record*)key)->name);
    }
    static record* create(const char* name) { 
	return new (self_class, test_storage, strlen(name)) record(name);
    }
    record(const char* name) { strcpy(this->name, name); }

    CLASSINFO(record, NO_REFS);
};
    
REGISTER(record);

class root_object : public object { 
  public: 
    avl_tree*   tree;
    hash_table* hash;

    CLASSINFO(root_object, REF(tree) REF(hash));
    
    root_object(size_t hash_size) { 
	tree = new_in (*get_storage(), avl_tree)(0);
	hash = hash_table::create(test_storage, hash_size);
    }
};

REGISTER(root_object);

class search_key : public avl_tree_key { 
  public:
    const char* name;

    virtual int compare(avl_tree_key const* key) const { 
	return strcmp(name, ((record*)key)->name);
    }
    search_key(const char* name) { this->name = name; }
};


#define random(mod) (rand() % mod)

void random_test(int n_iterations)
{
    struct node { 
	union { 
	    long k[2];
	    node* next;
	};
    };
    const int table_size = 32*1024;
    const int checkpoint_period = 64*1024;
    node* table = new node[table_size];
    int i;
    node* np;
    char buf[64];

    for (i = 0; i < table_size; i++) { 
	table[i].next = &table[i+1];
    }
    table[table_size-1].next = NULL;
    node* free_node_list = table;
    int n_inserted = 0, n_removed = 0, n_checkpoints = 0;
    int n_garbage_objects = 0;
    root_object* root = (root_object*)test_storage.get_root_object();
    srand(time(NULL));
    test_storage.garbage_collection(); 

    time_t start_time = time(NULL);
    while (n_iterations >= 0) { 
	if ((n_iterations & (checkpoint_period-1)) == 0) { 
	    int n_deallocated = test_storage.garbage_collection();
	    assert(n_deallocated == n_garbage_objects);
	    test_storage.flush();
	    n_iterations -= 1;
	    n_checkpoints += 1;
	    n_garbage_objects = 0;
	}
	switch (random(8)) { 
	  default: 
	    if (free_node_list != NULL) { 
		np = free_node_list;
		free_node_list = np->next;
		np->k[0] = rand() | 1;
		np->k[1] = rand();
		sprintf(buf, "%x%x.%x", (int)np->k[0], (int)np->k[1], 
			(int)(np - table));
		record* ins_rec = record::create(buf);
	        record* rec = (record*)root->tree->insert(ins_rec);
		if (ins_rec == rec) { 
		    root->hash->put(buf, rec);
		    n_inserted += 1;
		} else { 
		    n_garbage_objects += 1;
		}
		n_iterations -= 1;
		continue;
	    }
	  case 0:
	    i = random(table_size);
	    np = &table[i];
	    if (np->k[0] & 1) { // record not free
		sprintf(buf, "%x%x.%x", (int)np->k[0], (int)np->k[1], i);
		record* rec = (record*)root->hash->get(buf);
		assert(rec != NULL);
		bool removed_from_tree = root->tree->remove(rec);
		assert(removed_from_tree);
		bool removed_from_hash = root->hash->del(buf, rec);
		assert(removed_from_hash);
		np->next = free_node_list;
		free_node_list = np;
		delete rec;
		n_removed += 1;
		n_iterations -= 1;
	    }
	}
	printf("Proceed %d inserts, %d removes, %d checkpoints, "
	       "total %d elements  \r", 
	       n_inserted, n_removed, n_checkpoints, n_inserted - n_removed);
    }
    printf("\nElapsed time %d\n", (int)(time(NULL) - start_time));
    delete table;
}



void input(char* prompt, char* buf, size_t buf_size)
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


int do_test()
{
    if (test_storage.open(storage::use_transaction_log|
			  storage::support_virtual_functions)) 
    {
	record*    rec;
	char       buf[256];
	search_key key(buf);
	root_object* root = (root_object*)test_storage.get_root_object();
	
	if (root == NULL) { 
	    root = new_in(test_storage, root_object)(117);
	    test_storage.set_root_object(root);
	}
	while (true) { 
	    puts("---------------------------------------\n"
		 "Select action with AVL-tree:\n"
		 "\ti) insert new record\n"
		 "\ts) search record\n"
		 "\td) delete record\n"
		 "\tp) print list of record\n"
		 "\tg) garbage collection\n"
		 "\tr) random test\n"
		 "\tq) quit");
	    input("> ", buf, sizeof buf);

	    switch (*buf) { 
	      case 'i': case 'I':
		input("Input new record name: ", buf, sizeof buf);
		rec = record::create(buf);
		if (root->tree->insert(rec) != rec) {
		    printf("Record '%s' already exists\n", buf);
		} else { 
		    root->hash->put(buf, rec);
		}
		continue;
	      case 's': case 'S':
		input("Search for: ", buf, sizeof buf);
		rec = (record*)root->hash->get(buf);
		if (rec == NULL) { 
		    printf("No such record '%s' in tree\n", buf);
		} else { 
		    if (rec != root->tree->search(&key)) { 
			printf("Inconsistency in storage\n");
		    } else { 
			printf("Hits %d times\n", ++rec->hits);
		    }
		}
		continue;
	      case 'd': case 'D':
		input("Input record name to be removed: ", buf, sizeof buf);
		rec = (record*)root->hash->get(buf);
		if (rec == NULL) { 
		    printf("No such record '%s' in tree\n", buf);
		} else {
		    if (!root->tree->remove(rec)) { 
			printf("Failed to remove record from tree\n");
		    }
		    if (!root->hash->del(buf, rec)) { 
			printf("Failed to remove record from hash table\n");
		    }
		    delete rec;
		}
		continue;
	      case 'p': case 'P':
		rec = (record*)root->tree->next;
		while ((l2elem*)rec != root->tree) { 
		    printf("%s\n", rec->name);
		    rec = (record*)rec->next;
		}
		continue;
	      case 'g': case 'G':
		printf("Free %d objects\n", test_storage.garbage_collection());
		continue;
	      case 'r': case 'R':
		input("Specify number of iterations: ", buf, sizeof buf);
		random_test(atoi(buf));
		continue;
	      case 'q': case 'Q':
		input("Save changes Yes/No/Cancel: ", buf, sizeof buf);
		switch (*buf) { 
		  case 'y': case 'Y':
		    test_storage.flush();
		    test_storage.close();
		    return EXIT_SUCCESS;
		  case 'n': case 'N':
		    test_storage.rollback();
		    test_storage.close();
		    return EXIT_SUCCESS;
		} 
	    }
	}
    } else { 
	printf("Failed to open database\n");
	return EXIT_FAILURE;
    }
}

int main() 
{ 
    SEN_TRY { 
	return do_test();
    } SEN_ACCESS_VIOLATION_HANDLER(); 
    return EXIT_FAILURE;
}
