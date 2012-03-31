# $Id: Makefile,v 1.5 2002-03-21 05:52:37 pgr Exp $

SOURCES = \
	ecm.cpp \
	ecmc.cpp \
	ecmcp850.cpp \
	ecmeuc.cpp \
	ecmrom8.cpp \
	ecmsjis.cpp \
	ecmucs2.cpp \
	ecmucs2r.cpp \
	ecmutf8.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@ar ruv $@ $(OBJECTS)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
