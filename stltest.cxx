//-< STLTEST.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     31-May-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 31-May-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example of using STL classes with POST++
//-------------------------------------------------------------------*--------*

#include "post_stl.h"
#include "iostream"
#include "vector"
#include "string"

#ifndef NO_NAMESPACES
using namespace std;
#endif

USE_POST_NAMESPACE

typedef vector<string> text;


int main() 
{
    storage sto("teststl.odb");
    if (sto.open(storage::fixed)) { 
	text* root  = (text*)sto.get_root_object();
	if (root != NULL) { 
	    for (text::iterator i = root->begin(); 
		 i != root->end(); i++)
	    {
		cout << *i << '\n';
	    }
	} else { 
	    root = new (sto) text;
	    sto.set_root_object((object*)root);
	}
	cout << "\nAdd some lines. Terminate input with empty line.\n";
	while (true) { 
	    char buf[256];
	    cin.getline(buf, sizeof buf);
	    if (*buf == '\0') { 
		cout << "End of session\n";
		sto.flush();
		sto.close();
		return EXIT_SUCCESS;
	    }
	    root->push_back(*new (sto) string(buf));
	}
    } else { 
	cerr << "Failed to open storage\n";
	return EXIT_FAILURE;
    }
}

