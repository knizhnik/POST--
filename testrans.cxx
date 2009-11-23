#include "avltree.h"
#include <time.h>

USE_POST_NAMESPACE

class record : public object, public l2elem { 
  public:
    char    buf[4000];
    int     code;

    record(int i) { prune(); code = i; }
    CLASSINFO(record, NO_REFS);
};    
	
REGISTER(record);

const size_t max_storage_size = 64*1024*1024;
storage tstor("testrans", max_storage_size);

int main(int argc, char* argv[]) 
{ 
    SEN_TRY { 
	int code;
	
	if (tstor.open(storage::use_transaction_log)) { 
	    const int n_records = 10000;
	    int i, n;
	    time_t start_time;
	    record* root = (record*)tstor.get_root_object();
	    if (root == NULL) { 
		start_time = time(NULL);
		printf("Initializing database...\n");
		root = new_in(tstor, record)(0);
		tstor.set_root_object(root);
		for (i = 1; i < n_records; i++) { 
		    (new_in(tstor, record)(i))->link_after(root);
		}
		tstor.commit();
		printf("Elapsed time for initialization = %d\n", 
		       int(time(NULL) - start_time));
	    }
	    l2elem* rp = root;
	    i = 1;
	    printf("Checking database consistency...\n");
	    while ((rp = rp->next) != root) { 
		i += 1;
		assert(rp->prev->next==rp && ((record*)rp)->code < n_records);
		if ((i & 1023)  == 0) { 
		    ((record*)rp)->code = -1;
		}
	    }
	    new_in(tstor, record)(n_records);
	    assert(i == n_records);
	    printf("Checking rollback...\n");
	    tstor.rollback();
	    i = 1;
	    while ((rp = rp->next) != root) { 
		i += 1;
		assert(rp->prev->next==rp && ((record*)rp)->code < n_records);
	    }	
	    assert(i == n_records);
	    if (argc > 1) { 
		n = atoi(argv[1]);
	    } else { 
		n = 100;
	    }
	    printf("Performing %d transactions...\n", n);
	    start_time = time(NULL);
	    for (i = 0; i < n; i++) { 
		rp = root->next;
		rp->unlink();
		rp->link_before(root);
		tstor.commit();
	    }
	    printf("Elapsed time for %d transactions: %d\n", 
		   n, int(time(NULL) - start_time));
	    tstor.close(); 
	    code = EXIT_SUCCESS;
	} else { 
	    printf("Failed to open storage\n");
	    code = EXIT_FAILURE;
	}
	return code;
    } SEN_ACCESS_VIOLATION_HANDLER(); 
    return EXIT_FAILURE;
}
