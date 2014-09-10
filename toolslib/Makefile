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

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS) dlmalloc.o

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS) dlmalloc.o
	@$(AR) ruv $@ $(OBJECTS) dlmalloc.o
