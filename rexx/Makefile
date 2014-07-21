.PHONY: all

SOURCES = \
	main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	$(CXX) -o $(EXENAME) $(CPPFLAGS) -D COM_JAXO_YAXX_DENY_XML \
		$(OBJECTS) \
		-L$(LIBDIR) -lbarerexx -ldecnb -ltools -lencoding -lres -lm \
		-L/usr/local/lib

clean:
	-rm $(OBJECTS)

