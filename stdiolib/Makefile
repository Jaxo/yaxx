# $Id: Makefile,v 1.5 2013-08-02 06:38:58 pgr Exp $

SOURCES= \
	basio.c \
	memio.c \
	printf.c \
	stdio.c \
#	java2c.c \
#	jniio.c \

LIBDIR ?= ../lib
LIBNAME ?= stdio
OBJECTS = $(SOURCES:.c=.o)

all:	$(LIBDIR)/lib$(LIBNAME).a
-include $(SOURCES:.c=.d)

override CFLAGS+=-DLIBMAKER_$(LIBNAME) -DUSE_INPROC $(STDIO_INCLUDE) -MMD


clean:
	-rm -f $(OBJECTS) $(SOURCES:.c=.d) $(LIBDIR)/lib$(LIBNAME).a

depend:
	g++ -E -MMD $(CFLAGS) $(SOURCES) > /dev/null

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@$(AR) ruv $@ $(OBJECTS)

#makedepend -- $(CFLAGS) -- *.[ch]

# DO NOT DELETE THIS LINE --															   make depend depends on it.
