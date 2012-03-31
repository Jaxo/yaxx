# $Id: Makefile,v 1.15 2002-04-09 07:43:52 pgr Exp $

SOURCES = \
	main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	g++ -o $(EXENAME) $(CPPFLAGS) \
		$(OBJECTS) \
		-L$(LIBDIR) -lrexx -ldecnb -lyasp3 -ltools -lencoding -lres -lm \
		-L/usr/local/lib -lstdc++ \

clean:
	-rm $(OBJECTS) $(EXENAME)
