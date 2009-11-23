#         Makefile for GiST test program
#
# Generalized Search Tree

PROGRAM = BTree

SRCS = command.cpp gram.y scan.l BTentry.cpp BTnode.cpp BTpredicate.cpp 
       
OBJS = command.o gram.tab.o scan.o BTentry.o BTnode.o BTpredicate.o 

DEST = .

$(PROGRAM): ../libGiST/libGiST.a $(OBJS) .depend
	$(LINKER) -o $(PROGRAM) $(LDFLAGS) $(OBJS) $(LIBS)

.depend:
	$(MAKE) depend
	touch .depend

gram.tab.cpp: gram.y
	$(YACC) $(YACCFLAGS) gram.y
	mv gram.tab.c gram.tab.cpp

scan.cpp: scan.l
	$(LEX) $(LEXFLAGS) scan.l >scan.cpp

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCS) $*.cpp

clean:
	rm -f $(OBJS) $(PROGRAM) gram.tab.cpp scan.cpp gram.tab.h gram.output *~ #*# *pure* TAGS

depend:
	makedepend $(DEFINES) $(INCS) $(SRCS)

install: $(PROGRAM)
	echo Installing $(PROGRAM) in $(DEST)
	install -s $(PROGRAM) $(DEST)

# DO NOT DELETE

