# $Id: Makefile,v 1.8 2002-05-11 07:07:14 jlatone Exp $

SOURCES = \
	BuiltInName.cpp \
	DayMonth.cpp \
	HtmlDtd.cpp \
	Keyword.cpp \
	MsgTemplate.cpp

OBJECTS = $(SOURCES:.cpp=.o)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@ar ruv $@ $(OBJECTS)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
