# Microsoft Developer Studio Project File - Name="syntax" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=syntax - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "syntax.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "syntax.mak" CFG="syntax - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "syntax - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "syntax - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "syntax - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "_LIB" /D "_DLL" /D "NDEBUG" /D "WIN32" /D YASP3_API=__declspec(dllexport) /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "syntax - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "_LIB" /D "_DLL" /D "_DEBUG" /D "WIN32" /D YASP3_API=__declspec(dllexport) /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "syntax - Win32 Release"
# Name "syntax - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\charset.cpp
# End Source File
# Begin Source File

SOURCE=.\dlmbuild.cpp
# End Source File
# Begin Source File

SOURCE=.\dlmfind.cpp
# End Source File
# Begin Source File

SOURCE=.\dlmlist.cpp
# End Source File
# Begin Source File

SOURCE=.\features.cpp
# End Source File
# Begin Source File

SOURCE=.\quantity.cpp
# End Source File
# Begin Source File

SOURCE=.\rsvdname.cpp
# End Source File
# Begin Source File

SOURCE=.\sdcl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\charset.h
# End Source File
# Begin Source File

SOURCE=.\delim.h
# End Source File
# Begin Source File

SOURCE=.\dlmbuild.h
# End Source File
# Begin Source File

SOURCE=.\dlmfind.h
# End Source File
# Begin Source File

SOURCE=.\dlmlist.h
# End Source File
# Begin Source File

SOURCE=.\features.h
# End Source File
# Begin Source File

SOURCE=.\quantity.h
# End Source File
# Begin Source File

SOURCE=.\rsvdname.h
# End Source File
# Begin Source File

SOURCE=.\sdcl.h
# End Source File
# End Group
# End Target
# End Project
