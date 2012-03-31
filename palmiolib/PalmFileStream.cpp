/*
* $Id: PalmFileStream.cpp,v 1.22 2003-01-22 09:18:34 pgr Exp $
*
* Palm File streams
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include <unix_fcntl.h>
#include <stdio.h>
#include <string.h>
#include "miscutil.h"
#include "PalmFileStream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

struct PalmFilePath {
   UInt32 creator;
   UInt32 type;
   UInt32 cardNo;
   char const * path;
};

/*---------------------------------------PalmFileStreamBuf::PalmFileStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
PalmFileStreamBuf::PalmFileStreamBuf() : m_fh(0), m_isWrite(false) {
}

/*--------------------------------------PalmFileStreamBuf::~PalmFileStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
PalmFileStreamBuf::~PalmFileStreamBuf() {
   if (m_fh) close();
}

/*----------------------------------------------------PalmFileStreamBuf::open-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * PalmFileStreamBuf::open(
   char const * path,
   unsigned long opflgs,
   UInt32 cardNo,
   UInt32 creator,
   UInt32 type
) {

   if (m_fh) {
      m_err = fileErrInUse;
      return 0;
   }else {
      m_isWrite = (opflgs & fileModeReadOnly) == 0;
      m_fh = FileOpen(
         cardNo,
         path,
         type,
         creator,
         opflgs,
         &m_err
      );
      if (m_err != 0) {
         m_fh = 0;
         m_path = 0;
         return 0;
      }else {
         m_cardNo = cardNo;
         strcpy(m_path = new char[1+strlen(path)], path);
         return this;
      }
   }
}

/*---------------------------------------------------PalmFileStreamBuf::close-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * PalmFileStreamBuf::close()
{
   if (m_fh) {
      int pos = -1;
      sync();
      if (m_isWrite) {
         pos = FileTell(m_fh, 0, 0);
         if (pos > 0) FileTruncate(m_fh, pos);
      }
      m_err = FileClose(m_fh);
      if (pos == 0) {
         m_err = FileDelete(m_cardNo, m_path);
      }
      m_fh = 0;
      resetBuf();
      delete [] m_path;
      m_path = 0;
      return (m_err == 0)? this : 0;
   }else {
      m_err = fileErrCloseError;
      return 0;
   }
}

/*----------------------------------------------------PalmFileStreamBuf::read-+
|                                                                             |
+----------------------------------------------------------------------------*/
int PalmFileStreamBuf::read(char * pchBuf, int iLen)
{
   m_err = 0;       // reset
   if (iLen <= 0) {
      return 0;     // reading or writing 0 bytes is always OK
   }else if (!m_fh) {
      m_err = fileErrInvalidDescriptor;
      return 0;
   }else {
      iLen = FileRead(m_fh, pchBuf, 1, iLen, &m_err);
      return (!m_err || (m_err == fileErrEOF))? iLen : EOF; // EOF means error
   }
}

/*---------------------------------------------------PalmFileStreamBuf::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
int PalmFileStreamBuf::write(char const * pchBuf, int iLen)
{
   m_err = 0;       // reset
   if (iLen <= 0) {
      return 0;     // reading or writing 0 bytes is always OK
   }else if (!m_fh) {
      m_err = fileErrInvalidDescriptor;
      return 0;
   }else {
      iLen = FileWrite(m_fh, pchBuf, 1, iLen, &m_err);
      return (!m_err)? iLen : EOF;   // EOF means error
   }
}

/*---------------------------------------------------PalmFileStreamBuf::lseek-+
|                                                                             |
+----------------------------------------------------------------------------*/
long PalmFileStreamBuf::lseek(long offset, int origin)
{
   if (!m_fh) {
      m_err = fileErrInvalidDescriptor;
      return EOF;
   }else {
      FileOriginEnum ori;
      switch (origin) {
      case SEEK_CUR :   ori = fileOriginCurrent;      break;
      case SEEK_END :   ori = fileOriginEnd;          break;
      case SEEK_SET :   ori = fileOriginBeginning;    break;
      }
      m_err = FileSeek(m_fh, offset, ori);
      return (m_err == 0)? FileTell(m_fh, 0, 0) : EOF;
   }
}

