# $Id: Makefile,v 1.4 2002-10-16 11:54:12 pgr Exp $

SOURCES = \
	DecNumber.cpp \
	DecRexx.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@ar ruv $@ $(OBJECTS)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
