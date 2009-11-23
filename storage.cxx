//-< STORAGE.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 21-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Memory allocation, garbage collection, data storing/retrieving
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "object.h"    

BEGIN_POST_NAMESPACE

simple_mutex storage::global_mutex;

class storage_class_descriptor : public object { 
  public:
    storage_class_descriptor* next;
    size_t size;
    char   name[1];

    CLASSINFO(storage_class_descriptor, REF(next));
};

REGISTER(storage_class_descriptor);


static void storage_page_map_constructor(object*) {}
static class_descriptor storage_page_map_class("storage_page_map", 
					       sizeof(storage_page_map),
					       &storage_page_map_constructor);


ptrdiff_t storage::base_address_shift;
gc_segment* storage::gc_stack;
storage* storage::chain;

#ifdef LARGE_OBJECTS

const int storage::block_chain[64] = { 
     0,  0,  1,  2,  3,  4,  5,  6,  7,  8, 
     9, 10, 11, 11, 11, 11, 12, 12, 13, 13, 
    13, 14, 14, 14, 14, 15, 15, 15, 15, 15, 
    15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 
    16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17
};

const int storage::block_size[64] = { 
     16,  16,  24,  32,  40,  48,  56,  64,  72,  80,  
     88,  96, 128, 128, 128, 128, 144, 144, 168, 168, 
    168, 204, 204, 204, 204, 256, 256, 256, 256, 256, 
    256, 256, 336, 336, 336, 336, 336, 336, 336, 336, 
    336, 336, 512, 512, 512, 512, 512, 512, 512, 512, 
    512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 
    512, 512, 512, 512
}; 

#else

const int storage::block_chain[32] = { 
     0,  0,  1,  2,  3,  4,  5,  6,  7,  8, 
     9,  9, 10, 10, 10, 10, 11, 11, 11, 11, 
    11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 
    12, 12
};

const int storage::block_size[32] = { 
     16,  16,  24,  32,  40,  48,  56,  64,  72,  80,  
     96,  96, 128, 128, 128, 128, 168, 168, 168, 168, 
    168, 256, 256, 256, 256, 256, 256, 256, 256, 256, 
    256, 256
};
#endif


inline void storage_header::adjust_references(ptrdiff_t shift)
{
    int i;
    if (page_map != NULL) { 
	*(char**)&page_map += shift;
    }
    for (i = items(free_block_chain); --i >= 0;) { 
	if (free_block_chain[i] != NULL) { 
	    *(char**)&free_block_chain[i] += shift;
	}
    }
    if (free_page_chain != NULL) { 
	*(char**)&free_page_chain += shift;
    }
    if (root_object != NULL) {
	*(char**)&root_object += shift;
    }
    if (class_dictionary != NULL) {
	*(char**)&class_dictionary += shift;
    }
    for (i = n_static_data; --i >= 0;) { 
	*(char**)&static_data[i] += shift;
    }
}

void storage::handle_error(const char* func) 
{ 
    post_file::msg_buf buf;
    data_file.get_error_text(buf, sizeof buf);
    fprintf(stderr, "file::%s: %s\n", func, buf);
    abort();
}

