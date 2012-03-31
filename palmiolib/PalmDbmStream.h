/*
* $Id: PalmDbmStream.h,v 1.34 2002-08-26 14:26:21 pgr Exp $
*
*  Base classes for palm file streams
*
*  PalmDbmStreamBuf:  streambuf class for Palm-DataBaseManager based PDB
*
*  PalmDbmByteInStream and PalmDbmOutStream:
*     are istream/ostream using a PalmDbmStreamBuf
*     (as: ifstream/ofstream)
*/

#ifndef COM_JAXO_PALMIO_PALMDBMSTREAM_H_INCLUDED
#define COM_JAXO_PALMIO_PALMDBMSTREAM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "FileStreamBuf.h"
#include "migstream.h"
#include "iostream.h"
#include "URI.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

class DbmRecCursor;

/*-----------------------------------------------------------PalmDbmStreamBuf-+
|  open flags (unsigned long opflgs) must be one of:                          |
|        dmModeReadOnly         Open RO                        0x0001         |
|        dmModeWrite            Open RW                        0x0002         |
|        dmModeReadWrite        Open RW                        0x0003         |
|                                                                             |
|        Possibly OR'ed with:                                                 |
|                                                                             |
|        dmModeExclusive        don't let anyone else open it  0x0008         |
|        dmModeShowSecret       force show of secret records   0x0010         |
|                                                                             |
|  recNo                                                                      |
|        when -1      indicate a non-record-oriented (aka "Byte") stream      |
|        when 0       position to the first record (if any)                   |
|        when >= 1    indicates what rec number to start with                 |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmDbmStreamBuf : public FileStreamBuf {
public:
   PalmDbmStreamBuf();

   FileStreamBuf * open(
      char const * pszName,    // DataBase name ("MemoDB")
      unsigned long opflgs,    // openflags
      int cardNo,              // Card # (0)
      UInt32 creator,          // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
      UInt32 type,             // Type (data)
      char const * pCat,       // category
      int recNo,               // rec number to start with
      int sizeIncrt            // incrt size (new records)
   );
   FileStreamBuf * close();
   Err getErrorNo() const;

protected:
   Err m_err;
   DmOpenRef m_fh;
   DbmRecCursor * m_pCursor;
   int m_offset;
   int m_size;
   int m_sizeIncrt;            // only open_write (-1 for open_read)
   int m_totalSize;
   int m_positionEof;          // only open_write (-1 until known)

   long lseek(long offset, int origin);  // really: virtual
   virtual int firstRecordNum() = 0;
   virtual int lastRecordNum() = 0;

private:
   void resetFields();
   int computeBestSize(int iMin) const;
   bool createDataBase(
      int cardNo,
      char const * pszName,
      UInt32 creator,
      UInt32 type,
      unsigned long opflgs
   );
   bool setDbmRecCursor(char const * pCat, bool createIt);
   int read(char * pchBuf, int iLen);
   int write(char const * pchBuf, int iLen);
   bool nextRecord();
   bool prevRecord();

   PalmDbmStreamBuf & operator=(PalmDbmStreamBuf const & source); // no!
   PalmDbmStreamBuf(PalmDbmStreamBuf const & source);             // no!
};

/* -- INLINES -- */
inline Err PalmDbmStreamBuf::getErrorNo() const {
   return m_err;
}

