/*
* $Id: PalmFileStream.h,v 1.24 2003-01-22 09:19:04 pgr Exp $
*
*  Base classes for palm file streams
*
*  PalmFileStreamBuf:  streambuf class for Palm-File based PDB
*  PalmFileStreamBuf:
*     is an example of a derived class that uses fread() and
*     fwrite() to feed/empty the buffer.
*
*  PalmFileInStream and PalmFileOutStream:
*     are istream/ostream using a PalmFileStreamBuf
*     (as: ifstream/ofstream)
*/

#ifndef COM_JAXO_PALMIO_PALMFILESTREAM_H_INCLUDED
#define COM_JAXO_PALMIO_PALMFILESTREAM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "FileStreamBuf.h"
#include "migstream.h"
#include "iostream.h"
#include "ConsoleSchemeHandler.h"
#include "RegisteredURI.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------------------PalmFileStreamBuf-+
|  open flags (unsigned long opflgs) must be one of:                          |
|                                                                             |
|  fileModeReadOnly       Open RO                                             |
|  fileModeReadWrite      Open/create RW, trunc                               |
|  fileModeUpdate         Open/create RW                                      |
|  fileModeAppend         Open/create RW, app                                 |
|                                                                             |
|  Possibly OR'ed with:                                                       |
|                                                                             |
|  fileModeDontOverWrite  if fileModeReadWrite, create only                   |
|  fileModeExclusive      exclusive access                                    |
|  fileModeAnyTypeCreator not same creator is ok                              |
|  fileModeTemporary      temp file (deleted when closed)                     |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmFileStreamBuf : public FileStreamBuf {
public:
   PalmFileStreamBuf();
   ~PalmFileStreamBuf();

   Err getErrorNo() const;

   FileStreamBuf * open(
      char const * path,
      unsigned long opflgs = fileModeUpdate | fileModeAnyTypeCreator,
      UInt32 cardNo = 0,
      UInt32 creator = 'Rexx',
      UInt32 type = 'data'
   );
   FileStreamBuf * close();
   static char * makeTempPath(char * buf, char const * suffix);

private:
   FileHand m_fh;
   bool m_isWrite;
   UInt32 m_cardNo;
   char * m_path;
   Err m_err;

   int read(char * pchBuf, int iLen);
   int write(char const * pchBuf, int iLen);
   long lseek(long offset, int origin);

   PalmFileStreamBuf & operator=(PalmFileStreamBuf const & source); // no!
   PalmFileStreamBuf(PalmFileStreamBuf const & source);             // no!
};

inline Err PalmFileStreamBuf::getErrorNo() const {
   return m_err;
}

/*-------------------------------------------------------------PalmFileStream-+
| PalmFileStream IS-A iostream to manipulate a PalmFileStreamBuf.             |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmFileStream: public iostream {
public:
   PalmFileStream(
      char const * path,
      unsigned long opflgs = fileModeUpdate | fileModeAnyTypeCreator,
      UInt32 cardNo = 0,
      UInt32 creator = 'Rexx',
      UInt32 type = 'data',
      char * buf = 0,
      int len = 64
   );
   ~PalmFileStream();
   Err getLastError() const;

private:
   PalmFileStreamBuf m_buf;

   PalmFileStream & operator=(PalmFileStream const & source); // no!
   PalmFileStream(PalmFileStream const & source);             // no!
};

inline Err PalmFileStream::getLastError() const {
   return m_buf.getErrorNo();
}

/*------------------------------------------------------PalmFileSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmFileSchemeHandler : public URI::SchemeHandler {
public:
   PalmFileSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      void onNew(URI::Data & uri);
      void onDelete(URI::Data & uri);
      iostream * makeStream(URI const & uri, ios__openmode om);
   };
};

inline PalmFileSchemeHandler::PalmFileSchemeHandler(
) :
   SchemeHandler(new Rep) {
}

/*--------------------------------------------PalmFileRedirectorSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class PalmFileRedirectorSchemeHandler : public ConsoleSchemeHandler {
public:
   PalmFileRedirectorSchemeHandler(
      char const * cinPath = 0,
      char const * coutPath = 0,
      char const * cerrPath = 0
   );

private:
   class Rep : public ConsoleSchemeHandler::Rep {
   public:
      Rep(char const * cinPath, char const * coutPath, char const * cerrPath);
      ~Rep();
   private:
      istream & getCin();
      ostream & getCout();
      ostream & getCerr();

      istream * m_pCin;
      ostream * m_pCout;
      ostream * m_pCerr;
   };
};

/* -- INLINES -- */
inline PalmFileRedirectorSchemeHandler::PalmFileRedirectorSchemeHandler(
   char const * cinPath,
   char const * coutPath,
   char const * cerrPath
) :
   ConsoleSchemeHandler(new Rep(cinPath, coutPath, cerrPath)) {
}
inline istream & PalmFileRedirectorSchemeHandler::Rep::getCin()  {
   return *m_pCin;
}
inline ostream & PalmFileRedirectorSchemeHandler::Rep::getCout() {
   return *m_pCout;
}
inline ostream & PalmFileRedirectorSchemeHandler::Rep::getCerr() {
   return *m_pCerr;
}


#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
