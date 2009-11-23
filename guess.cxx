//-< GUESS.CXX >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  2-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Sample program: game "Guess an animal"
//-------------------------------------------------------------------*--------*

#include "object.h"

USE_POST_NAMESPACE

#define MAX_STRING_SIZE 256

storage guess_storage("guess.odb");

class tree_node : public object {
  public: 
    tree_node* guess(); 

    static tree_node* what_is_it(tree_node* parent); 
    
    CLASSINFO(tree_node, REF(yes) REF(no));

    static tree_node* create(tree_node* no, char* question, tree_node* yes) { 
	return new (self_class, guess_storage, strlen(question)) 
	           tree_node(no, question, yes);
    }

  protected:
    tree_node* yes;
    tree_node* no;
    char       question[1]; 

    tree_node(tree_node* no, char* question, tree_node* yes) { 
	this->no  = no;
	this->yes = yes;
	strcpy(this->question, question);
    }
};


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

bool positive_answer()
{
    char buf[16];
    return fgets(buf, sizeof buf, stdin) && (*buf == 'y' || *buf == 'Y');
}


tree_node* tree_node::what_is_it(tree_node* parent) 
{
    char animal[MAX_STRING_SIZE], question[MAX_STRING_SIZE];
    input("What is it ?\n", animal, sizeof animal);
    input("What is difference from other ?\n", question, sizeof question);
    return create(parent, question, create(NULL, animal, NULL));
}

tree_node* tree_node::guess()
{  
    printf("May be, %s (y/n) ? ", question);
    if (positive_answer()) { 
	if (yes == NULL) { 
	    printf("It was very simple question for me...\n");
	} else { 
	    tree_node* clarify = yes->guess();
	    if (clarify != NULL) { 
		yes = clarify;
	    }
	}
    } else { 
	if (no == NULL) { 
	    if (yes == NULL) { 
		return what_is_it(this);
	    } else {
		no = what_is_it(NULL);
	    } 
	} else { 
	    tree_node* clarify = no->guess();
	    if (clarify != NULL) { 
		no = clarify;
	    }
	}
    }
    return NULL; 
}

REGISTER(tree_node);


int main() 
{
    if (guess_storage.open()) { 
	tree_node* root = (tree_node*)guess_storage.get_root_object();
	while(true) { 
	    printf("Think of an animal.\nReady (y/n) ? ");
	    if (!positive_answer()) {
		break;
	    }
	    if (root == NULL) { 
		root = tree_node::what_is_it(NULL);
		guess_storage.set_root_object(root);
	    } else { 
		root->guess();
	    }
	}
	printf("End of the game\n");
        guess_storage.flush();
	guess_storage.close();
	return EXIT_SUCCESS;
    } else { 
	printf("Failed to open database\n");
	return EXIT_FAILURE;
    }
}





