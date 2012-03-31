# Microsoft Developer Studio Project File - Name="rexxlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=rexxlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rexxlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rexxlib.mak" CFG="rexxlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rexxlib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "rexxlib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rexxlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D REXX_API=__declspec(dllexport) /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D REXX_API=__declspec(dllexport) /D YAXX_NAMESPACE=Yaxx /D "WIN32" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /machine:I386
# ADD LINK32 ../toolslib/Release/toolslib.lib ../decnblib/Release/decnblib.lib ../reslib/Release/reslib.lib ../yasp3lib/Release/yasp3lib.lib /nologo /dll /machine:I386 /out:"../Release/rexxlib.dll"

!ELSEIF  "$(CFG)" == "rexxlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D REXX_API=__declspec(dllexport) /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D REXX_API=__declspec(dllexport) /D YAXX_NAMESPACE=Yaxx /D "WIN32" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /x /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../toolslib/Debug/toolslib.lib ../decnblib/Debug/decnblib.lib ../reslib/Debug/reslib.lib ../yasp3lib/Debug/yasp3lib.lib /nologo /dll /incremental:no /map /debug /machine:I386 /out:"../Debug/rexxlib.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "rexxlib - Win32 Release"
# Name "rexxlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Arguments.cpp
# End Source File
# Begin Source File

SOURCE=.\BatchCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltIn.cpp
# End Source File
# Begin Source File

SOURCE=.\CharConverter.cpp
# End Source File
# Begin Source File

SOURCE=.\Clauses.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Compiler.cpp
# End Source File
# Begin Source File

SOURCE=.\Constants.cpp
# End Source File
# Begin Source File

SOURCE=.\DataQueueMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\HashMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Interpreter.cpp
# End Source File
# Begin Source File

SOURCE=.\IOMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\IRexx.cpp
# End Source File
# Begin Source File

SOURCE=.\Label.cpp
# End Source File
# Begin Source File

SOURCE=.\OnTimeCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\Pool.cpp
# End Source File
# Begin Source File

SOURCE=.\Rexx.cpp
# End Source File
# Begin Source File

SOURCE=.\RexxString.cpp
# End Source File
# Begin Source File

SOURCE=.\Routines.cpp
# End Source File
# Begin Source File

SOURCE=.\Symbol.cpp
# End Source File
# Begin Source File

SOURCE=.\TimeClock.cpp
# End Source File
# Begin Source File

SOURCE=.\Tokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\Tracer.cpp
# End Source File
# Begin Source File

SOURCE=.\Variable.cpp
# End Source File
# Begin Source File

SOURCE=.\VariableHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\XmlIOMgr.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Arguments.h
# End Source File
# Begin Source File

SOURCE=.\BatchCompiler.h
# End Source File
# Begin Source File

SOURCE=.\BuiltIn.h
# End Source File
# Begin Source File

SOURCE=.\CharConverter.h
# End Source File
# Begin Source File

SOURCE=.\Clauses.h
# End Source File
# Begin Source File

SOURCE=.\CodeBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Compiler.h
# End Source File
# Begin Source File

SOURCE=.\Constants.h
# End Source File
# Begin Source File

SOURCE=.\DataQueueMgr.h
# End Source File
# Begin Source File

SOURCE=.\Exception.h
# End Source File
# Begin Source File

SOURCE=.\gendef.h
# End Source File
# Begin Source File

SOURCE=.\HashMap.h
# End Source File
# Begin Source File

SOURCE=.\Interpreter.h
# End Source File
# Begin Source File

SOURCE=.\IOMgr.h
# End Source File
# Begin Source File

SOURCE=.\IRexx.h
# End Source File
# Begin Source File

SOURCE=.\Label.h
# End Source File
# Begin Source File

SOURCE=.\OnTimeCompiler.h
# End Source File
# Begin Source File

SOURCE=.\Pool.h
# End Source File
# Begin Source File

SOURCE=.\Rexx.h
# End Source File
# Begin Source File

SOURCE=.\RexxString.h
# End Source File
# Begin Source File

SOURCE=.\Routines.h
# End Source File
# Begin Source File

SOURCE=.\Symbol.h
# End Source File
# Begin Source File

SOURCE=.\TimeClock.h
# End Source File
# Begin Source File

SOURCE=.\Tokenizer.h
# End Source File
# Begin Source File

SOURCE=.\Tracer.h
# End Source File
# Begin Source File

SOURCE=.\Variable.h
# End Source File
# Begin Source File

SOURCE=.\VariableHandler.h
# End Source File
# Begin Source File

SOURCE=.\XmlIOMgr.h
# End Source File
# End Group
# End Target
# End Project
