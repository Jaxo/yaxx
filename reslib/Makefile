SOURCES = \
	BuiltInName.cpp \
	DayMonth.cpp \
	HtmlDtd.cpp \
	Keyword.cpp \
	MsgTemplate.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@ar ruv $@ $(OBJECTS)

