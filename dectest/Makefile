SOURCES = \
	main.cpp \
	DecTest.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	$(CXX) -o $(EXENAME) $(CXXFLAGS) \
		$(OBJECTS) \
		-L$(LIBDIR) -ldecnb \
		-L/usr/local/lib -lstdc++ \

clean:
	-rm $(OBJECTS)
