# Microsoft Developer Studio Project File - Name="toolslib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=toolslib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "toolslib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "toolslib.mak" CFG="toolslib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "toolslib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "toolslib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "toolslib - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D TOOLS_API=__declspec(dllexport) /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D TOOLS_API=__declspec(dllexport) /D "WIN32" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ../reslib/Release/reslib.lib encoding/Release/encoding.lib /nologo /dll /machine:I386 /out:"../Release/toolslib.dll"

!ELSEIF  "$(CFG)" == "toolslib - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D TOOLS_API=__declspec(dllexport) /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D TOOLS_API=__declspec(dllexport) /D "WIN32" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /x /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../reslib/Debug/reslib.lib encoding/Debug/encoding.lib /nologo /dll /incremental:no /map /debug /machine:I386 /out:"../Debug/toolslib.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "toolslib - Win32 Release"
# Name "toolslib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\arglist.cpp
# End Source File
# Begin Source File

SOURCE=.\BinarySearch.cpp
# End Source File
# Begin Source File

SOURCE=.\ByteString.cpp
# End Source File
# Begin Source File

SOURCE=.\ConsoleSchemeHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\dlmalloc.c
# End Source File
# Begin Source File

SOURCE=.\Encoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Escaper.cpp
# End Source File
# Begin Source File

SOURCE=.\FileStreamBuf.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpSchemeHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\LinkedList.cpp
# End Source File
# Begin Source File

SOURCE=.\mbstring.cpp
# End Source File
# Begin Source File

SOURCE=.\MemStream.cpp
# End Source File
# Begin Source File

SOURCE=.\miscutil.cpp
# End Source File
# Begin Source File

SOURCE=.\new.cpp
# End Source File
# Begin Source File

SOURCE=.\Reader.cpp
# End Source File
# Begin Source File

SOURCE=.\RefdItem.cpp
# End Source File
# Begin Source File

SOURCE=.\RefdKey.cpp
# End Source File
# Begin Source File

SOURCE=.\RegisteredURI.cpp
# End Source File
# Begin Source File

SOURCE=.\RWBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\sort.cpp
# End Source File
# Begin Source File

SOURCE=.\StdFileStream.cpp
# End Source File
# Begin Source File

SOURCE=.\StringBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemContext.cpp
# End Source File
# Begin Source File

SOURCE=.\tpbhash.cpp
# End Source File
# Begin Source File

SOURCE=.\tpblist.cpp
# End Source File
# Begin Source File

SOURCE=.\tplist.cpp
# End Source File
# Begin Source File

SOURCE=.\tplistix.cpp
# End Source File
# Begin Source File

SOURCE=.\tplistwh.cpp
# End Source File
# Begin Source File

SOURCE=.\tpset.cpp
# End Source File
# Begin Source File

SOURCE=.\uccompos.cpp
# End Source File
# Begin Source File

SOURCE=.\ucformat.cpp
# End Source File
# Begin Source File

SOURCE=.\ucstring.cpp
# End Source File
# Begin Source File

SOURCE=.\URI.cpp
# End Source File
# Begin Source File

SOURCE=.\Writer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\arglist.h
# End Source File
# Begin Source File

SOURCE=.\BinarySearch.h
# End Source File
# Begin Source File

SOURCE=.\ByteString.h
# End Source File
# Begin Source File

SOURCE=.\ConsoleSchemeHandler.h
# End Source File
# Begin Source File

SOURCE=.\dlmalloc.h
# End Source File
# Begin Source File

SOURCE=.\Encoder.h
# End Source File
# Begin Source File

SOURCE=.\Escaper.h
# End Source File
# Begin Source File

SOURCE=.\FileStreamBuf.h
# End Source File
# Begin Source File

SOURCE=.\HttpSchemeHandler.h
# End Source File
# Begin Source File

SOURCE=.\LinkedList.h
# End Source File
# Begin Source File

SOURCE=.\mbstring.h
# End Source File
# Begin Source File

SOURCE=.\MemStream.h
# End Source File
# Begin Source File

SOURCE=.\migstream.h
# End Source File
# Begin Source File

SOURCE=.\miscutil.h
# End Source File
# Begin Source File

SOURCE=.\Reader.h
# End Source File
# Begin Source File

SOURCE=.\RefdItem.h
# End Source File
# Begin Source File

SOURCE=.\RefdKey.h
# End Source File
# Begin Source File

SOURCE=.\RegisteredURI.h
# End Source File
# Begin Source File

SOURCE=.\RWBuffer.h
# End Source File
# Begin Source File

SOURCE=.\sort.h
# End Source File
# Begin Source File

SOURCE=.\StdFileStream.h
# End Source File
# Begin Source File

SOURCE=.\StringBuffer.h
# End Source File
# Begin Source File

SOURCE=.\SystemContext.h
# End Source File
# Begin Source File

SOURCE=.\tpbhash.h
# End Source File
# Begin Source File

SOURCE=.\tpblist.h
# End Source File
# Begin Source File

SOURCE=.\tplist.h
# End Source File
# Begin Source File

SOURCE=.\tplistix.h
# End Source File
# Begin Source File

SOURCE=.\tplistwh.h
# End Source File
# Begin Source File

SOURCE=.\tpset.h
# End Source File
# Begin Source File

SOURCE=.\uccompos.h
# End Source File
# Begin Source File

SOURCE=.\ucformat.h
# End Source File
# Begin Source File

SOURCE=.\ucstring.h
# End Source File
# Begin Source File

SOURCE=.\URI.h
# End Source File
# Begin Source File

SOURCE=.\Writer.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\toolslib.def
# End Source File
# End Group
# End Target
# End Project