bool storage::open(int flags
#ifdef LARGE_OBJECTS
                   , int // this parameter is added only to ensure that both POST library and user application 
                         // were compiled with equal state of LARGE_OBJECTS macro
#endif
                   )
{
    CS2(global_mutex, mutex);
    if (data_file.open((flags & use_transaction_log) 
		        ? post_file::shadow_pages_transaction
		        : (flags & no_file_mapping) 
		           ? post_file::load_in_memory 
		           : (flags & fault_tolerant)
		             ? post_file::copy_on_write_map 
		             : post_file::map_file,
		       (flags & read_only) 
		        ? post_file::read_only : post_file::read_write))
    {
	hdr = (storage_header*)data_file.get_base();
	assert((size_t(hdr) & (page_size-1)) == 0/*check proper alignment*/); 
	if (data_file.get_size() < sizeof(storage_header)) { 
	    if (!data_file.set_size(page_size)) { 
		handle_error("set_size");
	    }
	    if (flags & read_only) { 
		TRACE_MSG(("Empty file opened in readonly mode\n"));
		return false;
	    }
	    memset(hdr, 0, page_size);
	}
	load_class_dictionary();
	bool file_updated = false;
	if (hdr->base_address != hdr 
	    || (flags & do_garbage_collection)
	    || ((flags & support_virtual_functions)
		&& strcmp(hdr->timestamp, post_file::get_program_timestamp()) != 0))
	{
	    TRACE_MSG(("storage::open: %s\n", 
		  (hdr->base_address != hdr) ? "base_address changed"
		  : (flags & do_garbage_collection) ? "do garbage collection"
		  : "executable file and strorage have different timestamps"));
	    if (flags & read_only) { 
		if (!data_file.set_protection(post_file::read_write)) { 
		    handle_error("set_protection");
		}
	    }
	    if (hdr->base_address != NULL) { 
		base_address_shift = (char*)hdr - (char*)hdr->base_address;
		if (base_address_shift != 0) { 
		    if (flags & fixed) { 
			TRACE_MSG(("Failed to map to the same address\n"));
			return false;
		    }
#ifndef _WIN64
		    TRACE_MSG(("storage::open: adjust references: shift=%ld\n",
			       base_address_shift));
#else
		    TRACE_MSG(("storage::open: adjust references: shift=%i64>\n",
			       base_address_shift));
#endif
		    hdr->adjust_references(base_address_shift);
		}
		if (flags & do_garbage_collection) { 
		    garbage_collection(base_address_shift);
		} else { 
		    adjust_references(base_address_shift);
		}
		base_address_shift = 0;
	    } else {  
		// initialize storage
		hdr->file_size = page_size;
		hdr->version = POST_VERSION;
	    }
	    file_updated = true;
	    hdr->base_address = (void*)hdr;
	    if (flags & support_virtual_functions) { 
		strcpy(hdr->timestamp, post_file::get_program_timestamp());
	    } else { 
		*hdr->timestamp = '\0';
	    }
	    
	    if (flags & read_only) { 
		if (!data_file.set_protection(post_file::read_only)) { 
		    handle_error("set_protection");
		}
	    }
	}
	curr_free_page = hdr->free_page_chain;
	if (!(flags & read_only)) { 
	    file_updated |= update_class_dictionary();
	    if (file_updated && (flags & use_transaction_log)) { 
		if (!data_file.commit()) { 
		    handle_error("commit");
		}
	    }
	}
	next = chain;
	chain = this;
	return true;
    }
    return false;
}

void storage::load_class_dictionary()
{
    int count = hdr->n_classes + object_header::first_cid;
    ptrdiff_t shift = (char*)hdr - (char*)hdr->base_address;
    storage_class_descriptor* scd;    
    class_descriptor* cd;

    dbs_class_mapping = new class_descriptor*[count + class_descriptor::count];
    app_class_mapping = new int[class_descriptor::count];
    memset(dbs_class_mapping, 0, 
	   sizeof(class_descriptor*)*(count + class_descriptor::count));
    memset(app_class_mapping, 0, sizeof(int)*class_descriptor::count);

    cd = &storage_class_descriptor::self_class;
    app_class_mapping[cd->id] = object_header::metaclass_cid;
    dbs_class_mapping[object_header::metaclass_cid] = cd;

    cd = &storage_page_map_class;
    app_class_mapping[cd->id] = object_header::page_map_cid;
    dbs_class_mapping[object_header::page_map_cid] = cd;
    
    for (scd = hdr->class_dictionary; scd != NULL; scd = scd->next) { 
	scd = (storage_class_descriptor*)((char*)scd + shift);
	count -= 1;
	cd = class_descriptor::find_class(scd->name);
#if DEBUG_LEVEL >= DEBUG_CHECK
        printf("class name=%s, cd=%p\n", scd->name, cd);
	assert(cd != NULL && cd->size == scd->size);
#endif
	if (cd != NULL) { 
	    app_class_mapping[cd->id] = count;
	    dbs_class_mapping[count] = cd;
	} 
    }
} 

