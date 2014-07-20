SOURCES = \
	DecNumber.cpp \
	DecRexx.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@ar ruv $@ $(OBJECTS)

