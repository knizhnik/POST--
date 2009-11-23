//-< TESTNEW.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     16-Nov-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 16-Nov-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example: test for redefined new operator
//-------------------------------------------------------------------*--------*

#include "storage.h"

USE_POST_NAMESPACE

int main() 
{
    storage store("testnew");
    if (store.open(storage::fixed)) { 
	char** text = (char**)store.get_root_object();
	if (text != NULL) {
	    for (char** str = text; *str != NULL; str++) { 
		fputs(*str, stdout);
		delete *str;
	    }
	    delete[] text;
	}
	int n_lines = 0;
	int allocated = 8;
	text = new char*[allocated];
	char buf[256];
	while (fgets(buf, sizeof buf, stdin)) { 
	    if (n_lines+1 == allocated) { 
		char** new_text = new char*[allocated *= 2];
		for (int i = 0; i < n_lines; i++) { 
		    new_text[i] = text[i];
		}
		delete[] text;
	        text = new_text;
	    }
	    text[n_lines] = new char[strlen(buf)+1];
	    strcpy(text[n_lines++], buf);
	}
	text[n_lines] = NULL;
	store.set_root_object(text);
	store.flush();
	store.close();
    }
    return 0;
}