bool storage::update_class_dictionary() 
{
    int count = hdr->n_classes + object_header::first_cid;
    int n_stored_classes = 0;
    storage_class_descriptor* scd;
    class_descriptor* cd;

    for (cd = class_descriptor::list; cd != NULL; cd = cd->next) { 
	if (app_class_mapping[cd->id] == 0) { 
	    app_class_mapping[cd->id] = count;
	    dbs_class_mapping[count++] = cd;
	    scd = (storage_class_descriptor*)
		_allocate(storage_class_descriptor::self_class.id, 
			  (unsigned)(sizeof(storage_class_descriptor)
                                     + strlen(cd->name)));
	    strcpy(scd->name, cd->name);
	    scd->size = cd->size;
	    scd->next = hdr->class_dictionary;
	    hdr->class_dictionary = scd;
	    n_stored_classes += 1;
	    TRACE_MSG(("storage::update_class_dictionary: put class '%s' "
		       "size=%ld in storage\n", cd->name, cd->size));
	}
    }
    hdr->n_classes = count - object_header::first_cid;
    return n_stored_classes != 0;
}

object_header* storage::get_header(object const* obj) 
{ 
    object_header* hp;
    size_t addr = size_t(obj) - sizeof(object_header);
    size_t offs = addr - size_t(hdr);
    assert(offs < hdr->file_size);
    size_t page_no = offs / page_size;

    if (hdr->page_map == NULL 
	|| !(hdr->page_map->bits[page_no >> 3] & (1 << (page_no & 7))))
    { 
	hp = (object_header*)(addr & ~(page_size-1));
	if (hp->size + sizeof(object_header) <= page_size/2) { 
	    size_t size = 
		block_size[((hp->size + sizeof(object_header)+7) >> 3) - 1];
	    if (size & (size-1)) { 
		offs &= page_size-1; //offset within page
		hp = (object_header*)(addr - (offs % size));
	    } else { // power of 2
		hp = (object_header*)(addr & ~(size-1));
	    }
	} 
    } else { // large object occupies several pages
	do { 
	    page_no -= 1;
	} while (hdr->page_map->bits[page_no >> 3] & (1 << (page_no & 7)));
	hp = (object_header*)((char*)hdr + page_no*page_size);
    }
    assert(hp->cid != object_header::free_object);
    return hp;
}

size_t storage::garbage_collection()
{
    CS2(global_mutex, mutex);
    base_address_shift = 0;
    return garbage_collection(0);
}

