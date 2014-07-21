.PHONY: bare full clean

SOURCES = \
	Arguments.cpp \
	BatchCompiler.cpp \
	BuiltIn.cpp \
	CharConverter.cpp \
	Clauses.cpp \
	CodeBuffer.cpp \
	Compiler.cpp \
	Constants.cpp \
	DataQueueMgr.cpp \
	Exception.cpp \
	HashMap.cpp \
	Interpreter.cpp \
	IOMgr.cpp \
	Label.cpp \
	OnTimeCompiler.cpp \
	Pool.cpp \
	Rexx.cpp \
	RexxString.cpp \
	Routines.cpp \
	Symbol.cpp \
	TimeClock.cpp \
	Tokenizer.cpp \
	Tracer.cpp \
	Variable.cpp \
	VariableHandler.cpp \
	XmlIOMgr.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

bare: bare.chk $(LIBDIR)/lib$(LIBNAME).a

full: full.chk $(LIBDIR)/lib$(LIBNAME).a

bare.chk:
	-rm $(OBJECTS) full.chk
	touch bare.chk

full.chk:
	-rm $(OBJECTS) bare.chk
	touch full.chk

clean:
	-rm $(OBJECTS)

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@$(AR) ruv $@ $(OBJECTS)

