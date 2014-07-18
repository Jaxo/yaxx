# $Id: Makefile,v 1.14 2002-04-05 09:28:16 pgr Exp $

.PHONY: all clean dectest toolstest

HERE := $(shell pwd)

# To override standard vars like CPPFLAGS, use make -e CPPFLAGS="..."
MISC = \
	"CPPFLAGS = -O "   \
	"LIBDIR = $(HERE)/lib"

all clean:
	mkdir -p $(HERE)/lib
	@cd decnblib          ; $(MAKE) $@ "LIBNAME = decnb"  $(MISC)
	@cd reslib            ; $(MAKE) $@ "LIBNAME = res"  $(MISC)
	@cd toolslib          ; $(MAKE) $@ "LIBNAME = tools"  $(MISC)
	@cd toolslib/encoding ; $(MAKE) $@ "LIBNAME = encoding"  $(MISC)
	@cd rexxlib           ; $(MAKE) $@ "LIBNAME = rexx"  $(MISC)
	@cd yasp3lib          ; $(MAKE) $@ "LIBNAME = yasp3" $(MISC)
	@cd yaxx              ; $(MAKE) $@ "EXENAME = yaxx" $(MISC)

dectest:
	@cd decnblib          ; $(MAKE) all "LIBNAME = decnb"  $(MISC)
	@cd dectest           ; $(MAKE) clean all "EXENAME = dectest"  $(MISC)

toolstest:
	@cd toolslib          ; $(MAKE) all "LIBNAME = tools"  $(MISC)
	@cd toolslib/encoding ; $(MAKE) all "LIBNAME = encoding"  $(MISC)
	@cd toolstest         ; $(MAKE) clean all "EXENAME = toolstest" $(MISC)