/*---------------------------------------------PalmFileStream::PalmFileStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
PalmFileStream::PalmFileStream(
   char const * path,
   unsigned long opflgs,
   UInt32 cardNo,
   UInt32 creator,
   UInt32 type,
   char * buf,
   int len

) :
   iostream(&m_buf)
{
   m_buf.pubsetbuf(buf, len);
   if (!m_buf.open(path, opflgs, cardNo, creator, type)) {
      clear(rdstate() | ios::failbit);
   }
}

/*--------------------------------------------PalmFileStream::~PalmFileStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
PalmFileStream::~PalmFileStream() {
   m_buf.close();
}

/*------------------------------------------PalmFileSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * PalmFileSchemeHandler::Rep::getID() const {
   return "FILE";
}

/*------------------------------------------PalmFileSchemeHandler::Rep::onNew-+
| This implements the following syntax:                                       |
|                                                                             |
| "file://"                required                                           |
| (                                                                           |
|    creator               default is "Rexx"                                  |
|    ("." type)?           default is "data"                                  |
|    "@"                   don't forget the '@'!  <=== WARNING                |
| )?                                                                          |
| palmhostId?              default is "localhost"  FOR FUTURE USE             |
| (":" cardno)?            default is 0                                       |
| "/" fileName             required                                           |
+----------------------------------------------------------------------------*/
void PalmFileSchemeHandler::Rep::onNew(URI::Data & uri)
{
   char * pCreator = 0;
   char * pType = 0;
   unsigned char subCode = 0;

   if (uri.path[0] == '\0') {
      //>>>PGR: this is an error, indeed, but no way to catch it, sigh.
      /*
      | file:// (no path) is our BaseUri, but there is no easy way to
      | catch the 'valid' exception (due to an abusive use of macros:
      | FORM_MAP_ENTRY, etc.)  So we will just pretend that this is
      | correct, but keep uri.user set to NULL.  In case someone
      | tries to open the base URI, he will be caught later.
      | Sorry, but I can't do better.
      */
      //subCode = 1;              // error: missing file name
      return;
   }else {
      pCreator = uri.userInfo;
      if (pCreator) {
         pType=strchr(uri.userInfo, '.');
         if (pType && (*pType = '\0', strlen(++pType) > 4)) {
            subCode = 2;          // error: type's name exceeds 4 bytes
         }else if (strlen(pCreator) > 4) {
            subCode = 3;          // error: creator's name exceeds 4 bytes
         }
      }
   }
   if (subCode != 0) {
      throw URI::MalformedException(URI::HANDLER_ERROR, subCode);
   }else {
      PalmFilePath * p = new PalmFilePath;
      if (!pCreator || (p->creator=c2int(pCreator), !p->creator)) {
         p->creator = 'Rexx';
      }
      if (!pType || (p->type=c2int(pType), !p->type)) {
         p->type = 'data';
      }
      // hide the leading forward slash
      p->path = 1 + uri.path;
      p->cardNo = uri.port;
      if (p->cardNo == -1) p->cardNo = 0;
      uri.user = p;
   }
}

/*---------------------------------------PalmFileSchemeHandler::Rep::onDelete-+
|                                                                             |
+----------------------------------------------------------------------------*/
void PalmFileSchemeHandler::Rep::onDelete(URI::Data & uri) {
   delete (PalmFilePath *)uri.user;
}

/*-------------------------------------PalmFileSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * PalmFileSchemeHandler::Rep::makeStream(
   URI const & uri, ios__openmode om
) {
   assert (uri.getUser());
   if (!uri.getUser()) {
      return 0;   // see above
   }else {
      PalmFilePath const * p = (PalmFilePath const *)uri.getUser();
      unsigned long opflgs;
      if (om == ios::in) {
         opflgs = fileModeReadOnly | fileModeAnyTypeCreator;
      }else {
         opflgs = fileModeUpdate | fileModeAnyTypeCreator;
      }
      return new PalmFileStream(
         p->path,
         opflgs,
         p->cardNo,
         p->creator,
         p->type
      );
   }
}

/*----------------------------------PalmFileRedirectorSchemeHandler::Rep::Rep-+
|                                                                             |
+----------------------------------------------------------------------------*/
PalmFileRedirectorSchemeHandler::Rep::Rep(
   char const * cinPath,
   char const * coutPath,
   char const * cerrPath
) {
   m_pCin = RegisteredURI(
      cinPath? cinPath : ConsoleSchemeHandler::cinUri
   ).getStream(ios::in);
   m_pCout = RegisteredURI(
      coutPath? coutPath : ConsoleSchemeHandler::coutUri
   ).getStream(ios::out);
   m_pCerr = RegisteredURI(
      cerrPath? cerrPath : ConsoleSchemeHandler::cerrUri
   ).getStream(ios::out);
}

/*---------------------------------PalmFileRedirectorSchemeHandler::Rep::~Rep-+
|                                                                             |
+----------------------------------------------------------------------------*/
PalmFileRedirectorSchemeHandler::Rep::~Rep() {
   delete m_pCin;
   delete m_pCout;
   delete m_pCerr;
}

/*--------------------------------------------PalmFileStreamBuf::makeTempPath-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * PalmFileStreamBuf::makeTempPath(char * buf, char const * suffix)
{
   strcpy(buf, "RxXXXXXX");
   strcat(buf, suffix);
   return buf;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
