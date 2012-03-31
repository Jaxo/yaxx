# $Id: Makefile,v 1.7 2002-03-21 05:46:28 pgr Exp $

SOURCES = \
	charset.cpp \
	dlmbuild.cpp \
	dlmfind.cpp \
	dlmlist.cpp \
	features.cpp \
	quantity.cpp \
	rsvdname.cpp \
	sdcl.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	@ar ruv $(LIBDIR)/lib$(LIBNAME).a $(OBJECTS)

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
