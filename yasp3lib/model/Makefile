# $Id: Makefile,v 1.6 2002-03-21 05:47:51 pgr Exp $

SOURCES = \
	hitbits.cpp \
	mdllctr.cpp \
	mdltkn.cpp \
	mdltree.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	@ar ruv $(LIBDIR)/lib$(LIBNAME).a $(OBJECTS)

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
