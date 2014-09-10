# Usage: make
#         (debug=1)? (os=(win32 | win64))? (clean)?
#         (rexx | yaxx | dectest | toolstest | all)

.PHONY: all clean yaxx rexx dectest toolstest
HERE := $(shell pwd)
ifdef debug
CPPFLAGS += -O0 -g -D _DEBUG
else
CPPFLAGS += -O2 -D NDEBUG
endif

ifeq ($(os),win32)
 export CXX := i686-w64-mingw32-g++   # i586-mingw32msvc-g++
 export CC := i686-w64-mingw32-gcc    # i586-mingw32msvc-gcc
 export AR := i686-w64-mingw32-ar     # i586-mingw32msvc-ar
 RUN_EXT := _32.exe
 CPPFLAGS += -D _WIN32 -D WIN32 -D NODLL -static-libgcc -static-libstdc++
else ifeq ($(os),win64)
 export CXX := x86_64-w64-mingw32-g++
 export CC := x86_64-w64-mingw32-gcc
 export AR := x86_64-w64-mingw32-ar
 RUN_EXT := _64.exe
 CPPFLAGS += -D _WIN32 -D WIN32 -D NODLL -static-libgcc -static-libstdc++
else
 RUN_EXT := .out
endif

$(eval $(shell csh version CPPFLAGS))

export LIBDIR := $(HERE)/lib
export CPPFLAGS

yaxx:
	mkdir -p $(HERE)/lib
	@cd decnblib          ; $(MAKE) "LIBNAME=decnb"
	@cd reslib            ; $(MAKE) "LIBNAME=res"
	@cd toolslib          ; $(MAKE) "LIBNAME=tools"
	@cd toolslib/encoding ; $(MAKE) "LIBNAME=encoding"
	@cd rexxlib           ; $(MAKE) full "LIBNAME=rexx"
	@cd yasp3lib          ; $(MAKE) "LIBNAME=yasp3"
	@cd yaxx              ; $(MAKE) "EXENAME=yaxx$(RUN_EXT)"

rexx: CPPFLAGS += -D COM_JAXO_YAXX_DENY_XML
rexx:
	mkdir -p $(HERE)/lib
	@cd decnblib          ; $(MAKE) "LIBNAME=decnb"
	@cd reslib            ; $(MAKE) "LIBNAME=res"
	@cd toolslib          ; $(MAKE) "LIBNAME=tools"
	@cd toolslib/encoding ; $(MAKE) "LIBNAME=encoding"
	@cd rexxlib           ; $(MAKE) bare "LIBNAME=barerexx"
	@cd rexx              ; $(MAKE) "EXENAME=rexx$(RUN_EXT)"

dectest:
	mkdir -p $(HERE)/lib
	@cd decnblib          ; $(MAKE) "LIBNAME=decnb"
	@cd dectest           ; $(MAKE) "EXENAME=dectest$(RUN_EXT)"

toolstest:
	mkdir -p $(HERE)/lib
	@cd toolslib          ; $(MAKE) "LIBNAME=tools"
	@cd toolslib/encoding ; $(MAKE) "LIBNAME=encoding"
	@cd toolstest         ; $(MAKE) "EXENAME=toolstest$(RUN_EXT)"

all: yaxx rexx dectest toolstest

clean:
	@cd decnblib          ; $(MAKE) clean
	@cd reslib            ; $(MAKE) clean
	@cd toolslib          ; $(MAKE) clean
	@cd toolslib/encoding ; $(MAKE) clean
	@cd rexxlib           ; $(MAKE) clean
	@cd yasp3lib          ; $(MAKE) clean
	@cd yaxx              ; $(MAKE) clean
	@cd rexx              ; $(MAKE) clean
	@cd dectest           ; $(MAKE) clean
	@cd toolstest         ; $(MAKE) clean
	rm -rf $(HERE)/lib
