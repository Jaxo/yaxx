SOURCES = \
	main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: CXXFLAGS += -D COM_JAXO_YAXX_DENY_XML

all:	$(OBJECTS)
	g++ -o $(EXENAME) $(CPPFLAGS) -D COM_JAXO_YAXX_DENY_XML \
		$(OBJECTS) \
		-L$(LIBDIR) -lbarerexx -ldecnb -ltools -lencoding -lres -lm \
		-L/usr/local/lib -lstdc++ \

clean:
	-rm $(OBJECTS)