size_t storage::garbage_collection(ptrdiff_t shift)
{
    const size_t gc_init_stack_size = 64*1024;
    gc_segment* stack_bottom = new gc_segment[gc_init_stack_size];
    gc_segment* end_of_stack = stack_bottom + gc_init_stack_size;
    gc_segment* sp = stack_bottom;
    if (hdr->root_object) {
	sp->ptr = (object**)&hdr->root_object;
	sp->len = 1;
	sp += 1;
	// Eliminate shift done by storage_header::adjust_reference 
	// to make it possible to work with this reference as with other ones
	*(char**)&hdr->root_object -= shift; 
    }
    if (hdr->class_dictionary) { 
	sp->ptr = (object**)&hdr->class_dictionary;
	sp->len = 1;
	sp += 1;
	*(char**)&hdr->class_dictionary -= shift; 
    }
    if (hdr->n_static_data > 0) { 
	sp->ptr = hdr->static_data;
	sp->len = hdr->n_static_data;
	sp += 1;
	for (int i = hdr->n_static_data; --i >= 0;) { 
	    *(char**)&hdr->static_data[i] -= shift;
	}
    }
    if (hdr->page_map) { 
	hdr->page_map->cid |= object_header::gc_marked;
    }
    while (sp != stack_bottom) { 
	sp -= 1;
	object** opp = sp->ptr;
	size_t len = sp->len;
	do { 
	    if (*opp != NULL) { 
		*(char**)opp += shift;
		object_header* hp = get_header(*opp);
		if (!(hp->cid & object_header::gc_marked)) { 
		    class_descriptor* desc = get_object_class(hp->cid);
		    hp->cid |= object_header::gc_marked;
		    //
		    // Number of pointer segments in object is not greater than
		    // object_size/pointer_size, and so the size of stack 
		    // needed for storing this segments is not more 
		    // than 2 * size of the object.
		    //
		    if ((char*)sp + 2*hp->size >= (char*)end_of_stack) { 
			size_t old_size = sp - stack_bottom;
			size_t new_size = 
			    old_size*2 + (hp->size / sizeof(object*));
			gc_segment* new_stack = new gc_segment[new_size];
			memcpy(new_stack, stack_bottom, 
			       old_size*sizeof(gc_segment));
			sp = new_stack + old_size;
			delete[] stack_bottom;
			stack_bottom = new_stack;
			end_of_stack = new_stack + new_size;
		    }
		    gc_stack = sp;
		    desc->constructor((object*)(hp+1));
		    sp = gc_stack;
		}
	    }
	    opp += 1;
	} while (--len != 0);
    }
    delete[] stack_bottom;
    gc_stack = NULL;

    curr_free_page = NULL;
    hdr->free_page_chain = NULL; // will be reconstructed
    memset(hdr->free_block_chain, 0, sizeof hdr->free_block_chain);

    // ... and sweep
    size_t n_deallocated_objects = 0;
    size_t n_free_objects = 0;
    size_t n_used_objects = 0;
    size_t used_size = 0;
    storage_free_block* bp = (storage_free_block*)((char*)hdr + page_size);
    storage_free_block* end_of_storage = 
	(storage_free_block*)((char*)hdr + hdr->file_size);
    while (bp < end_of_storage) { 
	size_t size = bp->size + sizeof(object_header);
	if (size > page_size/2) { // large object
	    if (bp->cid & object_header::gc_marked) {
		bp->cid &= ~object_header::gc_marked;
		n_used_objects += 1;
		used_size += size;
	    } else { 
		n_deallocated_objects += bp->cid != object_header::free_object;
		n_free_objects += 1;
		free_page(bp);
	    }
	    bp = (storage_free_block*)ALIGN((ptrdiff_t)bp + size, page_size);
	} else { 			    
	    size_t n = ((size+7) >> 3) - 1;
	    size = block_size[n];
	    storage_free_block* end_of_page = 
		(storage_free_block*)((char*)bp + page_size - size + 1);
	    storage_free_block chain_hdr;
	    storage_free_block* chain = &chain_hdr; 
	    size_t used = n_used_objects;
	    do { 
		if (bp->cid & object_header::gc_marked) {
		    bp->cid &= ~object_header::gc_marked;
		    n_used_objects += 1;
		    used_size += size;
		} else { 
		    n_deallocated_objects += 
			bp->cid != object_header::free_object;
		    n_free_objects += 1;
		    bp->cid = object_header::free_object;
		    chain = chain->next = bp;
		}
		bp = (storage_free_block*)((char*)bp+size);
	    } while (bp < end_of_page);

	    if (used == n_used_objects) { 
		// there are no used objects at this page
		free_page(chain_hdr.next); // free the whole page
	    } else {
		chain->next = hdr->free_block_chain[block_chain[n]];
		hdr->free_block_chain[block_chain[n]] = chain_hdr.next;
	    } 
	    bp = (storage_free_block*)ALIGN((ptrdiff_t)bp, page_size);
	}
    }

    TRACE_MSG(("Free %ld objects\n"
	       "Total in storage %ld free and %ld used objects\n"
	       "Currently %ld bytes out of %ld are used\n", 
	       n_deallocated_objects, n_free_objects, n_used_objects,
	       used_size, hdr->file_size));
    return n_deallocated_objects;
}
		     
inline void storage::call_constructor(object_header* hp) 
{ 
    get_object_class(hp->cid)->constructor((object*)(hp+1));
}


