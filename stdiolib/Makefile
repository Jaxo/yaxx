# $Id: Makefile,v 1.5 2013-08-02 06:38:58 pgr Exp $

SOURCES= \
	basio.c \
	memio.c \
	printf.c \
	stdio.c \
#	java2c.c \
#	jniio.c \

OBJECTS=$(SOURCES:.c=.o)

override CFLAGS+=-DLIBMAKER_$(LIBNAME) -DUSE_INPROC $(STDIO_INCLUDE)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@$(AR) ruv $@ $(OBJECTS)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm -f $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a

depend:
	makedepend -- $(CFLAGS) -- *.[ch]

# DO NOT DELETE THIS LINE -- make depend depends on it.

