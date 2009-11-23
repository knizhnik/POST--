//-< ARRAY.H >-------------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     17-Mar-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 17-Mar-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Arrays: example of POST++ class definition
//-------------------------------------------------------------------*--------*

// P. Shaffer version

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "object.h"

BEGIN_POST_NAMESPACE


//--------------------------------------------------------------------------//
template<class T> 
class array_of_scalar : public object 
{
  public:
    size_t get_size() const { return size; }

    T& operator[](size_t i) { 
        assert(i < size);
        return buf[i];
    }
    T get_at(size_t i) const { 
        assert(i < size);
        return buf[i];
    }
    void put_at(size_t i, T val) { 
        assert(i < size);
        buf[i] = val;
    }
    
    static array_of_scalar* create(storage& store, size_t size) 
    { 
        return new (self_class,store,(size-1)*sizeof(T)) array_of_scalar(size);
    }

    array_of_scalar* reallocate(size_t new_size) 
    { 
        array_of_scalar* a = create(*get_storage(), new_size);
        memcpy(a->buf, buf, sizeof(T)*(new_size > size ? size : new_size));
        delete this;
        return a;
    }

    T* body() { return buf; }

    CLASSINFO(array_of_scalar, NO_REFS);
  protected:
    size_t size;
    T      buf[1];

    array_of_scalar(size_t len) : size(len) {}
};

//--------------------------------------------------------------------------//
typedef array_of_scalar<char>           ArrayOfChar;
typedef array_of_scalar<unsigned char>  ArrayOfByte;
typedef array_of_scalar<unsigned short> ArrayOfWord;
typedef array_of_scalar<int>            ArrayOfInt;
typedef array_of_scalar<unsigned>       ArrayOfNat;
typedef array_of_scalar<float>          ArrayOfFloat;
typedef array_of_scalar<double>         ArrayOfDouble;

//--------------------------------------------------------------------------//
template<class T> class array_of_ptr : public object 
{
  public:
    size_t get_size() const { return size; }

    T*& operator[](size_t i) 
    { 
        assert(i < size);
        return buf[i];
    }
    
    T* get_at(size_t i) const 
    { 
        assert(i < size);
        return buf[i];
    }
    
    void put_at(size_t i, T* val) 
    { 
        assert(i < size);
        buf[i] = val;
    }
    
    static array_of_ptr* create(storage& store, size_t size) 
    { 
        return new (self_class, store, (size-1)*sizeof(T*)) array_of_ptr(size);
    }

    array_of_ptr* reallocate(size_t new_size) 
    { 
        array_of_ptr* a = create(*get_storage(), new_size);
        memcpy(a->buf, buf, sizeof(T*)*(new_size > size ? size : new_size));
        delete this;
        return a;
    }

    T** body() { return buf; }

    CLASSINFO(array_of_ptr, VREFS(buf));
  protected:
    size_t size;
    T*     buf[1];

    array_of_ptr(size_t len) : size(len) {}
};

//--------------------------------------------------------------------------//
typedef array_of_ptr<object> ArrayOfObject;

//--------------------------------------------------------------------------//
class POST_DLL_ENTRY String : public ArrayOfChar 
{ 
public: 
    size_t get_length() const { return size-1; }
    
    int  compare(const char* str) const { return strcmp(buf, str); }

    int  compare(String const& str) const { return -str.compare(buf); }

    int  index(const char* str) const 
    { 
        char* p = strstr((char*)buf, (char*)str); 
        return p ? p - buf : -1; 
    }
    
    bool operator == (const char* ptr) const 
    { 
        return compare(ptr) == 0; 
    }
    
    bool operator != (const char* ptr) const 
    { 
        return compare(ptr) != 0; 
    }

    static String* create(storage& store, size_t size) 
    { 
        return new (self_class, store, size-1) String(size);
    }

    static String* create(storage& store, const char* str) 
    { 
        size_t len = strlen(str);
        String* s = new (self_class, store, len) String(len+1);
        memcpy(s->buf, str, len+1);
        return s;
    }

    CLASSINFO(String, NO_REFS);
  protected:
    String(size_t len) : ArrayOfChar(len) {}
};
    
