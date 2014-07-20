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
	-rm $(OBJECTS)
