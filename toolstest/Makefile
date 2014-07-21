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
	$(CXX) $(CXXFLAGS) -o $(EXENAME) \
		$(OBJECTS) \
		-L$(LIBDIR) -ltools -lencoding \
		-L/usr/local/lib -lstdc++ \

clean:
	-rm $(OBJECTS)
