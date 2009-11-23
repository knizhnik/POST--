//-< TESTPERF.CXX >--------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example: test for T-tree (also used for measuring performance)
//-------------------------------------------------------------------*--------*

#include "judyarr.h"
#include <string.h>
#include <time.h>

USE_POST_NAMESPACE

const int nRecords = 1000000;

class Record : public object { 
  public:
    long ikey;
    char skey[1];

    static Record* create(storage& db, long ikey, const char* skey) { 
	return new (self_class, db, strlen(skey)) Record(ikey, skey);
    }

    Record(long ikey, char const* skey) { 
        this->ikey = ikey;
        strcpy(this->skey, skey);
    }

    CLASSINFO(Record, NO_REFS);
};

REGISTER(Record);

class Root : public object { 
  public:
    bit_array*    bitmap;
    sparse_array* array;
    dictionary*   dict;

    CLASSINFO(Root, REF(array) REF(bitmap) REF(dict));

    Root(storage& db) {
        dict = dictionary::create();
        bitmap = bit_array::create();
        array = sparse_array::create();
    }
};
    
REGISTER(Root);

int main() 
{ 
    const size_t maxStorageSize = 640*1024*1024;
    storage db("testjudy", maxStorageSize);

    if (db.open()) { 
        char buf[16];
        char pbuf[16];
        int  i;
        long index;
        long prev;
        Record* rec;
	Root* root = (Root*)db.get_root_object();
	if (root == NULL) { 
	    root = new_in(db, Root)(db);
	    db.set_root_object(root);
	}
	
        nat8 key = 2009;
        time_t start = time(NULL);
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621u*key + 2718281829u) % 1000000007;
            char buf[16];
            index = (long)key;
            sprintf(buf, "%lx", index);
            rec = Record::create(db, index,  buf);
            root->array->set(index, rec);
            root->dict->set(buf, rec);
            root->bitmap->set(index, true);
        }
	printf("Elapsed time for inserting %d objects: %d seconds\n", 
               nRecords, (time(NULL) - start));

        key = 2009;
        start = time(NULL);
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621u*key + 2718281829u) % 1000000007;
            index = (long)key;
            sprintf(buf, "%x", index);
            Record* rec1 = (Record*)root->array->get(index);
            Record* rec2 = (Record*)root->dict->get(buf);
            bool isSet = root->bitmap->get(index);
            assert(rec1 != NULL && rec1 == rec2 
                   && rec1->ikey == index && strcmp(rec2->skey, buf) == 0
                   && isSet);
        }
	printf("Elapsed time for %d lookups: %d seconds\n", 
               nRecords, (time(NULL) - start));

        key = 2009;
        start = time(NULL);
        i = 0;
        index = 0;
        prev = -1;
        for (rec = (Record*)root->array->first(index); rec != NULL; rec = (Record*)root->array->next(index))
        {
            assert(prev < index);
            prev = index;
            assert(rec->ikey == index);
            i += 1;
        }
        assert(i == nRecords);

        index = 0;
        prev = -1;
        bool found;
        i = 0;
        for (found = root->bitmap->first(index); found; found = root->bitmap->next(index))
        {
            assert(prev < index);
            prev = index;
            i += 1;
        }
        assert(i == nRecords);

        *buf = '\0';
        *pbuf = '\0';
        i = 0;
        for (rec = (Record*)root->dict->first(buf); rec != NULL; rec = (Record*)root->dict->next(buf))
        {
            assert(strcmp(pbuf, buf) < 0);
            strcpy(pbuf, buf);
            assert(strcmp(rec->skey, buf) == 0);
            i += 1;
        }
        assert(i == nRecords);
	printf("Elapsed time for iteration through %d objects: %d seconds\n", 
               nRecords, (time(NULL) - start));
        

        key = 2009;
        start = time(NULL);
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621u*key + 2718281829u) % 1000000007;
            index = (long)key;
            sprintf(buf, "%x", index);
            rec = (Record*)root->array->get(index);
            assert(rec->ikey == index);
            bool rc1 = root->array->remove(index);
            bool rc2 = root->dict->remove(buf);
            bool rc3 = root->bitmap->remove(index);
            assert(rc1 && rc2 && rc3);
            delete rec;
        }
	printf("Elapsed time for deleting %d objects: %d seconds\n", 
               nRecords, (time(NULL) - start));
	db.flush();
	db.close();
	return EXIT_SUCCESS;
    } else { 
	printf("Failed to open database\n");
	return EXIT_FAILURE;
    }
}
