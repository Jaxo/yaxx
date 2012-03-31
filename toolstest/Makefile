# $Id: Makefile,v 1.6 2002-05-29 13:23:16 pgr Exp $

SOURCES = \
	main.cpp \
	testFileStream.cpp \
	testList.cpp \
	testReaderWriter.cpp \
	testTransient.cpp \
	testTransition.cpp \
	testLinkedList.cpp \
	testMemStream.cpp \
	testURI.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	g++ $(CPPFLAGS) -o $(EXENAME) \
		$(OBJECTS) \
		-L$(LIBDIR) -ltools -lencoding \
		-L/usr/local/lib -lstdc++ \

clean:
	-rm $(OBJECTS) $(EXENAME)
