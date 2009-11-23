#         Makefile for GiST test program
#
# Generalized Search Tree

PROGRAM = RSTree

SRCS = command.cpp gram.y scan.l RTentry.cpp RTnode.cpp RTpredicate.cpp RT.cpp
       
OBJS = command.o gram.tab.o scan.o RTentry.o RTnode.o RTpredicate.o RT.o

DEST = .

$(PROGRAM): $(OBJS) .depend
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
command.o: /usr/include/bits/confname.h /usr/include/ctype.h
command.o: /usr/include/endian.h /usr/include/bits/endian.h RT.h
command.o: ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
command.o: /usr/include/stdlib.h /usr/include/math.h
command.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
command.o: /usr/include/bits/mathcalls.h ../libGiST/GiSTentry.h
command.o: /usr/include/string.h ../libGiST/GiSTpredicate.h
command.o: ../libGiST/GiSTlist.h ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h
command.o: ../libGiST/GiSTstore.h ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h
command.o: ../../ttree.h ../../object.h ../../classinfo.h ../../storage.h
command.o: ../../file.h ../../stdtp.h /usr/include/limits.h
command.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
command.o: ../../dnmarr.h RTentry.h RTnode.h RTpredicate.h command.h
gram.o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
gram.o: /usr/include/libio.h /usr/include/_G_config.h
gram.o: /usr/include/bits/types.h /usr/include/features.h
gram.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
gram.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
gram.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
gram.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h RT.h
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
gram.o: ../../dnmarr.h RTentry.h RTnode.h RTpredicate.h command.h
scan.o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
scan.o: /usr/include/libio.h /usr/include/_G_config.h
scan.o: /usr/include/bits/types.h /usr/include/features.h
scan.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
scan.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
scan.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
scan.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h RT.h
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
scan.o: ../../dnmarr.h RTentry.h RTnode.h RTpredicate.h gram.tab.h
RTentry.o: RT.h ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
RTentry.o: /usr/include/features.h /usr/include/sys/cdefs.h
RTentry.o: /usr/include/gnu/stubs.h /usr/include/stdlib.h
RTentry.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
RTentry.o: /usr/include/math.h /usr/include/bits/huge_val.h
RTentry.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
RTentry.o: ../libGiST/GiSTentry.h /usr/include/string.h
RTentry.o: ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
RTentry.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h ../libGiST/GiSTstore.h
RTentry.o: ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h ../../ttree.h
RTentry.o: ../../object.h ../../classinfo.h ../../storage.h ../../file.h
RTentry.o: ../../stdtp.h /usr/include/stdio.h /usr/include/bits/types.h
RTentry.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
RTentry.o: /usr/include/bits/wchar.h /usr/include/gconv.h
RTentry.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
RTentry.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
RTentry.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
RTentry.o: ../../dnmarr.h RTentry.h /usr/include/g++-3/iostream.h
RTentry.o: /usr/include/g++-3/streambuf.h RTnode.h RTpredicate.h
RTnode.o: /usr/include/string.h /usr/include/features.h
RTnode.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
RTnode.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h RT.h
RTnode.o: ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
RTnode.o: /usr/include/stdlib.h /usr/include/math.h
RTnode.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
RTnode.o: /usr/include/bits/mathcalls.h ../libGiST/GiSTentry.h
RTnode.o: ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
RTnode.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h ../libGiST/GiSTstore.h
RTnode.o: ../libGiST/GiSTcursor.h ../libGiST/GiSTdb.h ../../ttree.h
RTnode.o: ../../object.h ../../classinfo.h ../../storage.h ../../file.h
RTnode.o: ../../stdtp.h /usr/include/stdio.h /usr/include/bits/types.h
RTnode.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
RTnode.o: /usr/include/bits/wchar.h /usr/include/gconv.h
RTnode.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
RTnode.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
RTnode.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
RTnode.o: ../../dnmarr.h RTentry.h /usr/include/g++-3/iostream.h
RTnode.o: /usr/include/g++-3/streambuf.h RTnode.h RTpredicate.h
RTpredicate.o: RT.h ../libGiST/GiST.h ../libGiST/GiSTdefs.h
RTpredicate.o: /usr/include/assert.h /usr/include/features.h
RTpredicate.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
RTpredicate.o: /usr/include/stdlib.h
RTpredicate.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
RTpredicate.o: /usr/include/math.h /usr/include/bits/huge_val.h
RTpredicate.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
RTpredicate.o: ../libGiST/GiSTentry.h /usr/include/string.h
RTpredicate.o: ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
RTpredicate.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h
RTpredicate.o: ../libGiST/GiSTstore.h ../libGiST/GiSTcursor.h
RTpredicate.o: ../libGiST/GiSTdb.h ../../ttree.h ../../object.h
RTpredicate.o: ../../classinfo.h ../../storage.h ../../file.h ../../stdtp.h
RTpredicate.o: /usr/include/stdio.h /usr/include/bits/types.h
RTpredicate.o: /usr/include/libio.h /usr/include/_G_config.h
RTpredicate.o: /usr/include/wchar.h /usr/include/bits/wchar.h
RTpredicate.o: /usr/include/gconv.h
RTpredicate.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
RTpredicate.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
RTpredicate.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
RTpredicate.o: ../../dnmarr.h RTentry.h /usr/include/g++-3/iostream.h
RTpredicate.o: /usr/include/g++-3/streambuf.h RTnode.h RTpredicate.h
RT.o: /usr/include/string.h /usr/include/features.h /usr/include/sys/cdefs.h
RT.o: /usr/include/gnu/stubs.h
RT.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
RT.o: ../libGiST/GiST.h ../libGiST/GiSTdefs.h /usr/include/assert.h
RT.o: /usr/include/stdlib.h /usr/include/math.h /usr/include/bits/huge_val.h
RT.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
RT.o: ../libGiST/GiSTentry.h ../libGiST/GiSTpredicate.h ../libGiST/GiSTlist.h
RT.o: ../libGiST/GiSTnode.h ../libGiST/GiSTpath.h ../libGiST/GiSTstore.h
RT.o: ../libGiST/GiSTcursor.h RT.h ../libGiST/GiSTdb.h ../../ttree.h
RT.o: ../../object.h ../../classinfo.h ../../storage.h ../../file.h
RT.o: ../../stdtp.h /usr/include/stdio.h /usr/include/bits/types.h
RT.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
RT.o: /usr/include/bits/wchar.h /usr/include/gconv.h
RT.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
RT.o: /usr/include/bits/stdio_lim.h /usr/include/limits.h
RT.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h ../../dnmarr.h
RT.o: RTentry.h /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
RT.o: RTnode.h RTpredicate.h
