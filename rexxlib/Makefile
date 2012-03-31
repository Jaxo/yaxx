# $Id: Makefile,v 1.12 2002-10-16 11:52:11 pgr Exp $

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

$(LIBDIR)/lib$(LIBNAME).a:	$(OBJECTS)
	@ar ruv $@ $(OBJECTS)

all:	$(LIBDIR)/lib$(LIBNAME).a

clean:
	-rm $(OBJECTS) $(LIBDIR)/lib$(LIBNAME).a
