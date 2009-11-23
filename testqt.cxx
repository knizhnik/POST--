//-< TESTQT.CXX >----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     16-Nov-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 16-Nov-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example: test for compatibility with Qt library
//-------------------------------------------------------------------*--------*

#include "storage.h"
#include "qarray.h"

USE_POST_NAMESPACE

int main() 
{
    storage store("testqt");
    if (store.open(storage::fixed)) { 
	void* root = store.get_root_object();
	if (root != NULL) {
	    QArray<char*>& text = *(QArray<char*>*)root;
	    for (int i = 0; text[i] != NULL; i++) { 
		fputs(text[i], stdout);
		delete text[i];
	    }
	    delete &text;
	}
	int n_lines = 0;
	int allocated = 8;
	QArray<char*>& text = *new QArray<char*>(allocated);
	char buf[256];
	while (fgets(buf, sizeof buf, stdin)) { 
	    if (n_lines+1 == allocated) { 
		text.resize(allocated *= 2);
	    }
	    text[n_lines] = new char[strlen(buf)+1];
	    strcpy(text[n_lines++], buf);
	}
	text[n_lines] = NULL;
	store.set_root_object(&text);
	store.flush();
	store.close();
    }
    return 0;
}



