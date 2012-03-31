# $Id: Makefile,v 1.6 2002-03-21 05:49:05 pgr Exp $

SOURCES = \
	entity.cpp \
	entmgr.cpp \
	entsolve.cpp \
	entstrea.cpp \
	externid.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	@ar ruv $(LIBDIR)/lib$(LIBNAME).a $(OBJECTS)

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
