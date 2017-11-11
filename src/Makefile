#
# Makefile for sv2agw DJGPP libsocket interface
#
#

include Makefile.cfg

CFLAGS  += -I../include -Wall -DUSE_FORTIFY	-DTN_SRV
LDFLAGS += -L../lib
LIBS     = -lsocket

OBJS = ringbuf.o agwsubs.o agwpe_if.o tn_srv.o cmd.o fortify.o

.PHONY:	all dep clean distclean dist depend.dep

test.exe: test.c $(OBJS)
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) $(OBJS) $(LIBS) 
#	strip $@

fortify.o: fortify.c fortify.h
	$(CC) -c $(CFLAGS) fortify.c

tn_srv.o: tn_srv.c
	$(CC) -c $(CFLAGS) tn_srv.c

cmd.o: cmd.c
	$(CC) -c $(CFLAGS) cmd.c

ringbuf.o: ringbuf.c ringbuf.h
	$(CC) -c $(CFLAGS) ringbuf.c

agwpe_if.o: agwpe_if.c agwpe_if.h
	$(CC) -c $(CFLAGS) agwpe_if.c

agwsubs.o: agwsubs.c agwsubs.h
	$(CC) -c $(CFLAGS) agwsubs.c

%.exe:	%.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) $(OBJS) $(LIBS)
ifeq ($(LSCK_STRIPALL),y)
	strip $@
endif

all:	test.exe servern.exe depend.dep

dep:	depend.dep
	$(CC) $(CFLAGS) -MM *.c > depend.dep

clean:
	rm -f *.o

distclean: clean
	rm -f *.\$$\$$\$$
	rm -f *~
	rm -f $(TARGETS)


lint:
	lclint *.c -Id:\djgpp\include -warnposix -weak -retvalint

cxref:
	rm -f ../doc/cxref/*.*
	cxref *.c *.h -DTN_SRV -Id:/djgpp/include -O../doc/cxref -xref-all -Nsv2agw -index-all
	cxref *.c *.h -DTN_SRV -Id:/djgpp/include -O../doc/cxref -xref-all -Nsv2agw -index-all -html32 -html32-src
	
info:
	makeinfo --output=../doc/info/agwtcplib.info agwtcplib.txi
	makeinfo --html --output=../doc/info/agwtcplib.html agwtcplib.txi

html: test.c
	sh -c "/cygwin/usr/local/bin/code2html -l c -o html $< >../doc/src_html/$<.html"

dist:
	rm -f ../distrib/*
	pkzip -rp d:\djgpp\tmp\agwpelib.zip ..\*.*
	mv d:\djgpp\tmp\agwpelib.zip ..\distrib
	cp ../doc/info/agwtcplib.html ../distrib

$(OBJS) $(LOBJS):
include depend.dep
