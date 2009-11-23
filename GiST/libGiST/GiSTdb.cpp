// -*- Mode: C++ -*-
//
//          GiSTdb.cpp

#include "GiSTdb.h"
#include "GiSTpath.h"

REGISTER(GiST_POST_Page);
REGISTER(GiST_POST_Tree);
REGISTER(GiST_POST_Root);

GiSTdb::GiSTdb(storage& sto) : repository(sto) {}


void 
GiSTdb::Create(const char *tableName)
{
    GiST_POST_Context  ctx;
    GiST_POST_Relation relation;
    search_buffer      sb;    

    if (IsOpen()) { 
	return;
    }
    SetOpen(1);

    ctx.firstKeyInclusion = true;
    ctx.lastKeyInclusion = true;
    ctx.selectionLimit = 0;
    ctx.buffer = &sb;
    
    GiST_POST_Root* root = (GiST_POST_Root*)repository.get_root_object();    
    GiST_POST_Tree* tree;

    if (root->index->find(ctx) == 0) { 
	tree = GiST_POST_Tree::create(repository, tableName);
	root->index->insert(tree, relation);	
    } else { 
	tree = (GiST_POST_Tree*)sb[0];
    }
    rootPage = tree->root;
}

void 
GiSTdb::Open(const char *tableName)
{
    GiST_POST_Context  ctx;
    search_buffer      sb;    

    ctx.firstKeyInclusion = true;
    ctx.lastKeyInclusion = true;
    ctx.selectionLimit = 0;
    ctx.buffer = &sb;
    
    GiST_POST_Root* root = (GiST_POST_Root*)repository.get_root_object();    
    if (root->index->find(ctx) != 0) { 
	GiST_POST_Tree* tree = (GiST_POST_Tree*)sb[0];
	rootPage = tree->root;
	SetOpen(1);
    }
}

void 
GiSTdb::Close()
{
    if (!IsOpen()) { 
	return;
    }
    SetOpen(0);
}

void 
GiSTdb::Read(GiSTpage page, char *buf)
{
    if (IsOpen()) {
	GiST_POST_Page* pg = (page == GiSTRootPage)? rootPage : (GiST_POST_Page*)page;
	memcpy(buf, pg, GIST_PAGE_SIZE);
    }
}

void 
GiSTdb::Write(GiSTpage page, const char *buf)
{
    if (IsOpen()) {
	GiST_POST_Page* pg = (page == GiSTRootPage)? rootPage : (GiST_POST_Page*)page;
	memcpy(pg, buf, GIST_PAGE_SIZE);
    }
}

GiSTpage 
GiSTdb::Allocate()
{
    if (!IsOpen()) { 
	return 0;
    }
    return (GiSTpage)new_in(repository, GiST_POST_Page);
}

void 
GiSTdb::Deallocate(GiSTpage page)
{
    delete (GiST_POST_Page*)page;
}








