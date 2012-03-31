
1. If you want to switch between CW8 and CW9, then
   you should do this to go back & forth:

   CW8:
   cd mwerks
   cvs update -A decnblib/decnblib.mcp irexx2/irexx2.mcp palmiolib/palmiolib.mcp rexxlets/RexxletHack/RexxletHack.mcp rexxlib/rexxlib-builtin.mcp rexxlib/rexxlib-compiler.mcp rexxlib/rexxlib-interpreter.mcp rexxlib/rexxlib-misc.mcp rexxlib/rexxlib-rexxstring.mcp rexxlib/rexxlib-routinesvar.mcp toolslib/toolslib1.mcp toolslib/toolslib2.mcp yasp3lib/yasp3lib-element.mcp yasp3lib/yasp3lib-entity.mcp yasp3lib/yasp3lib-model.mcp yasp3lib/yasp3lib-parser1.mcp yasp3lib/yasp3lib-parser2.mcp yasp3lib/yasp3lib-syntax.mcp yasp3lib/yasp3lib.mcp 
   rm -rf ./decnblib/decnblib_Data ./irexx2/irexx2_Data ./palmiolib/palmiolib_Data ./rexxapps/hello/hello_Data ./rexxapps/MVILoto/MVILoto_Data ./rexxapps/MVISpace/MVISpace_Data ./rexxlets/hello/hello_Data ./rexxlets/RexxletHack/RexxletHack_Data ./rexxlib/rexxlib-builtin_Data ./rexxlib/rexxlib-compiler_Data ./rexxlib/rexxlib-interpreter_Data ./rexxlib/rexxlib-misc_Data ./rexxlib/rexxlib-rexxstring_Data ./rexxlib/rexxlib-routinesvar_Data ./toolslib/toolslib1_Data ./toolslib/toolslib2_Data ./yasp3lib/yasp3lib-element_Data ./yasp3lib/yasp3lib-entity_Data ./yasp3lib/yasp3lib-model_Data ./yasp3lib/yasp3lib-parser1_Data ./yasp3lib/yasp3lib-parser2_Data ./yasp3lib/yasp3lib-syntax_Data  ./yasp3lib/yasp3lib_Data
   rm lib/*

   CW9:
   cd mwerks
   cvs update -r CW9 decnblib/decnblib.mcp irexx2/irexx2.mcp palmiolib/palmiolib.mcp rexxlets/RexxletHack/RexxletHack.mcp rexxlib/rexxlib-builtin.mcp rexxlib/rexxlib-compiler.mcp rexxlib/rexxlib-interpreter.mcp rexxlib/rexxlib-misc.mcp rexxlib/rexxlib-rexxstring.mcp rexxlib/rexxlib-routinesvar.mcp toolslib/toolslib1.mcp toolslib/toolslib2.mcp yasp3lib/yasp3lib-element.mcp yasp3lib/yasp3lib-entity.mcp yasp3lib/yasp3lib-model.mcp yasp3lib/yasp3lib-parser1.mcp yasp3lib/yasp3lib-parser2.mcp yasp3lib/yasp3lib-syntax.mcp yasp3lib/yasp3lib.mcp 
   rm -rf ./decnblib/decnblib_Data ./irexx2/irexx2_Data ./palmiolib/palmiolib_Data ./rexxapps/hello/hello_Data ./rexxapps/MVILoto/MVILoto_Data ./rexxapps/MVISpace/MVISpace_Data ./rexxlets/hello/hello_Data ./rexxlets/RexxletHack/RexxletHack_Data ./rexxlib/rexxlib-builtin_Data ./rexxlib/rexxlib-compiler_Data ./rexxlib/rexxlib-interpreter_Data ./rexxlib/rexxlib-misc_Data ./rexxlib/rexxlib-rexxstring_Data ./rexxlib/rexxlib-routinesvar_Data ./toolslib/toolslib1_Data ./toolslib/toolslib2_Data ./yasp3lib/yasp3lib-element_Data ./yasp3lib/yasp3lib-entity_Data ./yasp3lib/yasp3lib-model_Data ./yasp3lib/yasp3lib-parser1_Data ./yasp3lib/yasp3lib-parser2_Data ./yasp3lib/yasp3lib-syntax_Data  ./yasp3lib/yasp3lib_Data
   rm lib/*

2. If you want a CW8 and a CW9 mwerks directory to co-exist,
   then here's what I did, assuming you have a CW8 mwerks dir.
   NOTE that this doesn't work perfectly, because cvs does not
   recognize the renamed mwerks8 directory when recursing from above.

   mv mwerks mwerks8
   cd ../..
   cvs co yaxx/mwerks
   cd yaxx/mwerks
   cvs update -r CW9 decnblib/decnblib.mcp irexx2/irexx2.mcp palmiolib/palmiolib.mcp rexxlets/RexxletHack/RexxletHack.mcp rexxlib/rexxlib-builtin.mcp rexxlib/rexxlib-compiler.mcp rexxlib/rexxlib-interpreter.mcp rexxlib/rexxlib-misc.mcp rexxlib/rexxlib-rexxstring.mcp rexxlib/rexxlib-routinesvar.mcp toolslib/toolslib1.mcp toolslib/toolslib2.mcp yasp3lib/yasp3lib-element.mcp yasp3lib/yasp3lib-entity.mcp yasp3lib/yasp3lib-model.mcp yasp3lib/yasp3lib-parser1.mcp yasp3lib/yasp3lib-parser2.mcp yasp3lib/yasp3lib-syntax.mcp yasp3lib/yasp3lib.mcp 

As an aside, CW8 and CW9 can co-exist on the same machine,
but since there are registry issues (e.g., associations and
version-dependent PROGIDs), you have to run their regservers.bat
if you want to switch between the two.  I don't do this, though.
I live with the CW9 registry settings and build the CW8 version
manually by loading all the MCPs and building each one since
the build script will not work due to the PROGID issue
(it would have been nice if CW used version-dependent PROGIDs).