command.o: /usr/include/stdio.h /usr/include/features.h
command.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
command.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
command.o: /usr/include/bits/types.h /usr/include/libio.h
command.o: /usr/include/_G_config.h /usr/include/wchar.h
command.o: /usr/include/bits/wchar.h /usr/include/gconv.h
command.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
command.o: /usr/include/bits/stdio_lim.h /usr/include/g++-3/fstream.h
command.o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
command.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
command.o: /usr/include/bits/confname.h BT.h ../libGiST/GiST.h
command.o: ../libGiST/GiSTdefs.h /usr/include/assert.h /usr/include/stdlib.h
command.o: /usr/include/math.h /usr/include/bits/huge_val.h
command.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
command.o: ../libGiST/GiSTentry.h /usr/include/string.h
command.o: ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
command.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h ../libGiST/GiSTstore.h
command.o: ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h ../../ttree.h
command.o: ../../object.h ../../classinfo.h ../../storage.h ../../file.h
command.o: ../../stdtp.h /usr/include/limits.h
command.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
command.o: ../../dnmarr.h BTentry.h BTnode.h BTpredicate.h command.h
gram.o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
gram.o: /usr/include/libio.h /usr/include/_G_config.h
gram.o: /usr/include/bits/types.h /usr/include/features.h
gram.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
gram.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
gram.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
gram.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h BT.h
gram.o: ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
gram.o: /usr/include/stdlib.h /usr/include/math.h
gram.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
gram.o: /usr/include/bits/mathcalls.h ../libGiST/GiSTentry.h
gram.o: /usr/include/string.h ../libGiST/GiSTpredicate.h
gram.o: ../libGiST/GiSTlist.h ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h
gram.o: ../libGiST/GiSTstore.h ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h
gram.o: ../../ttree.h ../../object.h ../../classinfo.h ../../storage.h
gram.o: ../../file.h ../../stdtp.h /usr/include/stdio.h
gram.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
gram.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
gram.o: ../../dnmarr.h BTentry.h BTnode.h BTpredicate.h command.h
scan.o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
scan.o: /usr/include/libio.h /usr/include/_G_config.h
scan.o: /usr/include/bits/types.h /usr/include/features.h
scan.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
scan.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
scan.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
scan.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h BT.h
scan.o: ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
scan.o: /usr/include/stdlib.h /usr/include/math.h
scan.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
scan.o: /usr/include/bits/mathcalls.h ../libGiST/GiSTentry.h
scan.o: /usr/include/string.h ../libGiST/GiSTpredicate.h
scan.o: ../libGiST/GiSTlist.h ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h
scan.o: ../libGiST/GiSTstore.h ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h
scan.o: ../../ttree.h ../../object.h ../../classinfo.h ../../storage.h
scan.o: ../../file.h ../../stdtp.h /usr/include/stdio.h
scan.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
scan.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
scan.o: ../../dnmarr.h BTentry.h BTnode.h BTpredicate.h gram.tab.h
BTentry.o: BT.h ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
BTentry.o: /usr/include/features.h /usr/include/sys/cdefs.h
BTentry.o: /usr/include/gnu/stubs.h /usr/include/stdlib.h
BTentry.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
BTentry.o: /usr/include/math.h /usr/include/bits/huge_val.h
BTentry.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
BTentry.o: ../libGiST/GiSTentry.h /usr/include/string.h
BTentry.o: ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
BTentry.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h ../libGiST/GiSTstore.h
BTentry.o: ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h ../../ttree.h
BTentry.o: ../../object.h ../../classinfo.h ../../storage.h ../../file.h
BTentry.o: ../../stdtp.h /usr/include/stdio.h /usr/include/bits/types.h
BTentry.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
BTentry.o: /usr/include/bits/wchar.h /usr/include/gconv.h
BTentry.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
BTentry.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
BTentry.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
BTentry.o: ../../dnmarr.h BTentry.h /usr/include/g++-3/iostream.h
BTentry.o: /usr/include/g++-3/streambuf.h BTnode.h BTpredicate.h
BTnode.o: /usr/include/string.h /usr/include/features.h
BTnode.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
BTnode.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h BT.h
BTnode.o: ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
BTnode.o: /usr/include/stdlib.h /usr/include/math.h
BTnode.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
BTnode.o: /usr/include/bits/mathcalls.h ../libGiST/GiSTentry.h
BTnode.o: ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
BTnode.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h ../libGiST/GiSTstore.h
BTnode.o: ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h ../../ttree.h
BTnode.o: ../../object.h ../../classinfo.h ../../storage.h ../../file.h
BTnode.o: ../../stdtp.h /usr/include/stdio.h /usr/include/bits/types.h
BTnode.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
BTnode.o: /usr/include/bits/wchar.h /usr/include/gconv.h
BTnode.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
BTnode.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
BTnode.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
BTnode.o: ../../dnmarr.h BTentry.h /usr/include/g++-3/iostream.h
BTnode.o: /usr/include/g++-3/streambuf.h BTnode.h BTpredicate.h
BTpredicate.o: BT.h ../libGiST/GiST.h ../libGiST/GiSTdefs.h
BTpredicate.o: /usr/include/assert.h /usr/include/features.h
BTpredicate.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
BTpredicate.o: /usr/include/stdlib.h
BTpredicate.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
BTpredicate.o: /usr/include/math.h /usr/include/bits/huge_val.h
BTpredicate.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
BTpredicate.o: ../libGiST/GiSTentry.h /usr/include/string.h
BTpredicate.o: ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
BTpredicate.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h
BTpredicate.o: ../libGiST/GiSTstore.h ../libGiST/GiSTcursor.h
BTpredicate.o: ../libGiST/GiSTdb.h ../../ttree.h ../../object.h
BTpredicate.o: ../../classinfo.h ../../storage.h ../../file.h ../../stdtp.h
BTpredicate.o: /usr/include/stdio.h /usr/include/bits/types.h
BTpredicate.o: /usr/include/libio.h /usr/include/_G_config.h
BTpredicate.o: /usr/include/wchar.h /usr/include/bits/wchar.h
BTpredicate.o: /usr/include/gconv.h
BTpredicate.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
BTpredicate.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
BTpredicate.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
BTpredicate.o: ../../dnmarr.h BTentry.h /usr/include/g++-3/iostream.h
BTpredicate.o: /usr/include/g++-3/streambuf.h BTnode.h BTpredicate.h
