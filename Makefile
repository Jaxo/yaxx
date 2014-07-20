# $Id: Makefile,v 1.14 2002-04-05 09:28:16 pgr Exp $
# To make a debug version, do:
# make -e "CXXFLAGS=-O0 -g -D _DEBUG"

.PHONY: all clean yaxx rexx dectest toolstest
HERE := $(shell pwd)
MISC = "LIBDIR=$(HERE)/lib"

yaxx: CXXFLAGS += -O2 -D NDEBUG
yaxx:
	mkdir -p $(HERE)/lib
	@cd decnblib          ; $(MAKE) "LIBNAME=decnb" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd reslib            ; $(MAKE) "LIBNAME=res" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd toolslib          ; $(MAKE) "LIBNAME=tools" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd toolslib/encoding ; $(MAKE) "LIBNAME=encoding" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd rexxlib           ; $(MAKE) full "LIBNAME=rexx" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd yasp3lib          ; $(MAKE) "LIBNAME=yasp3" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd yaxx              ; $(MAKE) "EXENAME=yaxx" "CXXFLAGS=$(CXXFLAGS)" $(MISC)

rexx: CXXFLAGS += -O2 -D NDEBUG
rexx:
	mkdir -p $(HERE)/lib
	@cd decnblib          ; $(MAKE) "LIBNAME=decnb" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd reslib            ; $(MAKE) "LIBNAME=res" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd toolslib          ; $(MAKE) "LIBNAME=tools" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd toolslib/encoding ; $(MAKE) "LIBNAME=encoding" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd rexxlib           ; $(MAKE) bare "LIBNAME=barerexx" "CXXFLAGS=$(CXXFLAGS) -D COM_JAXO_YAXX_DENY_XML" $(MISC)
	@cd rexx              ; $(MAKE) "EXENAME=rexx" "CXXFLAGS=$(CXXFLAGS)" $(MISC)

dectest: CXXFLAGS += -O2 -D NDEBUG
dectest:
	mkdir -p $(HERE)/lib
	@cd decnblib          ; $(MAKE) "LIBNAME=decnb" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd dectest           ; $(MAKE) "EXENAME=dectest" "CXXFLAGS=$(CXXFLAGS)" $(MISC)

toolstest: CXXFLAGS += -O2 -D NDEBUG
toolstest:
	mkdir -p $(HERE)/lib
	@cd toolslib          ; $(MAKE) "LIBNAME=tools" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd toolslib/encoding ; $(MAKE) "LIBNAME=encoding" "CXXFLAGS=$(CXXFLAGS)" $(MISC)
	@cd toolstest         ; $(MAKE) "EXENAME=toolstest" "CXXFLAGS=$(CXXFLAGS)" $(MISC)

all: yaxx rexx dectest toolstest                        q

clean:
	rm -rf $(HERE)/lib
