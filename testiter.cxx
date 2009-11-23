//-< TESTITER.CXX >--------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example: test for T-tree (also used for measuring performance)
//-------------------------------------------------------------------*--------*

#include "ttree.h"
#include "iterator.h"
#include <time.h>

USE_POST_NAMESPACE

const int nRecords = 10000;
const int nInsertedRecords = 100000;//0;

class Record : public object { 
  public:
    nat8 key;

    CLASSINFO(Record, NO_REFS);
};

REGISTER(Record);

class RecordSearchCtx : public dbSearchContext { 
  public:
    virtual int compare(void const* key, object* obj) { 
        return *(nat8*)key > ((Record*)obj)->key 
            ? 1 : *(nat8*)key == ((Record*)obj)->key ? 0 : -1;
    }
};

class RecordRelation : public dbRelation { 
  public:
    virtual int compare(object* o1, object* o2) { 
        return ((Record*)o1)->key > ((Record*)o2)->key 
            ? 1 : ((Record*)o1)->key == ((Record*)o2)->key ? 0 : -1;
    }
}; 

int main() 
{ 
    const size_t maxStorageSize = 64*1024*1024;
    storage test_storage("testperf", maxStorageSize);

    if (test_storage.open()) { 
        int8 inskey = 1999;
        int8 remkey = 1999;

        dbTtree* root = (dbTtree*)test_storage.get_root_object();
        if (root == NULL) { 
            root = new_in(test_storage, dbTtree);
            test_storage.set_root_object(root);
        }
        
        int i = 0, j = 0, n = 0, r;
        RecordRelation  relation;
        RecordSearchCtx ctx;
        search_buffer   sb;

        ctx.firstKeyInclusion = true;
        ctx.lastKeyInclusion = true;
        ctx.selectionLimit = 0;
        ctx.buffer = &sb;

        time_t start = time(NULL);
        while (i < nInsertedRecords) { 
            if (n >= nRecords) {
                remkey = (3141592621u*nat8(remkey) + 2718281829u) % 1000000007;
                ctx.firstKey = &remkey;
                ctx.lastKey = &remkey;
                r = root->find(ctx);
                assert(r == (remkey & 0xFF) + 1);
                n -= r;
                while (--r >= 0) { 
                    root->remove(sb[r], relation);
                    delete sb[r];
                }
            }
            inskey = (3141592621u*nat8(inskey) + 2718281829u) % 1000000007;
            r = int(inskey & 0xFF) + 1;
            size_t size = int(inskey >> 8) & 0xFFF;
            n += r;
            i += r;
            while (--r >= 0) { 
                Record* rec = 
                    new (Record::self_class, test_storage, size) Record;
                rec->key = inskey;
                root->insert(rec, relation);
            }
            if (i > j) { 
                printf("Insert %d objects...\r", i);
                fflush(stdout);
                j = i + 1000;
            }
        }
        printf("Elapsed time for %d objects: %d seconds\n", 
               nInsertedRecords, int(time(NULL) - start));

        long itCount = 0;
        Ttree_iterator it(root);
        while (it.IsValid())
        {
            Record* ob = (Record*) (*it);
            printf("key=%"INT8_FORMAT"d\n", ob->key);
            // do something with the Record object...
            itCount++;
            ++it;
        }
        printf("Iterator found %ld remaining unique Records in the tree\n", itCount);

        itCount = 0;        
        Ttree_iterator r_it(root, true);
        while (r_it.IsValid())
        {
            Record* ob = (Record*) (*r_it);
            printf("key=%"INT8_FORMAT"d\n", ob->key);
            // do something with the Record object...
            itCount++;
            --r_it;
        }
        printf("Reverse iterator found %ld remaining unique Records in the tree\n", itCount);

        test_storage.flush();
        test_storage.close();
        return EXIT_SUCCESS;
    } else { 
        printf("Failed to open database\n");
        return EXIT_FAILURE;
    }
}
