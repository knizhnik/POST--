//-< DNMARR.H >------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Transient (non-persistent) dynamc arra implementation
//-------------------------------------------------------------------*--------*

#ifndef __DNMARR_H__
#define __DNMARR_H__

BEGIN_POST_NAMESPACE

template<class T>
class POST_DLL_ENTRY dnm_array { 
  protected:
    T*     buf;
    size_t buf_size;
    size_t bp;
  public:

    size_t get_size() const { return bp; }

    void set_size(size_t new_size) { 
	bp = new_size;
	if (new_size > buf_size) {
	    if (new_size < buf_size*2) {
		new_size = buf_size*2;
	    }
	    T* new_buf = new T[new_size];
	    memcpy(new_buf, buf, buf_size*sizeof(T));
	    delete[] buf;
	    buf = new_buf;
	    buf_size = new_size;
	}
    }    

    void push(T obj) {
	int top = bp;
	set_size(top+1);
	buf[top] = obj;
    }

    bool is_empty() const { return bp == 0; }

    T pop() { 
	assert(bp != 0);
	return buf[--bp];
    }

    T& operator [](size_t index) { 
	assert(index < bp);
	return buf[index];
    }

    dnm_array(size_t init_size = 256) { 
	bp = 0;
	buf_size = init_size;
	buf = new T[init_size];
    }

    ~dnm_array() { 
	delete[] buf;
    }
};	

typedef dnm_array<object*> search_buffer;

END_POST_NAMESPACE

#endif