//--------------------------------------------------------------------------//
template<class T> class matrix : public object 
{
  public:
    size_t get_number_of_rows() const { return rows; }
    size_t get_number_of_colons() const { return colons; }

    T& operator()(size_t i, size_t j) { 
        assert(i < rows && j < colons);
        return buf[i*colons + j];
    }
    T get_at(size_t i, size_t j) const { 
        assert(i < rows && j < colons);
        return buf[i*colons + j];
    }
    void put_at(size_t i, size_t j, T val) { 
        assert(i < rows && j < colons);
        buf[i*colons + j] = val;
    }
    
    static matrix* create(storage& store, size_t rows, size_t colons)
    { 
    return new (self_class, store, (rows*colons-1)*sizeof(T)) 
        matrix(rows, colons);
    }

    CLASSINFO(matrix, NO_REFS);
  protected:
    size_t rows, colons;
    T      buf[1];

    matrix(size_t n_rows, size_t n_colons) : rows(n_rows), colons(n_colons) {}
};

//--------------------------------------------------------------------------//
typedef matrix<char>           MatrixOfChar;
typedef matrix<unsigned char>  MatrixOfByte;
typedef matrix<unsigned short> MatrixOfWord;
typedef matrix<int>            MatrixOfInt;
typedef matrix<unsigned>       MatrixOfNat;
typedef matrix<float>          MatrixOfFloat;
typedef matrix<double>         MatrixOfDouble;


//--------------------------------------------------------------------------//
template<class T> class dyn_array_of_scalar : public object 
{
public:
    size_t get_size() const { return used; }

    T& operator[](size_t i) 
    { 
        assert(i < used);
        return (*arr)[i];
    }
    T get_at(size_t i) const 
    { 
        assert(i < used);
        return arr->get_at(i);
    }
    void put_at(size_t i, T val) 
    { 
        assert(i < used);
        arr->put_at(i, val);
    }
     
    void set_size(size_t new_size) 
    { 
        if (new_size > allocated_size) 
        { 
            allocated_size = (new_size < allocated_size*2) 
		? allocated_size*2 : new_size;
            arr = arr->reallocate(allocated_size);
        }
        used = new_size;
    }

    void push(T val) 
    { 
        if (used == allocated_size) 
        { 
            allocated_size = (allocated_size+1)*2;
            arr = arr->reallocate(allocated_size);
        }
        arr->put_at(used++, val);
    }

    void ins(size_t pos, size_t count, T val) 
    { 
        size_t size = used;
        assert(pos <= size);
        set_size(size + count);
        T* p = arr->body();
        size_t dst, src;
        for (dst = size+count, src = size; src > pos; p[--dst] = p[--src]);
        while (dst > pos) p[--dst] = val;
    }

    void del(size_t pos, size_t count) 
    { 
        size_t size = used;
        assert(pos + count <= size);
        size_t dst, src;
        T* p = arr->body();
        for (dst = pos, src = dst+count; src < size; p[dst++] = p[src++]);
        used -= count;
    }
    
    T pop() { 
        assert(used != 0);
        return arr->get_at(--used);
    }

    T* body() { return arr->body(); }

    dyn_array_of_scalar(size_t init_allocated_len, size_t init_used_len = 0)
    { 
        used = init_used_len;
        allocated_size = init_used_len > init_allocated_len 
	    ? init_used_len : init_allocated_len;
        arr = array_of_scalar<T>::create(*get_storage(), allocated_size);
    }
    ~dyn_array_of_scalar() { delete arr; }

    CLASSINFO(dyn_array_of_scalar, REF(arr));

  protected:
    size_t allocated_size;
    size_t used;
    array_of_scalar<T>* arr;
};

