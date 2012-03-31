# $Id: Makefile,v 1.15 2003-01-21 15:52:34 pgr Exp $

SOURCES = \
	arglist.cpp \
	BinarySearch.cpp \
	ByteString.cpp \
	ConsoleSchemeHandler.cpp \
	Encoder.cpp \
	Escaper.cpp \
	FileStreamBuf.cpp \
	HttpSchemeHandler.cpp \
	LinkedList.cpp \
	mbstring.cpp \
	MemStream.cpp \
	miscutil.cpp \
	Reader.cpp \
        RegisteredURI.cpp \
	RefdItem.cpp \
	RefdKey.cpp \
	RWBuffer.cpp \
	sort.cpp \
	StdFileStream.cpp \
	StringBuffer.cpp \
	SystemContext.cpp \
	tpbhash.cpp \
	tpblist.cpp \
	tplist.cpp \
	tplistix.cpp \
	tplistwh.cpp \
	tpset.cpp \
	uccompos.cpp \
	ucformat.cpp \
	ucstring.cpp \
	URI.cpp \
	Writer.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

$(LIBDIR)/lib$(LIBNAME).a:      $(OBJECTS)
	@ar ruv $@ $(OBJECTS)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
