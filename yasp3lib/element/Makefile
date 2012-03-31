# $Id: Makefile,v 1.6 2002-03-21 05:51:01 pgr Exp $

SOURCES = \
	attrib.cpp \
	element.cpp \
	elmmgr.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	@ar ruv $(LIBDIR)/lib$(LIBNAME).a $(OBJECTS)

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
