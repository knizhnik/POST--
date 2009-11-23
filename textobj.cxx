//-< TEXTOBJ.CXX >---------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     12-Mar-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 12-Mar-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Implementation of text object
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "textobj.h"
#include <ctype.h>

BEGIN_POST_NAMESPACE

typedef unsigned char uchar_t; 

class conversion_table { 
  public:
    uchar_t upper_case[256];
    uchar_t lower_case[256];
    
    conversion_table() { 
	for (int i = 0; i < 256; i++) { 
	    upper_case[i] = toupper(i);
	    lower_case[i] = tolower(i);
	}
    }
};

static conversion_table cnv_table;

#define uppercase(c) cnv_table.upper_case[c]
#define lowercase(c) cnv_table.lower_case[c]

search_ctx::search_ctx(char* patterns[], int n, int andm, int exact)
: pattern(patterns), n_patterns(n), exact_mask(exact), and_mask(andm)
{
    assert(n > 0 && n <= max_patterns);

    int i, j, len;
    int min_len = 1000000000;
    int max_len = 0;
    position = -1;
    
    for (i = 0; i < n; i++) { 
	len = strlen(patterns[i]) - 1;
	pattern_length[i] = len;
	if (len < min_len) { 
	    min_len = len;
	}
	if (len > max_len) {
	    max_len = len;
	}
	if (!(exact & 1)) { 
	    for (j = 0; j <= len; j++) { 
		patterns[i][j] = uppercase(uchar_t(patterns[i][j]));
	    }
	}
	exact >>= 1;
    }
    assert(min_len >= 0);
    diff = max_len - min_len;
    min_length = min_len;

    for (i = 0; i < (int)items(shift); i++) { 
	shift[i] = min_len+1;
    }
    exact = exact_mask;
    if (exact & 1) { 
	for (j = 0; j < min_len; j++) { 
	    shift[uchar_t(patterns[0][j])] = min_len-j;
	}
    } else { 
	for (j = 0; j < min_len; j++) { 
	    shift[lowercase(uchar_t(patterns[0][j]))] = min_len-j;
	    shift[uchar_t(patterns[0][j])] = min_len-j;
	}
    }
    for (i = 1; i < n; i++) { 
	exact >>= 1;
	if (exact & 1) { 
	    for (j = 0; j < min_len; j++) { 
		uchar_t ch = patterns[i][j];
		if (shift[ch] > min_len-j) { 
		    shift[ch] = min_len-j;
		}
	    }
	} else { 	
	    for (j = 0; j < min_len; j++) { 
		uchar_t ch = patterns[i][j];
		if (shift[ch] > min_len-j) { 
		    shift[ch] = min_len-j;
		}
		if (shift[lowercase(ch)] > min_len-j) { 
		    shift[lowercase(ch)] = min_len-j;
		}
	    }
	}
    }
}


bool search_ctx::search(const char* p_text, int text_length)
{
    int i, j, k, l, m, n;
    uchar_t* s = (uchar_t*)p_text + position + 1;
    int* d = shift;
    m = min_length;
    n = text_length - position - 1;

    if (n_patterns == 1) { 
	uchar_t* p = (uchar_t*)pattern[0];
	
	if (exact_mask & 1) { 
	    for (i = m; i < n; i += d[s[i]]) { 
		j = m;
		k = i;
		while (p[j] == s[k]) { 
		    k -= 1;
		    if (--j < 0) { 
			position += k + 2;
			return true;
		    }
		}
	    }
	} else {
	    for (i = m; i < n; i += d[s[i]]) { 
		j = m;
		k = i;
		while (p[j] == uppercase(s[k])) { 
		    k -= 1;
		    if (--j < 0) { 
			position += k + 2;
			return true;
		    }
		}
	    }
	}

    } else { // more than one patterns
	int matched_mask = 0;

	if (exact_mask == 0) { // all comparisons ignore case
	    n -= diff; // avoid extra checking
	    for (i = m; i < n; i += d[s[i]]) { 
		for (j = n_patterns; --j >= 0;) { 
		    uchar_t* p = (uchar_t*)pattern[j];
		    if (p[m] == uppercase(s[i])) {  
			l = pattern_length[j];
			k = i + l - m;
			while (p[l] == uppercase(s[k])) { 
			    k -= 1;
			    if (--l < 0) { 
				matched_mask |= 1 << j;
				if ((and_mask & ~matched_mask) == 0) { 
				    position += k + 2;
				    return true;
				}
			    }
			}
		    }
		}
	    }
	    for (n += diff; i < n; i += d[s[i]]) { 
		for (j = n_patterns; --j >= 0;) { 
		    uchar_t* p = (uchar_t*)pattern[j];
		    if (p[m] == uppercase(s[i])) {  
			l = pattern_length[j];
			k = i + l - m;
			if (k < n) { 
			    while (p[l] == uppercase(s[k])) { 
				k -= 1;
				if (--l < 0) { 
				    matched_mask |= 1 << j;
				    if ((and_mask & ~matched_mask) == 0) { 
					position += k + 2;
					return true;
				    }
				}
			    }
			}
		    }
		}
	    }
	} else {
	    for (i = m; i < n; i += d[s[i]]) { 
		int exact = exact_mask;
		for (j = 0; j < n_patterns; j++, exact >>= 1) { 
		    uchar_t* p = (uchar_t*)pattern[j];
		    if (exact & 1) { 
			if (p[m] == s[i]) {  
			    l = pattern_length[j];
			    k = i + l - m;
			    if (k < n) { 
				while(p[l] == s[k]) { 
				    k -= 1;				    
				    if (--l < 0) { 
					matched_mask |= 1 << j;
					if ((and_mask & ~matched_mask) == 0) { 
					    position += k + 2;
					    return true;
					}
				    }
				}
			    }
			}
		    } else { // case insensitive comparison
			if (p[m] == uppercase(s[i])) {  
			    l = pattern_length[j];
			    k = i + l - m;
			    if (k < n) { 
				while(p[l] == uppercase(s[k])) { 
				    k -= 1;				    
				    if (--l < 0) { 
					matched_mask |= 1 << j;
					if ((and_mask & ~matched_mask) == 0) { 
					    position += k + 2;
					    return true;
					}
				    }
				}
			    }
			}
		    }
		}
	    }	    
	}
    }

    position = -1;
    return false;
}

//-------------------------------------------------------------

text* text::create(storage& store, const char* buf, size_t len)
{
    text* txt = new (self_class, store, len) text;
    memcpy(txt->data, buf, len);
    txt->length = len;
    txt->data[len] = '\0';
    return txt;
}

text* text::create(storage& store, FILE* f, size_t offs, size_t len)
{
    text* txt = new (self_class, store, len) text;
    fseek(f, offs, SEEK_SET);
    size_t rc = fread(txt->data, 1, len, f);
    txt->data[rc] = '\0';
    txt->length = rc;
    return txt;
}

REGISTER(text);

END_POST_NAMESPACE

