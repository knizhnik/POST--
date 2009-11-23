# -*- makefile -*-
# Makefile for Unix with g++ compiler

#WITH_JUDY=1

CC=g++
# if you use egcs-2.90.* version of GCC please add option -fno-exceptions 
# to reduce code size and increase performance

# Debug version
#CFLAGS = -c -Wall -O0 -g -DDEBUG_LEVEL=DEBUG_TRACE

# Optimized version
CFLAGS = -c -Wall -O6 -g -DDEBUG_LEVEL=DEBUG_CHECK

# Optimized version with switched off asserts
#CFLAGS = -c -Wall -O6 -g -DNDEBUG

LFLAGS=-g
AR=ar
ARFLAGS=-cru 

EXAMPLES=guess testtree testtext testspat testperf testiter dumpmem testnew 
STL_EXAMPLES=testmap stltest

INSTALL_LIB_PATH=/usr/local/post/lib
INSTALL_INC_PATH=/usr/local/post/inc

HEADERS=stdtp.h file.h storage.h classinfo.h object.h
OBJS=file.o storage.o classinfo.o avltree.o rtree.o ttree.o array.o hashtab.o textobj.o post_c_api.o iterator.o

ifdef WITH_JUDY
OBJS += judyarr.o
STDLIBS += Judy/src/obj/.libs/libJudy.a
EXAMPLES += testjudy
endif

all: libstorage.a postnew.o $(EXAMPLES)

judyarr.o: judyarr.cxx judyarr.h
	$(CC) $(CFLAGS) -IJudy/src judyarr.cxx

file.o: file.cxx $(HEADERS)
	$(CC) $(CFLAGS) file.cxx

storage.o: storage.cxx $(HEADERS)
	$(CC) $(CFLAGS) storage.cxx

classinfo.o: classinfo.cxx $(HEADERS)
	$(CC) $(CFLAGS) classinfo.cxx

avltree.o: avltree.cxx avltree.h $(HEADERS)
	$(CC) $(CFLAGS) avltree.cxx

rtree.o: rtree.cxx rtree.h dnmarr.h $(HEADERS)
	$(CC) $(CFLAGS) rtree.cxx

ttree.o: ttree.cxx ttree.h dnmarr.h $(HEADERS)
	$(CC) $(CFLAGS) ttree.cxx

array.o: array.cxx array.h $(HEADERS)
	$(CC) $(CFLAGS) array.cxx

hashtab.o: hashtab.cxx hashtab.h $(HEADERS)
	$(CC) $(CFLAGS) hashtab.cxx

iterator.o: iterator.cxx iterator.h $(HEADERS)
	$(CC) $(CFLAGS) iterator.cxx

textobj.o: textobj.cxx textobj.h $(HEADERS)
	$(CC) $(CFLAGS) textobj.cxx

postnew.o: postnew.cxx $(HEADERS)
	$(CC) $(CFLAGS) postnew.cxx

post_c_api.o: post_c_api.cxx post_c_api.h $(HEADERS)
	$(CC) $(CFLAGS) post_c_api.cxx

libstorage.a: $(OBJS)
	$(AR) $(ARFLAGS) libstorage.a $(OBJS)

install: libstorage.a
	mkdir -p $(INSTALL_LIB_PATH)
	cp libstorage.a $(INSTALL_LIB_PATH)
	mkdir -p $(INSTALL_INC_PATH)
	cp *.h $(INSTALL_INC_PATH)
#
# Examples
#

guess.o: guess.cxx $(HEADERS)
	$(CC) $(CFLAGS) guess.cxx

testtext.o: testtext.cxx textobj.h array.h $(HEADERS)
	$(CC) $(CFLAGS) testtext.cxx

testtree.o: testtree.cxx avltree.h hashtab.h $(HEADERS)
	$(CC) $(CFLAGS) testtree.cxx

testspat.o: testspat.cxx rtree.h dnmarr.h hashtab.h $(HEADERS)
	$(CC) $(CFLAGS) testspat.cxx

