//-< DUMPMEM.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 16-Nov-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Program for analyzing POST++ storage memory fragmentation
//-------------------------------------------------------------------*--------*

#include "object.h"

USE_POST_NAMESPACE

class storage_view : public storage { 
  public:
    storage_view(char const* name) : storage(name) {}
    void load_class_dictionary() {
	app_class_mapping = NULL;
	dbs_class_mapping = NULL;
    }
    bool open() { 
	return storage::open(storage::read_only);
    }
    void dump();
};


int main(int argc, char* argv[]) 
{
    if (argc < 2) { 
	fprintf(stderr, "dumpmem: Storage memory fragmentation analyzer\n"
		"Usage: dumpmem <storage-name>\n");
	return EXIT_FAILURE;
    }
    storage_view store(argv[1]);
    if (store.open()) { 
	store.dump();
	store.close();
	return EXIT_SUCCESS;
    }  
    return EXIT_FAILURE;
}

const int alloc_block_size[13] = { 
    16,  24,  32,  40,  48,  56,  64,  72,  80,  96,  128, 168, 256
};

void storage_view::dump()
{
    storage_free_block* bp;
    int n_free_pages = 0;
    int n_holes = 0;
    int i, n;
    for (bp = hdr->free_page_chain; bp != NULL; bp = bp->next) { 
	n_free_pages += (bp->size + sizeof(object_header)) / page_size;
	n_holes += 1;
    }
    int free_blocks[items(hdr->free_block_chain)];
    size_t free_space = n_free_pages * page_size;
    for (i = items(hdr->free_block_chain); --i >= 0;) { 
	for (n = 0, bp = hdr->free_block_chain[i]; bp != NULL; bp = bp->next) {
	    n += 1;
	}
	free_blocks[i] = n;
	free_space += n*alloc_block_size[i]; 
    }
    object_header* op = (object_header*)((char*)hdr + page_size);
    object_header* end = (object_header*)((char*)hdr + hdr->file_size);
    
    int block_used[items(hdr->free_block_chain)];
    int block_free[items(hdr->free_block_chain)];
    int block_pages[items(hdr->free_block_chain)];
    size_t block_fragmentation = 0;
    size_t page_fragmentation = 0;
    size_t n_objects = 0;
    size_t used_space = page_size;
    int free_pages = 0;
    int holes = 0;
    memset(block_pages, 0, sizeof block_pages);
    memset(block_used, 0, sizeof block_used);
    memset(block_free, 0, sizeof block_free);
    while (op < end) { 
	size_t obj_size = op->size + sizeof(object_header);
	if (obj_size <= page_size/2) { 
	    n = ((obj_size+7) >> 3) - 1;
	    i = block_chain[n];
	    block_pages[i] += 1;
	    int size = block_size[n];
	    char* end_of_page = (char*)op + page_size;
	    do { 
		if (op->cid != object_header::free_object) { 
		    block_used[i] += 1;
		    block_fragmentation += size - op->size;
		    used_space += op->size;
		    n_objects += 1;
		} else { 
		    block_free[i] += 1;
		}
		op = (object_header*)((char*)op + size);
	    } while ((char*)op + size <= end_of_page);
	    block_fragmentation += page_size % size;
	    op = (object_header*)end_of_page;
	} else {
	    if (op->cid != object_header::free_object) { 
		n_objects += 1;
		page_fragmentation += 
		    (-(int)obj_size & (page_size-1)) + sizeof(object_header);
		used_space += op->size;
		int n_pages = (obj_size + page_size - 1) / page_size;
		int page_no = ((char*)op - (char*)hdr) / page_size;
		if (hdr->page_map != NULL 
		    && op->cid >= (int)object_header::first_cid)
		{ 
		    char* bitmap = hdr->page_map->bits;
		    assert((bitmap[page_no >> 3] & (1 << (page_no & 7))) == 0);
		    while (--n_pages != 0) {
			page_no += 1;
			assert(bitmap[page_no >> 3] & (1 << (page_no & 7)));
		    }
		}
	    } else { 
		int n_pages = obj_size / page_size;
		free_pages += n_pages;
		holes += 1;
	    }
	    op = (object_header*)((char*)op + ((obj_size + page_size - 1)
				  & ~(page_size-1)));
	}
    }
    assert(n_free_pages == free_pages);
    assert(n_holes == holes);
    
    for (i = items(block_used); --i >= 0;) { 
	assert(block_free[i] == free_blocks[i]);
	assert(block_pages[i]*(page_size/alloc_block_size[i]) == 
	        block_used[i] + block_free[i]);
	printf("Blocks of size %d: used pages %d, used blocks %d, free blocks %d, usage %d%%\n", 
	       alloc_block_size[i], block_pages[i], block_used[i], block_free[i], block_pages[i] ? block_used[i]*100/(block_used[i] + block_free[i]) : 0);
    }
    printf("Free pages: %d, holes: %d, average hole size: %g\n", 
	   n_free_pages, n_holes, n_holes ? double(n_free_pages) / n_holes : 0);
    printf("Total objects in storage: %d\n\n", n_objects);

    printf("Storage size:            %10d\n", hdr->file_size);
    printf("  Free space:            %10d\n", free_space);
    printf("  Used by objects:       %10d\n", used_space);
    printf("  Fragmentation:         %10d (%d%%)\n", 
	   hdr->file_size - used_space - free_space, 
	   (hdr->file_size - used_space - free_space)*100/used_space);
    printf("    Page fragmentation:  %10d (%d%%)\n", 
	   page_fragmentation, page_fragmentation*100/used_space);
    printf("    Block fragmentation: %10d (%d%%)\n",
	   block_fragmentation, block_fragmentation*100/used_space);
}





