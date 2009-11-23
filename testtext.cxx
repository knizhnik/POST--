//-< TESTTEXT.CXX >--------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     12-Mar-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 12-Mar-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example: test for text object 
//-------------------------------------------------------------------*--------*

#include "textobj.h"
#include "array.h"

USE_POST_NAMESPACE

class text_file : public object {
    friend class directory;
  protected:
    text*      txt;
    text_file* next;
    String*    name;

  public:
    bool operator == (const char* file_name) const { 
	return *name == file_name;
    }
    bool operator != (const char* file_name) const { 
	return *name != file_name;
    }
    int size() const {
	return txt ? txt->length : 0;
    }
    void search(search_ctx& ctx) const;

    text_file(const char* file_name, text_file* chain);

    CLASSINFO(text_file, REF(txt) REF(next) REF(name));
};

REGISTER(text_file);

class directory : public object { 
  protected:
    text_file* file_list;
    
  public:
    void list() const;
    void add(const char* file_name);
    void remove(const char* file_name);
    void search(const char* pattern) const;

    CLASSINFO(directory, REF(file_list));
};

REGISTER(directory);

const int max_name_len = 256;


void text_file::search(search_ctx& ctx) const
{
    if (txt != NULL && txt->search(ctx))  {
	printf("*** In file '%s':\n", name->body());
	if (((ctx.and_mask-1) & ctx.and_mask) == 0) {
	    // Search set position in text when no more than 1 
	    // patterns are specified with '+' quilifier.
	    char* p = txt->data;
	    do { 
		int beg = ctx.position, end = beg, eof = txt->length;
		while (beg > 0 && p[beg-1] != '\n') { 
		    beg -= 1;
		}
		while (end < eof && p[end] != '\n') { 
		    end += 1;
		}
		printf("%.*s\n", end - beg, p + beg);
	    } while (txt->search(ctx));
	}
    }
}

text_file::text_file(const char* file_name, text_file* chain)
{
    FILE* f = fopen(file_name, "r");
    txt = NULL;
    next = chain;
    name = String::create(*get_storage(), file_name);
    if (f != NULL) { 
	fseek(f, 0, SEEK_END);
	txt = text::create(*get_storage(), f, 0, ftell(f)); 
	fclose(f);
    }
}

void directory::add(const char* file_name)
{
    file_list = new_in (*get_storage(),text_file) (file_name, file_list);
}

void directory::remove(const char* file_name)
{
    text_file *fp, **fpp;
    for (fpp = &file_list; 
	 (fp = *fpp) != NULL && *fp != file_name; 
	 fpp = &fp->next);

    if (fp != NULL) { 
	*fpp = fp->next;
	delete fp;
    } else { 
	printf("No such entry '%s'\n", file_name);
    }
}

void directory::list() const 
{
    int n = 0;
    for (text_file* fp = file_list; fp != NULL; fp = fp->next) { 
	printf("%16s\t%d\n", fp->name->body(), fp->size());
	n += 1;
    }
    printf("\nTotal %d files\n", n);
} 

void directory::search(const char* patterns) const
{
    const int max_patterns = 8;
    char  buf[max_patterns][max_name_len];
    char* bp[max_patterns];    
    int   n;
    int   and_mask = 0;
    int   pos;
    for (n = 0; 
	 n < max_patterns && sscanf(patterns, "%s%n", buf[n], &pos) == 1; 
	 n += 1, patterns += pos)
    {
	bp[n] = buf[n];
	if (buf[n][0] == '+') { 
	    and_mask |= 1 << n;
	    bp[n] += 1;
	}
	if (*bp[n] == '\0') { 
	    printf("Empty pattern\n");
	    n -= 1;
	}
    }
    if (n > 0) { 
	search_ctx ctx(bp, n, and_mask, 0);
	for (text_file* fp = file_list; fp != NULL; fp = fp->next) { 
	    ctx.reset();
	    fp->search(ctx);
	}
    }
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

int dbs_session() 
{
    storage text_storage("text");
    char buf[max_name_len];

    input("Login (administrator or user):", buf, sizeof buf);
    if (strcmp(buf, "administrator") == 0) { 
	if (!text_storage.open(storage::use_transaction_log)) { 
	    text_storage.get_error_text(buf, sizeof buf);
	    printf("Failed to open storage for administration: %s\n", buf);
	    return EXIT_FAILURE;
	} 
	directory* dir = (directory*)text_storage.get_root_object();
	if (dir == NULL) { 
	    dir = new_in(text_storage,directory);
	    text_storage.set_root_object(dir);
	}
	while (true) { 
	    input("Commands (add, list, remove, exit):", buf, sizeof buf);
	    if (strcmp(buf, "add") == 0) { 
		input("File name to add:", buf, sizeof buf);
		dir->add(buf);
	    } else if (strcmp(buf, "remove") == 0) { 
		input("File name to remove:", buf, sizeof buf);
		dir->remove(buf);
	    } else if (strcmp(buf, "list") == 0) { 
		dir->list();
	    } else if (strcmp(buf, "exit") == 0) { 
		break;
	    }
	}
    } else { // user login
	if (!text_storage.open(storage::read_only)) { 
	    printf("Failed to open storage\n");
	    return EXIT_FAILURE;
	} 
	directory* dir = (directory*)text_storage.get_root_object();
	if (dir == NULL) { 
	    printf("Storage is empty\n");
	    return EXIT_FAILURE;
	}
	while (true) { 
	    input("Search for (list of WORD or +WORD, period for exit):", 
		  buf, sizeof buf);
	    if (strcmp(buf, ".") != 0) {
		dir->search(buf);
	    } else { 
		break;
	    }
	}
    } 	
    text_storage.close();
    printf("End of session\n");
    return EXIT_SUCCESS;
}    


int main() 
{
    SEN_TRY { 
	return dbs_session();
    } SEN_ACCESS_VIOLATION_HANDLER(); 
    return EXIT_FAILURE;
}
 