/*-------------------------------------------------------PalmDbmByteStreamBuf-+
| This is the byte-oriented implementation of PalmDbmStreamBuf.               |
|                                                                             |
| The DataBase is seen as a continuous suite of characters,                   |
| and the underlying record structure is hidden.                              |
|                                                                             |
| However, when the DB exists, ONLY the length of the last record may         |
| change (if new bytes were added.)   For instance, rewriting "MyDb"          |
| with 24 bytes 'x' or 12 bytes 'y' leads to:                                 |
|                                                                             |
| MyDB (original) | overwritten with 24 'x': | overwritten with 12 'y':       |
|                 |                          |                                |
| "aaaaa"         | "xxxxx"                  | "yyyyy"                        |
| "bbbbbbbbb"     | "xxxxxxxxx"              | "yyyyyyy"                      |
| "cccc"          | "xxxxxxxxxx"             |                                |
|                                                                             |
| Also note the use of the MaxRecSize parameter: in overwritting with 24 'x', |
| the example above assumed that MacRecSize was at least 10.  If it had       |
| been set to 3, then overwritting with 23 'z' leads to:                      |
|                                                                             |
| "zzzzz"                                                                     |
| "zzzzzzzzz"                                                                 |
| "zzzz"                                                                      |
| "zz"                                                                        |
| "zz"                                                                        |
| "z"                                                                         |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmDbmByteStreamBuf : public PalmDbmStreamBuf {
private:
   int firstRecordNum();
   int lastRecordNum();
};

/*--------------------------------------------------------PalmDbmRecStreamBuf-+
| This is the record-oriented implementation of PalmDbmStreamBuf.             |
|                                                                             |
| Think about it as being a PalmDbmByte with only one record: e.g. when       |
| the end of the record is found, it is EOF (the next record in               |
| the DataBase is not fetched, as it would have been for a DbmByte.)          |
|                                                                             |
| However: seeks between record is allowed.  Here is how it works.            |
|                                                                             |
| The lseek method takes two parameters: offset, and origin.                  |
| offest is an integer, and origin is one of SEEK_SET, SEEK_CUR or            |
| SEEK_END.  Origin says that the offset is relative to (respectively)        |
| the beginning of the file (ios::beg), the current position (ios::cur),      |
| and the end of the file (ios::end.)                                         |
|                                                                             |
| Doing: lseek(-3, SEEK_END) would then position at 3 bytes before the        |
| end of the file.  And lseek(+3, SEEK_END) is... skipping BEYOND the end     |
| of the file.  The PalmDbmByteStreamBuf does not allow such action.          |
| (lseek "standard" behaviour in this case is wierd -- gap, no size extension |
| and we consider it to be highly error-prone.)                               |
|                                                                             |
| For PalmDbmRecStreamBuf, lseek(+3, SEEK_END) has a special meaning.         |
| The position will be set at +3 records after the end of the file.           |
| Similarly, lseek(-3, SEEK_SET) means to seek back at 3 records before       |
| the current record.  (SEEK_CUR does not cross records boundaries.)          |
|                                                                             |
| Note that PalmDbmRec does not provide any method to insert or delete        |
| a Record.  These actions are not the responsability of streambuf and        |
| are better handled directly by the methods of the Data Manager.             |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmDbmRecStreamBuf : public PalmDbmStreamBuf {
private:
   int firstRecordNum();
   int lastRecordNum();
   long lseek(long offset, int origin);
   long skipRecord(int delta);
};

/*----------------------------------------------------------PalmDbmByteStream-+
| PalmDbmByteStream IS-A iostream to manipulate a PalmDbmByteStreamBuf.       |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmDbmByteStream : public iostream {
public:
   PalmDbmByteStream(
      char const * pszName,    // DataBase name ("MemoDB")
      unsigned long opflgs = dmModeReadWrite,
      int cardNo = 0,          // Card # (0)
      UInt32 creator = 'Rexx', // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
      UInt32 type = 'data',    // Type (data)
      char const * pCat = 0,   // any category
      int sizeIncrt = 200      // incrt size (new records)
   );
   ~PalmDbmByteStream();
   Err getLastError() const;

private:
   PalmDbmByteStreamBuf m_buf;

   PalmDbmByteStream & operator=(PalmDbmByteStream const & source);   // no!
   PalmDbmByteStream(PalmDbmByteStream const & source);               // no!
};

inline Err PalmDbmByteStream::getLastError() const {
   return m_buf.getErrorNo();
}

/*-----------------------------------------------------------PalmDbmRecStream-+
| PalmDbmRecStream IS-A iostream to manipulate a PalmDbmRecStreamBuf.         |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmDbmRecStream : public iostream {
public:
   PalmDbmRecStream(
      char const * pszName,    // DataBase name ("MemoDB")
      unsigned long opflgs = dmModeReadWrite,
      int recNo = 0,           // record number
      int cardNo = 0,          // Card # (0)
      UInt32 creator = 'Rexx', // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
      UInt32 type = 'data',    // Type (data)
      char const * pCat = 0,   // any category
      int sizeIncrt = 200      // incrt size (new records)
   );
   ~PalmDbmRecStream();
   Err getLastError() const;

private:
   PalmDbmRecStreamBuf m_buf;

   PalmDbmRecStream & operator=(PalmDbmRecStream const & source);   // no!
   PalmDbmRecStream(PalmDbmRecStream const & source);               // no!
};

inline Err PalmDbmRecStream::getLastError() const {
   return m_buf.getErrorNo();
}

/*-------------------------------------------------------PalmDbmSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API PalmDbmSchemeHandler : public URI::SchemeHandler {
public:
   PalmDbmSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      void onNew(URI::Data & uri);
      void onDelete(URI::Data & uri);
      iostream * makeStream(URI const & uri, ios__openmode om);
   };
};

inline PalmDbmSchemeHandler::PalmDbmSchemeHandler() : SchemeHandler(new Rep) {
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
