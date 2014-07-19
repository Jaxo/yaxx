# $Id: Makefile,v 1.3 2002/04/09 07:43:52 pgr Exp $

SOURCES = \
	main.cpp \
	DecTest.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	g++ -o $(EXENAME) $(CPPFLAGS) \
		$(OBJECTS) \
		-L$(LIBDIR) -ldecnb \
		-L/usr/local/lib -lstdc++ \

clean:
	-rm $(OBJECTS) $(EXENAME)
