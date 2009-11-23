LIBRARY = libGiST.a

SRCS = GiSTnode.cpp GiST.cpp GiSTfile.cpp GiSTpredicate.cpp GiSTcursor.cpp GiSTdb.cpp
       
OBJS = $(SRCS:.cpp=.o)

DEST = /usr/local/lib

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCS) $*.cpp

$(LIBRARY): .depend $(OBJS)
	rm -f $(LIBRARY)
	ar rs $(LIBRARY) $(OBJS)

clean:
	rm -f $(OBJS) $(LIBRARY) *~ #*# *pure*

depend:
	makedepend $(DEFINES) $(INCS) $(SRCS)

.depend:
	$(MAKE) depend
	touch .depend

install: $(PROGRAM)
	echo Installing $(PROGRAM) in $(DEST)
	install -s $(PROGRAM) $(DEST)

# DO NOT DELETE

GiSTnode.o: /usr/include/string.h /usr/include/features.h
GiSTnode.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
GiSTnode.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h GiST.h
GiSTnode.o: GiSTdefs.h /usr/include/assert.h /usr/include/stdlib.h
GiSTnode.o: /usr/include/math.h /usr/include/bits/huge_val.h
GiSTnode.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
GiSTnode.o: GiSTentry.h GiSTpredicate.h GiSTlist.h GiSTnode.h GiSTpath.h
GiSTnode.o: GiSTstore.h GiSTcursor.h
GiST.o: /usr/include/string.h /usr/include/features.h
GiST.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
GiST.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h GiST.h
GiST.o: GiSTdefs.h /usr/include/assert.h /usr/include/stdlib.h
GiST.o: /usr/include/math.h /usr/include/bits/huge_val.h
GiST.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h GiSTentry.h
GiST.o: GiSTpredicate.h GiSTlist.h GiSTnode.h GiSTpath.h GiSTstore.h
GiST.o: GiSTcursor.h
GiSTfile.o: /usr/include/stdio.h /usr/include/features.h
GiSTfile.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
GiSTfile.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
GiSTfile.o: /usr/include/bits/types.h /usr/include/libio.h
GiSTfile.o: /usr/include/_G_config.h /usr/include/wchar.h
GiSTfile.o: /usr/include/bits/wchar.h /usr/include/gconv.h
GiSTfile.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
GiSTfile.o: /usr/include/bits/stdio_lim.h /usr/include/sys/types.h
GiSTfile.o: /usr/include/time.h /usr/include/sys/stat.h
GiSTfile.o: /usr/include/bits/stat.h /usr/include/fcntl.h
GiSTfile.o: /usr/include/bits/fcntl.h /usr/include/string.h
GiSTfile.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
GiSTfile.o: /usr/include/bits/confname.h GiSTfile.h GiSTstore.h GiSTdefs.h
GiSTfile.o: /usr/include/assert.h /usr/include/stdlib.h /usr/include/math.h
GiSTfile.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
GiSTfile.o: /usr/include/bits/mathcalls.h
GiSTpredicate.o: GiST.h GiSTdefs.h /usr/include/assert.h
GiSTpredicate.o: /usr/include/features.h /usr/include/sys/cdefs.h
GiSTpredicate.o: /usr/include/gnu/stubs.h /usr/include/stdlib.h
GiSTpredicate.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
GiSTpredicate.o: /usr/include/math.h /usr/include/bits/huge_val.h
GiSTpredicate.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
GiSTpredicate.o: GiSTentry.h /usr/include/string.h GiSTpredicate.h GiSTlist.h
GiSTpredicate.o: GiSTnode.h GiSTpath.h GiSTstore.h GiSTcursor.h
GiSTcursor.o: GiST.h GiSTdefs.h /usr/include/assert.h /usr/include/features.h
GiSTcursor.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
GiSTcursor.o: /usr/include/stdlib.h
GiSTcursor.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
GiSTcursor.o: /usr/include/math.h /usr/include/bits/huge_val.h
GiSTcursor.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
GiSTcursor.o: GiSTentry.h /usr/include/string.h GiSTpredicate.h GiSTlist.h
GiSTcursor.o: GiSTnode.h GiSTpath.h GiSTstore.h GiSTcursor.h
GiSTdb.o: GiSTdb.h GiSTstore.h GiSTdefs.h /usr/include/assert.h
GiSTdb.o: /usr/include/features.h /usr/include/sys/cdefs.h
GiSTdb.o: /usr/include/gnu/stubs.h /usr/include/stdlib.h
GiSTdb.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
GiSTdb.o: /usr/include/math.h /usr/include/bits/huge_val.h
GiSTdb.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
GiSTdb.o: GiSTpath.h
