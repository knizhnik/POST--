// Mode: -*- C++ -*-

//          GiSdb.h
//
// This provides interface beween GiST and GigaBASE storage

#ifndef GISTDB_H
#define GISTDB_H

#include "GiSTstore.h"
#include "ttree.h"

#define GIST_PAGE_SIZE (4096 - sizeof(object_header))

class GiST_POST_Page : public object {  
  public:
    char data[GIST_PAGE_SIZE];

    CLASSINFO(GiST_POST_Page, NO_REFS);
};

class GiST_POST_Tree : public object { 
  public:
    GiST_POST_Page* root;
    char            name[1];
    
    CLASSINFO(GiST_POST_Tree, REF(root));

    GiST_POST_Tree(storage& where, char const* name) 
    {
	strcpy(this->name, name);
	root = new_in(where, GiST_POST_Page);
    }	 

    static GiST_POST_Tree* create(storage& where, const char* name) {
	return new (self_class, where, strlen(name)) GiST_POST_Tree(where, name); 
    }
};

class GiST_POST_Root : public object { 
  public:
    dbTtree* index;
    
    CLASSINFO(GiST_POST_Root, REF(index));

    GiST_POST_Root(storage& where) { 
	index = new (self_class, where) dbTtree();
    }
};

class GiST_POST_Context : public dbSearchContext { 
  public:
    virtual int compare(void const* key, object* obj) { 
	return strcmp((char const*)key, ((GiST_POST_Tree*)obj)->name);
    }
};

class GiST_POST_Relation : public dbRelation { 
  public:
    virtual int compare(object* o1, object* o2) { 
	return strcmp(((GiST_POST_Tree*)o1)->name, ((GiST_POST_Tree*)o2)->name);
    }
}; 

class GiSTdb : public GiSTstore {
  public:
    GiSTdb(storage& sto);

    void Create(const char *filename);
    void Open(const char *filename);
    void Close();
    
    void Read(GiSTpage page, char *buf);
    void Write(GiSTpage page, const char *buf);
    GiSTpage Allocate();
    void Deallocate(GiSTpage page);
    void Sync() {}
    int  PageSize() const { return GIST_PAGE_SIZE; }
    
  private:
    GiST_POST_Page*    rootPage;
    storage&           repository;
};

#endif