void storage::adjust_references(ptrdiff_t shift)
{
    storage_free_block* bp = (storage_free_block*)((char*)hdr + page_size);
    storage_free_block* end_of_storage = 
	(storage_free_block*)((char*)hdr + hdr->file_size);
    if (shift == 0) { // minimize number of modified objects
	do { 
	    size_t size = bp->size + sizeof(object_header);
	    if (size > page_size/2) { // large object
		if (bp->cid != object_header::free_object) {
		    call_constructor(bp);
		}
		bp = (storage_free_block*)ALIGN((ptrdiff_t)bp + size, page_size);
	    } else { 			    
		size = block_size[((size+7) >> 3) - 1];
		storage_free_block* end_of_page = 
		    (storage_free_block*)((char*)bp + page_size - size + 1);
		do { 
		    if (bp->cid != object_header::free_object) { 
			call_constructor(bp);
		    }
		    bp = (storage_free_block*)((char*)bp+size);
		} while (bp < end_of_page);
		bp = (storage_free_block*)ALIGN((ptrdiff_t)bp, page_size);
	    }
	} while (bp < end_of_storage);
    } else { // shift != 0
	do { 
	    size_t size = bp->size + sizeof(object_header);
	    if (size > page_size/2) { // large object
		if (bp->cid == object_header::free_object) {
		    if (bp->next != NULL) { 
			*(char**)&bp->next += shift;
		    }
		} else { 
		    call_constructor(bp);
		}
		bp = (storage_free_block*)ALIGN((ptrdiff_t)bp + size, page_size);
	    } else { 			    
		size = block_size[((size+7) >> 3) - 1];
		storage_free_block* end_of_page = 
		    (storage_free_block*)((char*)bp + page_size - size + 1);
		do { 
		    if (bp->cid == object_header::free_object) { 
			if (bp->next != NULL) { 
			    *(char**)&bp->next += shift;
			}
		    } else { 
			call_constructor(bp);
		    }
		    bp = (storage_free_block*)((char*)bp+size);
		} while (bp < end_of_page);
		bp = (storage_free_block*)ALIGN((ptrdiff_t)bp, page_size);
	    }
	} while (bp < end_of_storage);
    }
}

void storage::flush()
{
    CS(mutex);
    if (!data_file.flush()) { 
	handle_error("flush");
    }
}

void storage::commit()
{
    CS(mutex);
    if (!data_file.commit()) { 
	handle_error("commit");
    }
}

void storage::rollback()
{
    CS(mutex);
    if (!data_file.rollback()) { 
	handle_error("rollback");
    } else { 
	curr_free_page = hdr->free_page_chain;
    }
}

void storage::close()
{
    CS2(global_mutex, mutex);

    delete[] app_class_mapping;
    delete[] dbs_class_mapping;

    storage *sp, **spp = &chain;
    while ((sp = *spp) != this) { 
	spp = &sp->next;
    }
    *spp = sp->next;

    if (!data_file.close()) { 
	handle_error("close");
    }    
}


void* storage::alloc_page(int cid, unsigned size)
{
    storage_free_block* bp = NULL;
    size_t aligned_size = ALIGN(size + sizeof(object_header), page_size);
    if (hdr->free_page_chain != NULL) { 
	storage_free_block** bpp = &curr_free_page->next;
	while ((bp = *bpp) != NULL && bp->size < size) { 
	    bpp = &bp->next;
	}
	if (bp == NULL) { 
	    bpp = &hdr->free_page_chain;
	    storage_free_block* save_next = curr_free_page->next;
	    curr_free_page->next = NULL;
	    while ((bp = *bpp) != NULL && bp->size < size) { 
		bpp = &bp->next;
	    }
	    curr_free_page->next = save_next;
	}
	if (bp != NULL) { 
	    if (bp->size > aligned_size) { 
		curr_free_page = bp;
		bp->size -= aligned_size;
		bp = (storage_free_block*)
		    ((char*)bp + bp->size + sizeof(object_header));
	    } else { 
		*bpp = bp->next;
		curr_free_page = bp->next ? bp->next : hdr->free_page_chain;
	    } 	    
	} 
    } 
    if (bp == NULL) { 
	bp = (storage_free_block*)((char*)hdr + hdr->file_size);
	if (!data_file.set_size(hdr->file_size + aligned_size)) { 
	    handle_error("set_size");
	} 
	hdr->file_size += aligned_size;
	if (hdr->page_map != NULL && cid != object_header::page_map_cid) { 
	    size_t old_map_size = hdr->page_map->size;
	    if (hdr->file_size > old_map_size*page_size*8) {
		// file_size+map_size+map_header >= page_size*8*map_size
		size_t new_map_size = 
		    (hdr->file_size + sizeof(object_header) + page_size*8-2)
 		                       / (page_size*8 - 1);
		// reserve space for future
		new_map_size = ALIGN(new_map_size + sizeof(object_header),
				     page_size)*2 - sizeof(object_header); 
		TRACE_MSG(("storage::alloc: reallocate page map, old size=%ld,"
			   " new size=%ld\n", old_map_size, new_map_size));
		storage_page_map* new_map = (storage_page_map*)
		    ((char*)alloc_page(object_header::page_map_cid, 
				       new_map_size)-sizeof(object_header));
		memcpy(new_map->bits, hdr->page_map->bits, old_map_size);
		memset(new_map->bits + old_map_size, 0, 
		       new_map_size - old_map_size);
		free_page(hdr->page_map);
		hdr->page_map = new_map;
	    }
	} 
    }
    bp->cid = cid;
    bp->size = size;
    if (size + sizeof(object_header) <= page_size/2) { 
        size_t n = ((size + sizeof(object_header) + 7) >> 3) - 1;
	size = block_size[n];
	storage_free_block* end_of_page = 
	    (storage_free_block*)((char*)bp + page_size - size + 1);
	storage_free_block* cp = (storage_free_block*)((char*)bp + size);
	do { 
	    cp->cid = object_header::free_object;
	    cp = cp->next = (storage_free_block*)((char*)cp + size);
	} while (cp < end_of_page);
	cp = (storage_free_block*)((char*)cp - size);
	cp->next = hdr->free_block_chain[block_chain[n]];
	hdr->free_block_chain[block_chain[n]] = 
	    (storage_free_block*)((char*)bp+size);
    } else if (aligned_size > page_size && cid != object_header::page_map_cid){
	if (hdr->page_map == NULL) { 
	    // file_size+map_size+map_header >= page_size*8*map_size
	    size_t map_size = 
		(hdr->file_size+sizeof(object_header)+page_size*8-2)
		                   / (page_size*8 - 1);
	    map_size = ALIGN(map_size + sizeof(object_header), page_size)
		       * init_page_map_size - sizeof(object_header); 
	    TRACE_MSG(("storage::alloc: allocate page map, size=%ld\n", 
		       map_size));
	    hdr->page_map = (storage_page_map*)
		((char*)alloc_page(object_header::page_map_cid, map_size)
		 - sizeof(object_header));
	    memset(hdr->page_map->bits, 0, map_size);
	}
	size_t page_no = size_t((char*)bp - (char*)hdr) / page_size; 
	size_t n_pages = aligned_size / page_size - 1;
	do { 
	    page_no += 1;
	    hdr->page_map->bits[page_no >> 3] |= 1 << (page_no & 7);
	} while (--n_pages != 0);
    } 
    return (object_header*)bp+1;
}



