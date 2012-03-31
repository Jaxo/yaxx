# $Id: Makefile,v 1.7 2002-03-21 05:47:15 pgr Exp $

SOURCES = \
	rcdbind.cpp \
	ysperror.cpp \
	yspevent.cpp \
	yspgrab.cpp \
	yspinput.cpp \
	yspmain.cpp \
	yspparse.cpp \
	yspsdcl.cpp \
	ysptag.cpp \
        CachedDtd.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	@ar ruv $(LIBDIR)/lib$(LIBNAME).a $(OBJECTS)

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
