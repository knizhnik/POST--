# -*- makefile -*-
# Makefile for Microsoft Windows with Microsoft Visual C++ 6.0 compiler and STLport library
# 
# Important: to redefine std::string to use post_alloc instead of standard allocator, 
# you should copy _string_fwd.h file to $STLPORT_HOME\strport\stl directory or replace in original
# _string_fwd.h file the following typedef
#     typedef basic_string<char, char_traits<char>, allocator<char> > string;
# with
#     typedef basic_string<char, char_traits<char>, post_alloc<char> > string;
#
CC=cl
# Debug version
CFLAGS = -MT -c -nologo -Zi -W3 -DDEBUG_LEVEL=DEBUG_TRACE

# Optimized veriosion
#CFLAGS = -c -Ox -MTd -G6 -nologo -Zi -W3 -DDEBUG_LEVEL=DEBUG_CHECK

# Optimized veriosion with switched off asserts
#CFLAGS = -c -Ox -G6 -nologo -Zi -W3 -DNDEBUG

LFLAGS=-Zi -MT -nologo 
STDLIBS=
AR=lib
ARFLAGS= 

EXAMPLES=guess.exe testtree.exe testtext.exe testspat.exe testperf.exe testrans.exe dumpmem.exe stltest.exe testmap.exe testnew.exe

HEADERS=stdtp.h file.h storage.h classinfo.h object.h
OBJS=file.obj storage.obj classinfo.obj avltree.obj rtree.obj ttree.obj array.obj hashtab.obj textobj.obj


all: storage.lib postnew.obj $(EXAMPLES)

file.obj: file.cxx $(HEADERS)
	$(CC) $(CFLAGS) file.cxx

storage.obj: storage.cxx $(HEADERS)
	$(CC) $(CFLAGS) storage.cxx

classinfo.obj: classinfo.cxx $(HEADERS) 
	$(CC) $(CFLAGS) classinfo.cxx

avltree.obj: avltree.cxx avltree.h $(HEADERS)
	$(CC) $(CFLAGS) avltree.cxx

rtree.obj: rtree.cxx rtree.h dnmarr.h $(HEADERS)
	$(CC) $(CFLAGS) rtree.cxx

ttree.obj: ttree.cxx ttree.h dnmarr.h $(HEADERS)
	$(CC) $(CFLAGS) ttree.cxx

array.obj: array.cxx array.h $(HEADERS)
	$(CC) $(CFLAGS) array.cxx

hashtab.obj: hashtab.cxx hashtab.h $(HEADERS)
	$(CC) $(CFLAGS) hashtab.cxx

textobj.obj: textobj.cxx textobj.h $(HEADERS)
	$(CC) $(CFLAGS) textobj.cxx

postnew.obj: postnew.cxx $(HEADERS)
	$(CC) $(CFLAGS) postnew.cxx

storage.lib: $(OBJS)
	$(AR) $(ARFLAGS) /OUT:storage.lib $(OBJS)



#
# Examples
#

guess.obj: guess.cxx $(HEADERS)
	$(CC) $(CFLAGS) guess.cxx

testtext.obj: testtext.cxx textobj.h array.h $(HEADERS)
	$(CC) $(CFLAGS) testtext.cxx

testtree.obj: testtree.cxx avltree.h hashtab.h $(HEADERS)
	$(CC) $(CFLAGS) testtree.cxx

testspat.obj: testspat.cxx rtree.h dnmarr.h hashtab.h $(HEADERS)
	$(CC) $(CFLAGS) testspat.cxx

testperf.obj: testperf.cxx ttree.h dnmarr.h $(HEADERS)
	$(CC) $(CFLAGS) testperf.cxx

testrans.obj: testrans.cxx avltree.h $(HEADERS)
	$(CC) $(CFLAGS) testrans.cxx

dumpmem.obj: dumpmem.cxx $(HEADERS)
	$(CC) $(CFLAGS) dumpmem.cxx

stltest.obj: stltest.cxx post_stl.h $(HEADERS)
	$(CC) $(CFLAGS) -I\STLport-4.5.1\stlport -DUSE_STD_ALLOCATORS -DUSE_STLPORT -DREDEFINE_DEFAULT_ALLOCATOR -GX stltest.cxx

testmap.obj: testmap.cxx post_stl.h $(HEADERS)
	$(CC) $(CFLAGS) -I\STLport-4.5.1\stlport -DUSE_STD_ALLOCATORS -DUSE_STLPORT -DREDEFINE_DEFAULT_ALLOCATOR -GX testmap.cxx

testnew.obj: testnew.cxx $(HEADERS)
	$(CC) $(CFLAGS) testnew.cxx

guess.exe: guess.obj storage.lib
	$(CC) $(LFLAGS) guess.obj storage.lib $(STDLIBS)

testtext.exe: testtext.obj storage.lib
	$(CC) $(LFLAGS) testtext.obj storage.lib $(STDLIBS)

testtree.exe: testtree.obj storage.lib
	$(CC) $(LFLAGS) testtree.obj storage.lib $(STDLIBS)

testspat.exe: testspat.obj storage.lib
	$(CC) $(LFLAGS) testspat.obj storage.lib $(STDLIBS)

testperf.exe: testperf.obj storage.lib
	$(CC) $(LFLAGS) testperf.obj storage.lib $(STDLIBS)

testrans.exe: testrans.obj storage.lib
	$(CC) $(LFLAGS) testrans.obj storage.lib $(STDLIBS)

dumpmem.exe: dumpmem.obj storage.lib
	$(CC) $(LFLAGS) dumpmem.obj storage.lib $(STDLIBS)

stltest.exe: stltest.obj storage.lib
	$(CC) $(LFLAGS) stltest.obj storage.lib $(STDLIBS) \STLport-4.5.1\lib\stlport_vc6_static.lib

testmap.exe: testmap.obj storage.lib
	$(CC) $(LFLAGS) testmap.obj storage.lib $(STDLIBS) \STLport-4.5.1\lib\stlport_vc6_static.lib

testnew.exe: testnew.obj postnew.obj
	$(CC) $(LFLAGS) testnew.obj postnew.obj storage.lib $(STDLIBS) 

#
# Service targets
#

cleanobj:
	-del *.obj,*.*~,*~,*.pch,*.pdb,*.ilk,*.exe,*.dsp,*.dsw,*.ncb,*.opt,*.plg

cleandbs: 
	-del *.odb,*.log,*.tmp

clean: cleanobj cleandbs

cleanall: clean
	-del *.lib


zip: cleanall
	cd ..
	-del post.zip 
	zip -r post.zip post

copyzip: zip
	copy post.zip a:

