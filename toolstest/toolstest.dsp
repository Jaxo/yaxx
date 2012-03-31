# Microsoft Developer Studio Project File - Name="toolstest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=toolstest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "toolstest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "toolstest.mak" CFG="toolstest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "toolstest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "toolstest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "toolstest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "toolstest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D TOOLS_API="" /D "SMALLBUF_FOR_TESTS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "toolstest - Win32 Release"
# Name "toolstest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\toolslib\BinarySearch.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\ConsoleSchemeHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecm.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmc.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmcp850.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmeuc.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmrom8.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmsjis.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmucs2.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmucs2r.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecmutf8.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\Encoder.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\Escaper.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\FileStreamBuf.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\HttpSchemeHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\LinkedList.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\MemStream.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\Reader.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\RefdItem.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\RegisteredURI.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\RWBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\StdFileStream.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\SystemContext.cpp
# End Source File
# Begin Source File

SOURCE=.\testEscaper.cpp
# End Source File
# Begin Source File

SOURCE=.\testFileStream.cpp
# End Source File
# Begin Source File

SOURCE=.\testLinkedList.cpp
# End Source File
# Begin Source File

SOURCE=.\testList.cpp
# End Source File
# Begin Source File

SOURCE=.\testMemStream.cpp
# End Source File
# Begin Source File

SOURCE=.\testReaderWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\testTransient.cpp
# End Source File
# Begin Source File

SOURCE=.\testTransition.cpp
# End Source File
# Begin Source File

SOURCE=.\testURI.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\tpblist.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\ucstring.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\URI.cpp
# End Source File
# Begin Source File

SOURCE=..\toolslib\Writer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\toolslib\BinarySearch.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\ConsoleSchemeHandler.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\cp850.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\ecm.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\Encoder.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\Escaper.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\euc2uc.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\FileStreamBuf.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\gendef.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\HttpSchemeHandler.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\LinkedList.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\MemStream.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\migstream.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\Reader.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\RefdItem.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\RefdKey.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\RegisteredURI.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\roman8.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\RWBuffer.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\sjis2uc.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\StdFileStream.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\SystemContext.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\tpblist.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\tplist.h
# End Source File
# Begin Source File

SOURCE=.\TransientFileStreamBuf.h
# End Source File
# Begin Source File

SOURCE=.\Transition.h
# End Source File
# Begin Source File

SOURCE=.\TransitionList.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\uc2euc.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\encoding\uc2sjis.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\ucformat.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\ucstring.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\URI.h
# End Source File
# Begin Source File

SOURCE=..\toolslib\Writer.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
