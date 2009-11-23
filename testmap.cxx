//-< TESTMAP.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     31-May-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 31-May-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Example of using STL map class with POST++
//-------------------------------------------------------------------*--------*

#include "post_stl.h"
#include <iostream>
#include <string>
#include <map>

#ifndef NO_NAMESPACES
using namespace std;
#endif
USE_POST_NAMESPACE

typedef map<string, string, less<string> > barmap;

int main()
{
    storage sto("bar.odb");
    if (sto.open(storage::fixed)) { 
        barmap* root  = (barmap*)sto.get_root_object();
	//
	// Microsoft implementation of STL map class uses _Tree template
	// which contains static _Nil component. This component is initialized 
	// by _Init() function when constructor of this class is called first 
	// time. To preserve the same address for this _Nil objects, 
	// programmer should create instances of all used map classes 
	// within begin_static_data, end_static_data section. In this case
	// post_alloc will return the same addresses for _Nil objects.
	//
	sto.begin_static_data();
	barmap* dummy = new (sto) barmap; 
	sto.end_static_data();
        if (root != NULL) { 
            for (barmap::iterator i = root->begin(); i != root->end(); i++) {
                cout << "Key=" << (*i).first 
		     << ", value=" << (*i).second << '\n';
            }
        } else { 
            root = new (sto) barmap;
            sto.set_root_object((object*)root);
	}
	cout << "Add some pairs. Type '.' to exit. Type '*' to clear the content.\n";
	while (true) { 
	    string key;
	    string value;
	    cout << "Key: ";
	    cin >> key;
	    if (key == ".") { 
		break;
	    }

	    //
	    //
	    //
	    if (key == "*") {

	      root->clear ();
	      continue;
	    }
	    //
	    //
	    //

	    cout << "Value: ";
	    cin >> value;
	    cout << "Key = " << key << ", Value = " << value << '\n';
#if 1
	    root->insert(barmap::value_type(key, value));
#else
	    root->insert(barmap::value_type(*new (sto) string(key), 
					    *new (sto) string(value)));
#endif
	}
	sto.flush();
	sto.close();
    }
    return 0;
}

