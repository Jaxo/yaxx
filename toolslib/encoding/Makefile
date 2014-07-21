SOURCES = \
	ecm.cpp \
	ecmc.cpp \
	ecmcp850.cpp \
	ecmeuc.cpp \
	ecmrom8.cpp \
	ecmsjis.cpp \
	ecmucs2.cpp \
	ecmucs2r.cpp \
	ecmutf8.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@$(AR) ruv $@ $(OBJECTS)

