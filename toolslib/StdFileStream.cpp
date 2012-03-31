/*
* $Id: StdFileStream.cpp,v 1.11 2011-07-29 10:26:37 pgr Exp $
*
* Standard File streams
*/

#ifdef __MWERKS__
#error Error: this file should not be used for systems with no native Basic IO
#endif

/*--------------+
| Include Files |
+--------------*/
#include <assert.h>

#if defined _WIN32
#include <io.h>
#include <direct.h>
#define mkstemp(t) _mktemp(t)
#define PATH_SEPARATOR '\\'

#elif defined unix
#include <sys/types.h>
#include <unistd.h>
#define PATH_SEPARATOR '/'
#endif

#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include "StdFileStream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

#if __GNUC__ >= 4
#define S_IREAD  S_IRUSR
#define S_IWRITE S_IWUSR
#endif

/*-----------------------------------------StdFileStreamBuf::StdFileStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
StdFileStreamBuf::StdFileStreamBuf() {
   m_fd = -1;
}

/*-----------------------------------------------------StdFileStreamBuf::open-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * StdFileStreamBuf::open(
   char const * pszPath,
   unsigned long opflgs
)
{
   #if defined _WIN32
   opflgs |= O_BINARY;
   #endif
   m_fd = ::open((char *)pszPath, opflgs, S_IWRITE|S_IREAD);
   return (m_fd >= 0)? this : 0;
}

/*----------------------------------------------------StdFileStreamBuf::close-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * StdFileStreamBuf::close()
{
   if (m_fd >= 0) {
      sync();
      ::close(m_fd);
      m_fd = -1;
   }
   resetBuf();
   return this;
}

/*-----------------------------------------------------StdFileStreamBuf::read-+
|                                                                             |
+----------------------------------------------------------------------------*/
int StdFileStreamBuf::read(char * pchBuf, int iLen) {
   return ((m_fd >= 0) && iLen)? ::read(m_fd, pchBuf, iLen) : 0;
}

/*----------------------------------------------------StdFileStreamBuf::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
int StdFileStreamBuf::write(char const * pchBuf, int iLen) {
   return ((m_fd >= 0) && iLen)? ::write(m_fd, pchBuf, iLen) : 0;
}

/*----------------------------------------------------StdFileStreamBuf::lseek-+
|                                                                             |
+----------------------------------------------------------------------------*/
long StdFileStreamBuf::lseek(long offset, int origin) {
   return (m_fd >= 0)? ::lseek(m_fd, offset, origin) : EOF;
}

/*------------------------------------------------------------StdFileInStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
StdFileInStream::StdFileInStream(
   char const * pszPath,
   unsigned long opflgs
) :
   istream(&m_buf)
{
   if (!m_buf.open((char *)pszPath, opflgs)) {
      clear(rdstate() | ios::failbit);
   }
}

/*------------------------------------------StdFileInStream::~StdFileInStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
StdFileInStream::~StdFileInStream() {
   m_buf.close();
}

/*-----------------------------------------------------------StdFileOutStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
StdFileOutStream::StdFileOutStream(
   char const * pszPath,
   unsigned long opflgs
) :
   ostream(&m_buf)
{
   if (!m_buf.open((char *)pszPath, opflgs)) {
      clear(rdstate() | ios::failbit);
   }
}

/*----------------------------------------StdFileOutStream::~StdFileOutStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
StdFileOutStream::~StdFileOutStream() {
   m_buf.close();
}

/*--------------------------------------------------------------StdFileStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
StdFileStream::StdFileStream(
   char const * pszPath,
   unsigned long opflgs
) :
   iostream(&m_buf)
{
   if (!m_buf.open((char *)pszPath, opflgs)) {
      clear(rdstate() | ios::failbit);
   }
}

/*----------------------------------------------StdFileStream::~StdFileStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
StdFileStream::~StdFileStream() {
   m_buf.close();
}


/*-------------------------------------------StdFileSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * StdFileSchemeHandler::Rep::getID() const {
   return "FILE";
}

/*--------------------------------------StdFileSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * StdFileSchemeHandler::Rep::makeStream(
   URI const & uri,
   ios__openmode om
) {
   char const * path = uri.getPath();
   #ifdef _WIN32
      // hide the leading forward slash in path is it is: "/d:/..."
      if (path[2] == ':') ++path;
   #endif
   if (om & ios::in) {
      return new StdFileStream(path, StdFileStreamBuf::OPEN_READ_FLAGS);
   }else {
      return new StdFileStream(path);
   }
}

/*STATIC---------------------------------------StdFileStreamBuf::makeTempPath-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * StdFileStreamBuf::makeTempPath(char * buf, char const * suffix) {
   int pathLen;
   char const * pTempDir = 0;
   pTempDir = getenv("TEMP");
   buf[0] = '\0';
   if (pTempDir) strcpy(buf, pTempDir);
   pathLen = strlen(buf);
   if (pathLen) {
      if (buf[pathLen-1] != PATH_SEPARATOR) {
         buf[pathLen] = PATH_SEPARATOR;
         buf[pathLen+1] = '\0';
      }
   }
   strcat(buf, "RxXXXXXX");
   mkstemp(buf);
   strcat(buf, suffix);
   return buf;
}

/*STATIC----------------------------------------StdFileStreamBuf::makeBaseUri-+
| The returned value must be freeed                                           |
+----------------------------------------------------------------------------*/
char * StdFileStreamBuf::makeBaseUri()
{
   static char const fileScheme[] = "file:///";
   char * cwd = getcwd(0, 0);
   char * fileBaseUri = (char *)malloc(sizeof fileScheme + strlen(cwd) + 1);
   strcpy(fileBaseUri, fileScheme);
   #if defined _WIN32
      {
         char const * src = cwd-1;
         char * tgt = fileBaseUri + sizeof fileScheme - 1;
         while (*++src) {
            *tgt++ = (*src == '\\')? '/' : *src;
         }
         *tgt++ = '/';
         *tgt = '\0';
      }
   #else
      strcat(fileBaseUri, cwd);
      strcat(fileBaseUri, "/");
   #endif
   free(cwd);
   return fileBaseUri;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