void storage::free_page(object_header* hp)
{
    storage_free_block *bp, *prev;
    storage_free_block *fp = (storage_free_block*)hp;

    if (curr_free_page != NULL && curr_free_page < fp) { 
	prev = curr_free_page;
	bp = prev->next;
    } else { 
	prev = NULL;
	bp = hdr->free_page_chain; 
    }

    while (bp != NULL && bp < fp) { 
	 prev = bp;
	 bp = bp->next;
    }
    assert(bp != fp/*object can't be deallocated twice*/);

    size_t aligned_size = ALIGN(fp->size + sizeof(object_header), page_size);
    fp->size = aligned_size - sizeof(object_header);
    if (aligned_size > page_size 
	&& fp->cid != object_header::free_object // reconstruction of free list
	&& fp->cid != object_header::page_map_cid) // allocation of page map
    {  
	assert(hdr->page_map != NULL);
	size_t page_no = size_t((char*)fp - (char*)hdr) / page_size; 
	size_t n_pages = aligned_size / page_size - 1;
	do { 
	    page_no += 1;
	    hdr->page_map->bits[page_no >> 3] &= ~(1 << (page_no & 7));
	} while (--n_pages != 0);
    }
    fp->cid = object_header::free_object;
    fp->next = bp;

    if (prev == NULL) { 
	hdr->free_page_chain = fp;
    } else { 
	if ((char*)prev + prev->size + sizeof(object_header) == (char*)fp) {
	    prev->size += fp->size + sizeof(object_header);
	    fp = prev;
	} else { 
	    prev->next = fp;
	}
    }
    if ((char*)fp + fp->size + sizeof(object_header) == (char*)bp) { 
	fp->size += bp->size + sizeof(object_header);
	fp->next = bp->next;
    }
    curr_free_page = fp;
}

char* storage::get_error_text(char* buf, size_t buf_size)
{
    return data_file.get_error_text(buf, buf_size);
}

storage::~storage() {}


REGISTER(post_raw_object);

END_POST_NAMESPACE