testperf.o: testperf.cxx ttree.h dnmarr.h $(HEADERS)
	$(CC) $(CFLAGS) testperf.cxx

testiter.o: testiter.cxx iterator.h ttree.h dnmarr.h $(HEADERS)
	$(CC) $(CFLAGS) testiter.cxx

testrans.o: testrans.cxx avltree.h $(HEADERS)
	$(CC) $(CFLAGS) testrans.cxx

dumpmem.o: dumpmem.cxx $(HEADERS)
	$(CC) $(CFLAGS) dumpmem.cxx

testnew.o: testnew.cxx $(HEADERS)
	$(CC) $(CFLAGS) testnew.cxx

testjudy.o: testjudy.cxx judyarr.h $(HEADERS)
	$(CC) $(CFLAGS) testjudy.cxx

guess: guess.o comptime.cxx libstorage.a
	$(CC) $(LFLAGS) -o guess guess.o comptime.cxx libstorage.a $(STDLIBS)

testtext: testtext.o comptime.cxx libstorage.a
	$(CC) $(LFLAGS) -o testtext testtext.o comptime.cxx libstorage.a $(STDLIBS)

testtree: testtree.o comptime.cxx libstorage.a
	$(CC) $(LFLAGS) -o testtree testtree.o comptime.cxx libstorage.a $(STDLIBS)

testspat: testspat.o comptime.cxx libstorage.a
	$(CC) $(LFLAGS) -o testspat testspat.o comptime.cxx libstorage.a $(STDLIBS)

testperf: testperf.o libstorage.a
	$(CC) $(LFLAGS) -o testperf testperf.o libstorage.a $(STDLIBS)

testiter: testiter.o libstorage.a
	$(CC) $(LFLAGS) -o testiter testiter.o libstorage.a $(STDLIBS)

testrans: testrans.o comptime.cxx libstorage.a
	$(CC) $(LFLAGS) -o testrans testrans.o comptime.cxx libstorage.a $(STDLIBS)

dumpmem: dumpmem.o libstorage.a
	$(CC) $(LFLAGS) -o dumpmem dumpmem.o libstorage.a $(STDLIBS)

testnew: testnew.o postnew.o
	$(CC) $(LFLAGS) -o testnew testnew.o postnew.o libstorage.a $(STDLIBS)

testjudy: testjudy.o libstorage.a
	$(CC) $(LFLAGS) -o testjudy -Xlinker --allow-multiple-definition testjudy.o libstorage.a $(STDLIBS)


#
# Service targets
#

cleanobj:
	rm -f *.o core *~ $(EXAMPLES) $(STL_EXAMPLES) testrans

cleandbs: 
	rm -f *.odb *.log *.tmp

clean: cleanobj cleandbs

cleanall: clean
	rm -f *.a


tgz:	cleanall
	chmod +x GiST/tests/runtests.sh
	cd ..; tar cvzf post.tgz post

copytgz: tgz
	mcopy -o ../post.tgz a:


STL_INCLUDES=/usr/local/include

stltest.o: stltest.cxx post_stl.h $(HEADERS)
	$(CC) $(CFLAGS) -I$(STL_INCLUDES) -DREDEFINE_DEFAULT_ALLOCATOR -DNO_NAMESPACES -DUSE_STD_ALLOCATORS -DREDEFINE_STRING stltest.cxx

stltest: stltest.o libstorage.a 
	$(CC) $(LFLAGS) -o stltest stltest.o libstorage.a 

testmap.o: testmap.cxx post_stl.h $(HEADERS)
	$(CC) $(CFLAGS) -I$(STL_INCLUDES) -DREDEFINE_DEFAULT_ALLOCATOR -DNO_NAMESPACES -DUSE_STD_ALLOCATORS -DREDEFINE_STRING testmap.cxx

testmap: testmap.o libstorage.a 
	$(CC) $(LFLAGS) -o testmap testmap.o libstorage.a 

