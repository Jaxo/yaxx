SOURCES = \
	main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(OBJECTS)
	$(CXX) -o $(EXENAME) $(CXXFLAGS) \
		$(OBJECTS) \
		-L$(LIBDIR) -lrexx -ldecnb -lyasp3 -ltools -lencoding -lres -lm \
		-L/usr/local/lib -lstdc++ \

clean:
	-rm $(OBJECTS)