//--------------------------------------------------------------------------//
template<class T> class dyn_array_of_ptr : public object 
{
public:
    size_t get_size() const 
    { 
        return used; 
    }

    T*& operator[](size_t i) 
    { 
        assert(i < used);
        return (*arr)[i];
    }
    T* get_at(size_t i) const 
    { 
        assert(i < used);
        return arr->get_at(i);
    }
    void put_at(size_t i, T* val) 
    { 
        assert(i < used);
        arr->put_at(i, val);
    }
     
    void set_size(size_t new_size) 
    { 
        if (new_size > allocated_size) 
        { 
            allocated_size = (new_size < allocated_size*2) 
		? allocated_size*2 : new_size;
            arr = arr->reallocate(allocated_size);
        } else { 
            T** p = arr->body();
            for (size_t i = used; i > new_size; p[--i] = NULL);
        }
        used = new_size;
    }

    void push(T* val) 
    { 
        if (used == allocated_size) 
        { 
            allocated_size = (allocated_size+1)*2;
            arr = arr->reallocate( allocated_size );
        }
        arr->put_at(used++, val);
    }

    void ins(size_t pos, size_t count, T* val) 
    { 
        size_t size = used;
        assert(pos <= size);
        set_size(size + count);
        T** p = arr->body();
        size_t dst, src;
        for (dst = size+count, src = size; src > pos; p[--dst] = p[--src]);
        while (dst > pos) p[--dst] = val;
    }

    void del(size_t pos, size_t count) 
    { 
        size_t size = used;
        assert(pos + count <= size);
        size_t dst, src;
        T** p = arr->body();
        for (dst = pos, src = dst+count; src < size; p[dst++] = p[src++]);
        used -= count;
        while (count != 0) { 
            p[dst++] = NULL;
            count -= 1;
        }
    }
    
    T* pop() 
    { 
        assert(used != 0);
        T* top = arr->get_at(--used);
        arr->put_at(used, NULL);
        return top;
    }

    T** body() { return arr->body(); }

    dyn_array_of_ptr(size_t init_allocated_len, size_t init_used_len = 0)
    { 
        used = init_used_len;
        allocated_size = init_used_len > init_allocated_len 
	    ? init_used_len : init_allocated_len;
        arr = array_of_ptr<T>::create(*get_storage(), allocated_size);
    }
    
    ~dyn_array_of_ptr() { delete arr; }

    CLASSINFO(dyn_array_of_ptr, REF(arr));

protected:
    size_t allocated_size;
    size_t used;
    array_of_ptr<T>* arr;
};

//--------------------------------------------------------------------------//
typedef POST_DLL_ENTRY dyn_array_of_scalar<char>           DynArrayOfChar;
typedef dyn_array_of_scalar<unsigned char>  DynArrayOfByte;
typedef dyn_array_of_scalar<unsigned short> DynArrayOfWord;
typedef dyn_array_of_scalar<int>            DynArrayOfInt;
typedef dyn_array_of_scalar<unsigned>       DynArrayOfNat;
typedef dyn_array_of_scalar<float>          DynArrayOfFloat;
typedef dyn_array_of_scalar<double>         DynArrayOfDouble;
typedef dyn_array_of_ptr<object>            DynArrayOfObject;

//------------------------------------------------------------------------//
class POST_DLL_ENTRY DynString : public DynArrayOfChar 
{ 
public: 
    size_t get_length() const 
    { 
	return used-1; 
    }
    
    int  compare(const char* str) const 
    { 
	return strcmp(arr->body(), str); 
    }

    int  compare(DynString const& str) const 
    { 
	return -str.compare(arr->body()); 
    }

    int  index(const char* str) const 
    { 
        char* p = strstr((char*)arr->body(), (char*)str); 
        return p ? p - arr->body() : -1; 
    }
    
    bool operator == (const char* ptr) const 
    { 
        return compare(ptr) == 0; 
    }
    
    bool operator != (const char* ptr) const 
    { 
        return compare(ptr) != 0; 
    }

    void operator = (const char* str)
    {
        assert(this);
        size_t len = strlen(str);
        set_size(len+1);
        memcpy(arr->body(), str, len+1);
    }
    
    static DynString* create(storage& store, size_t size) 
    { 
        return new (self_class, store, size-1) DynString(size);
    }

    static DynString* create(storage& store, const char* str) 
    { 
        size_t len = strlen(str);
        DynString* s = new (self_class, store) DynString(len+1);
        memcpy(s->arr->body(), str, len+1);
        return s;
    }

    CLASSINFO(DynString, NO_REFS);
  protected:
    DynString(size_t len) : DynArrayOfChar(len) {}
};

END_POST_NAMESPACE

#endif
