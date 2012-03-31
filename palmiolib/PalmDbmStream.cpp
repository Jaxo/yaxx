/*
* $Id: PalmDbmStream.cpp,v 1.58 2002-09-11 09:24:16 pgr Exp $
*
* Palm DataBase Manager streams
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "miscutil.h"
#include "DbmRecCursor.h"
#include "PalmDbmStream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

// #include <MemoryPrv.h>
#define memMaxChunkAllocSize 32767

struct DbmPath {
   UInt32 creator;
   UInt32 type;
   int recNo;               // 0 if undefined
   char const * category;   // may be empty
};

/*-----------------------------------------PalmDbmStreamBuf::PalmDbmStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
PalmDbmStreamBuf::PalmDbmStreamBuf() : m_err(0) {
   resetFields();
   pubsetbuf(0, 64);
}

/*----------------------------------------------PalmDbmStreamBuf::resetFields-+
| m_err is special (not reset here)                                           |
+----------------------------------------------------------------------------*/
void PalmDbmStreamBuf::resetFields()
{
   m_positionEof = -1;
   m_fh = 0;
   m_offset = 0;
   m_size = 0;
   m_totalSize = 0;
   m_pCursor = 0;
}

/*-----------------------------------------------------PalmDbmStreamBuf::open-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * PalmDbmStreamBuf::open(
   char const * pszName,    // DataBase name ("MemoDB")
   unsigned long opflgs,    // openflags
   int cardNo,              // Card # (0)
   UInt32 creator,          // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
   UInt32 type,             // Type (data)
   char const * pCat,       // any category (not "Unfiled"!)
   int recNo,               // rec number to start with
   int sizeIncrt            // incrt size (new records)
) {
   bool isWriteMode;
   if (m_fh) {
      m_err = fileErrInUse;
      return 0;
   }
   isWriteMode = (opflgs & dmModeWrite)? true : false;
   if (isWriteMode) {                   // opened in write mode
      if (sizeIncrt <= 0) {
         m_err = fileErrInvalidParam;
         return 0;
      }
      LocalID id = DmFindDatabase(cardNo, pszName);
      if (!id) {                        // double check it doesn't exist
         m_err = DmGetLastErr();
         if (
           (m_err != dmErrCantFind) ||  // other than "inexistant"?
           !createDataBase(cardNo, pszName, creator, type, opflgs)
         ) {
            return 0;
         }
      }else {
         m_fh = DmOpenDatabase(cardNo, id, opflgs);
      }
      m_sizeIncrt = sizeIncrt;
   }else {
      LocalID id = DmFindDatabase(cardNo, pszName);
      if (id) m_fh = DmOpenDatabase(cardNo, id, opflgs);
      m_sizeIncrt = -1;                 // in read mode
   }
   if (!m_fh) {
      m_err = DmGetLastErr();
      return 0;
   }

   if (setDbmRecCursor(pCat, isWriteMode)) {
      MemHandle curRec;
      if (recNo < 0) {                  // a DbmByteStream
         /*
         | DbmByteStream are not even advertized at today (6/16/02)
         | But it works, anyway.
         */
         curRec = m_pCursor->skip(0);
         if (curRec) {
            m_size = MemHandleSize(curRec);
         }
         return this;
      }else {                           // a DbmRecStream
         /*
         | For DbmRecStream, the recNo should be passed.
         | However, when no recNo is passed:
         | - if the file is in read-mode, the read position is at the
         |   first byte of the first record
         | - if the file is in write mode, a new record is created
         */
         if (isWriteMode) {

/* Better (but not yet verified)
|           int maxNum = m_pCursor->lastNum();
|           if ((recNo == 0) || (recNo > maxNum)) { // add a new record
|              if (recNo > maxNum+1) {
|                 curRec = 0;
|              }else {
|                 m_pCursor->skip(maxNum - m_pCursor->currentNum());
|                 curRec = m_pCursor->add(sizeIncrt);
|                 m_positionEof = 0;
|              }
|           }else {
|              curRec = m_pCursor->skip(recNo - 1);
|           }
*/
            int maxNum = m_pCursor->lastNum() + 1;
            if ((recNo == 0) || (recNo == maxNum)) { /* add a new record */
               m_pCursor->skip(maxNum - 1 - m_pCursor->currentNum());
               curRec = m_pCursor->add(sizeIncrt);
               m_positionEof = 0;
            }else if (recNo < maxNum) {
               curRec = m_pCursor->skip(recNo - 1);
            }else {
               curRec = 0;
            }
         }else {
            curRec = m_pCursor->skip((recNo > 1)? (recNo - 1) : 0);
         }
         if (curRec) {
            m_size = MemHandleSize(curRec);
            return this;
         }
      }
   }
   DmCloseDatabase(m_fh);
   resetFields();
   m_err = dmErrInvalidParam;
   return 0;
}

/*-------------------------------------------PalmDbmStreamBuf::createDataBase-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool PalmDbmStreamBuf::createDataBase(
   int cardNo,              // Card # (0)
   char const * pszName,    // DataBase name ("MemoDB")
   UInt32 creator,          // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
   UInt32 type,             // Type (data)
   unsigned long opflgs     // openflags
) {
   LocalID id;
   LocalID idInfo;
   AppInfoPtr pInfo;
   UInt16 attr;

   m_err = DmCreateDatabase(cardNo, pszName, creator, type, false);
   if (m_err) return false;
   id = DmFindDatabase(cardNo, (char *)pszName);
   if (!id) {
      m_err = DmGetLastErr();
      return false;               // can't create it.
   }
   m_fh = DmOpenDatabase(cardNo, id, opflgs);
   if (!m_fh) {
      m_err = DmGetLastErr();
      return false;
   }

   // Init the AppInfo
   m_err = DmOpenDatabaseInfo(m_fh, 0, 0, 0, 0, 0);
   if (m_err) goto clear;

   m_err = DmDatabaseInfo(cardNo, id, 0, &attr, 0, 0, 0, 0, 0, &idInfo, 0, 0, 0);
   if (m_err) goto clear;

   if (!idInfo) {         // brand new!
      MemHandle h = DmNewHandle(m_fh, sizeof *pInfo);
      if (!h) {
         m_err = dmErrMemError;
         goto clear;
      }
      idInfo = MemHandleToLocalID(h);
      attr |= dmHdrAttrBackup;
      DmSetDatabaseInfo(cardNo, id, 0, &attr, 0, 0, 0, 0, 0, &idInfo, 0, 0, 0);
   }
   pInfo = (AppInfoPtr)MemLocalIDToLockedPtr(idInfo, cardNo);
   DmSet(pInfo, 0, sizeof *pInfo, 0);    // clear the AppInfo block
   MemPtrUnlock(pInfo);                  // release!
   return true;

clear:
   DmCloseDatabase(m_fh);
   DmDeleteDatabase(cardNo, id);
   m_fh = 0;
   return false;
}

/*----------------------------------------------------PalmDbmStreamBuf::close-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * PalmDbmStreamBuf::close()
{
   if (m_fh) {
      m_err = 0;                    // reset
      sync();
      if (m_pCursor->isOpened() && (m_sizeIncrt > 0)) { // write_mode
         m_pCursor->removeFollowersTo(lastRecordNum()); // remove extras
         m_pCursor->resize(m_offset);                   // shrink current
         m_err = DmGetLastErr();
      }
      delete m_pCursor;
      Err err = DmCloseDatabase(m_fh);
      if (!m_err) m_err = err;     // whatever was first
      resetFields();
      resetBuf();
      return (m_err == 0)? this : 0;
   }else {
      m_err = fileErrCloseError;
      return 0;
   }
}

/*------------------------------------------PalmDbmStreamBuf::setDbmRecCursor-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool PalmDbmStreamBuf::setDbmRecCursor(char const * pCat, bool createIt)
{
   if (pCat) {
      char catName[dmCategoryLength];
      int ixCat = -1;
      for (int i = 0; ; ++i) {
         if (i == dmRecNumCategories) {
            if (!createIt || (ixCat < 0)) {
               return false;
            }else {
               CategorySetName(m_fh, ixCat, pCat);
               break;
            }
         }else {
            CategoryGetName(m_fh, i, catName);
            if (!catName[0]) {
               if (ixCat == -1) ixCat = i;
            }else if (!strcasecmp(catName, pCat)) {
               ixCat = i;
               break;
            }
         }
      }
      m_pCursor = new CategorizedDbmRecCursor(m_fh, ixCat);
   }else {
      m_pCursor = new SequentialDbmRecCursor(m_fh);
   }
   return true;
}

/*-----------------------------------------------------PalmDbmStreamBuf::read-+
| m_offset is the offset withing the current record                           |
| m_size is the size of the current record                                    |
| m_totalSize is the cumulated size of all records, excluding the current one |
+----------------------------------------------------------------------------*/
int PalmDbmStreamBuf::read(char * pchBuf, int iLen)
{
   m_err = 0;                       // reset
   if (iLen <= 0) {
      return 0;                     // reading or writing 0 bytes is always OK
   }else if (!m_fh) {
      m_err = fileErrInvalidDescriptor;
      return EOF;
   }else {
      char * pTgt = pchBuf;
      do {
         if ((m_offset == m_size) && !nextRecord()) {
            break;                  // EOF or error
         }
         int iMax = m_size - m_offset;
         if (iMax > iLen) iMax = iLen;
         if (
            (m_positionEof != -1) &&
            (iMax > (m_positionEof - m_totalSize - m_offset))
         ) {
            iMax = iLen = m_positionEof - m_totalSize - m_offset;
         }
         m_err = m_pCursor->get(pTgt, m_offset, iMax);
         if (!m_err) {
            m_offset += iMax;
            pTgt += iMax;
            iLen -= iMax;
         }else {
            break;
         }
      }while (iLen);
      return (!m_err)? pTgt - pchBuf : EOF;   // EOF really means error
   }
}

/*------------------------------------------PalmDbmStreamBuf::computeBestSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int PalmDbmStreamBuf::computeBestSize(int iMin) const {
   if (iMin + m_sizeIncrt >= memMaxChunkAllocSize) {
      return memMaxChunkAllocSize;
   }else {
      return (iMin + m_sizeIncrt);
   }
}

/*----------------------------------------------------PalmDbmStreamBuf::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
int PalmDbmStreamBuf::write(char const * pchBuf, int iLen)
{
   m_err = 0;             // reset.  you didn't catch it b4?  shame on you.
   if (iLen <= 0) {
      return 0;           // reading or writing 0 bytes is always OK
   }else if (!m_fh) {
      m_err = fileErrInvalidDescriptor;
      return EOF;
   }else {
      char const * pSrc = pchBuf;
      assert (m_sizeIncrt > 0);
      do {
         /*
         | if this is the last record, for the the first time
         | we notice it (m_positionEof is -1, e.g. not yet set),
         | set the EOF position (we can no more rely on the size)
         */
         if (m_positionEof == -1) {
            int maxNum = lastRecordNum();
            if (!maxNum || (m_pCursor->currentNum() == maxNum)) {
               // no record, or last record
               m_positionEof = m_totalSize + m_size;
            }
         }

         /*
         | Ensure room.  If current record is full, then,
         | if the next record exists, get it and reuse it,
         | or make a brand new one.
         */
         if (m_offset == m_size) {
            MemHandle curRec;
            int maxNum = lastRecordNum();
            int curNum = m_pCursor->currentNum();
            if (maxNum > curNum) {
               // next record exists!
               m_totalSize += m_size;
               curRec = m_pCursor->next();
               if (curRec) m_size = MemHandleSize(curRec);
               m_offset = 0;
            }else if (maxNum &&  (m_size < memMaxChunkAllocSize)) {
               // this is the last record, resize it
               m_size = computeBestSize(m_offset + iLen);
               curRec = m_pCursor->resize(m_size);
            }else {
               // no record, or last record not resizable
               m_totalSize += m_size;
               m_size = computeBestSize(iLen);
               curRec = m_pCursor->add(m_size);
               m_offset = 0;
            }
            if (!curRec) {
               m_err = DmGetLastErr();
               break;                  // I/O error
            }
         }

         /*
         | There is room.  Try to put here as many as we can.
         */
         int iMax = m_size - m_offset;
         if (iMax > iLen) iMax = iLen;
         m_err = m_pCursor->put(pSrc, m_offset, iMax);
         if (!m_err) {
            m_offset += iMax;
            pSrc += iMax;
            iLen -= iMax;
         }else {
            break;
         }
      }while (iLen);

      /*
      | Done.  Keep the EOF position up to date!
      */
      if (m_positionEof != -1) {
         int positionHere = m_totalSize + m_offset;
         if (positionHere > m_positionEof) {
            m_positionEof = positionHere;
         }
      }
      return (!m_err)? pSrc - pchBuf : EOF;   // EOF really means "error"
   }
}

/*-----------------------------------------------PalmDbmStreamBuf::nextRecord-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool PalmDbmStreamBuf::nextRecord()
{
   if (m_pCursor->currentNum() >= lastRecordNum()) {
      m_offset = m_size;
      return false;           // EOF (no more records to read)
   }
   m_totalSize += m_size;
   MemHandle curRec = m_pCursor->next();
   if (curRec == 0) {
      m_err = DmGetLastErr();
      return false;           // I/O error
   }
   m_offset = 0;
   m_size = MemHandleSize(curRec);
   return true;
}

/*-----------------------------------------------PalmDbmStreamBuf::prevRecord-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool PalmDbmStreamBuf::prevRecord()
{
   if (m_pCursor->currentNum() <= firstRecordNum()) {
      m_offset = 0;
      return false;               // BOF (before first record)
   }
   MemHandle curRec = m_pCursor->prev();
   if (curRec == 0) {
      m_err = DmGetLastErr();
      return false;               // I/O error
   }
   m_offset = m_size = MemHandleSize(curRec);
   m_totalSize -= m_size;
   return true;
}

/*----------------------------------------------------PalmDbmStreamBuf::lseek-+
|                                                                             |
+----------------------------------------------------------------------------*/
long PalmDbmStreamBuf::lseek(long offset, int origin)
{
   m_err = 0;
   if (!m_fh) {
      m_err = fileErrInvalidDescriptor;
      return EOF;
   }else {
      int newPos;
      switch (origin) {
      case SEEK_END:              // the worst
         while (nextRecord()) {}  // seek to the end
         if ((m_positionEof != -1) && (m_size > m_positionEof)) {
            newPos = m_positionEof + offset;
         }else {
            newPos = m_totalSize + m_size + offset;
         }
         break;
      case SEEK_SET:              // % Beginning
         newPos = offset;
         break;
      case SEEK_CUR:
         newPos = m_totalSize + m_offset + offset;
         break;
      default:
         return EOF;
      }
      if (newPos < m_totalSize) {
         do {
            if (!prevRecord()) {
               newPos = EOF;                         // too low!
               break;
            }
         }while (m_totalSize > newPos);
      }else {
         while (newPos > m_totalSize + m_size) {
            if (!nextRecord()) {
               newPos = EOF;                         // too high!
               break;
            }
         }
         /*
         | For write, the last record may not be fully written
         | and we don't want to seek beyond the positionEof limit!
         */
         if ((m_positionEof != -1) && (newPos > m_positionEof)) {
            m_offset = m_positionEof - m_totalSize;
            newPos = EOF;                            // too high!
         }
      }
      if (newPos != EOF) m_offset = newPos - m_totalSize;
      return newPos;
   }
}

/*---------------------------------------PalmDbmByteStreamBuf::firstRecordNum-+
| Ranged to the full DB nums                                                  |
+----------------------------------------------------------------------------*/
int PalmDbmByteStreamBuf::firstRecordNum() {
   return 0;
}

/*----------------------------------------PalmDbmByteStreamBuf::lastRecordNum-+
| Ranged to the full DB nums                                                  |
+----------------------------------------------------------------------------*/
int PalmDbmByteStreamBuf::lastRecordNum() {
   return m_pCursor->lastNum();
}

/*-------------------------------------------------PalmDbmRecStreamBuf::lseek-+
|                                                                             |
+----------------------------------------------------------------------------*/
long PalmDbmRecStreamBuf::lseek(long offset, int origin)
{
   switch (origin) {
   case SEEK_END:
      if (offset <= 0) {
         return PalmDbmStreamBuf::lseek(offset, SEEK_END);
      }else {
         return skipRecord(offset);
      }
   case SEEK_SET:              // % Beginning
      if (offset >= 0) {
         return PalmDbmStreamBuf::lseek(offset, SEEK_SET);
      }else {
         return skipRecord(offset);
      }
   case SEEK_CUR:
      return PalmDbmStreamBuf::lseek(offset, SEEK_CUR);
   default:
      return EOF;
   }
}

/*----------------------------------------PalmDbmRecStreamBuf::firstRecordNum-+
| Restricted to me, and me alone.                                             |
+----------------------------------------------------------------------------*/
int PalmDbmRecStreamBuf::firstRecordNum() {
   return m_pCursor->currentNum();
}

/*-----------------------------------------PalmDbmRecStreamBuf::lastRecordNum-+
| Restricted to me, and me alone.                                             |
+----------------------------------------------------------------------------*/
int PalmDbmRecStreamBuf::lastRecordNum() {
   return m_pCursor->currentNum();
}

/*--------------------------------------------PalmDbmRecStreamBuf::skipRecord-+
| For record-oriented streams                                                 |
+----------------------------------------------------------------------------*/
long PalmDbmRecStreamBuf::skipRecord(int delta)
{
   m_err = 0;
   if (!m_fh) {
      m_err = fileErrInvalidDescriptor;
      return EOF;
   }else {
      MemHandle curRec = m_pCursor->skip(delta);
      if (curRec == 0) {
         m_err = DmGetLastErr();
         return EOF;
      }
      m_positionEof = -1;
      m_offset = 0;
      m_size = MemHandleSize(curRec);
      m_totalSize = 0;
      return m_pCursor->currentNum();
   }
}

/*---------------------------------------PalmDbmByteStream::PalmDbmByteStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
PalmDbmByteStream::PalmDbmByteStream(
   char const * pszName,   // DataBase name ("MemoDB")
   unsigned long opflgs,   // open flags
   int cardNo,             // Card # (0)
   UInt32 creator,         // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
   UInt32 type,            // Type (data)
   char const * pCat,      // category
   int sizeIncrt           // incrt size (new records)
) :
   iostream(&m_buf)
{
   if (!m_buf.open(
         pszName,          // DataBase name ("MemoDB")
         opflgs,           // openflags
         cardNo,           // Card # (0)
         creator,          // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
         type,             // Type (data)
         pCat,             // category
         -1,               // rec number to start with
         sizeIncrt         // incrt size (new records)
      )
   ) {
      clear(rdstate() | ios::failbit);
   }
}

/*--------------------------------------PalmDbmByteStream::~PalmDbmByteStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
PalmDbmByteStream::~PalmDbmByteStream() {
   m_buf.close();
}

/*-----------------------------------------PalmDbmRecStream::PalmDbmRecStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
PalmDbmRecStream::PalmDbmRecStream(
   char const * pszName,   // DataBase name ("MemoDB")
   unsigned long opflgs,   // open flags
   int recNo,              // record number
   int cardNo,             // Card # (0)
   UInt32 creator,         // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
   UInt32 type,            // Type (data)
   char const * pCat,      // category
   int sizeIncrt           // incrt size (new records)
) :
   iostream(&m_buf)
{
   if (!m_buf.open(
         pszName,          // DataBase name ("MemoDB")
         opflgs,           // openflags
         cardNo,           // Card # (0)
         creator,          // Creator Id (Appl: 'memo', 'Rexx', sysFile..)
         type,             // Type (data)
         pCat,             // category
         recNo,            // rec number to start with
         sizeIncrt         // incrt size (new records)
      )
   ) {
      clear(rdstate() | ios::failbit);
   }
}

/*----------------------------------------PalmDbmRecStream::~PalmDbmRecStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
PalmDbmRecStream::~PalmDbmRecStream() {
   m_buf.close();
}

/*-------------------------------------------PalmDbmSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * PalmDbmSchemeHandler::Rep::getID() const {
   return "DM";
}

/*-------------------------------------------PalmDbmSchemeHandler::Rep::onNew-+
| This implements the following syntax:                                       |
|                                                                             |
| "dm://"                  required                                           |
| (                                                                           |
|    creator               default is "Rexx"                                  |
|    ("." type)?           default is "data"                                  |
|    "@"                   don't forget the '@'!  <=== WARNING                |
| )?                                                                          |
| palmhostId?              default is "localhost"  FOR FUTURE USE             |
| (":" cardno)?            default is 0                                       |
| "/" dbName               required                                           |
| (                                                                           |
|    "/"                   don't forget the '/'!  <=== WARNING                |
|    category?             default: any category                              |
|    ("/" recordNo)?       numeric: default 1st record  (if any)              |
| )?                                                                          |
|                                                                             |
| Examples:                                                                   |
|   dm://memo.data@localhost:1/MemoDB/Unfiled/123          full URI           |
|   dm:///MemoDB/REXX/12     12th record of category REXX of MemoDB on card 0 |
|   dm://joe@/MemoDB          1st record of MemoDB on card 0 (creator: joe)   |
|   dm://:3/MemoDB//5         5th record of MemoDB on card 3                  |
+----------------------------------------------------------------------------*/
void PalmDbmSchemeHandler::Rep::onNew(URI::Data & uri)
{
   char * pCreator = 0;
   char * pType = 0;
   char * pCategory = 0;
   long recNo = 0;
   unsigned char subCode = 0;

   if (uri.path[0] == '\0') {
      subCode = 1;                // error: missing database name
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
   if (!subCode) {                // so far, so good
      pCategory = strchr(uri.path+1, '/');
      if (pCategory) {
         *pCategory++ = '\0';
         int lenCategory;
         char * pRecNo = strchr(pCategory, '/');
         if (pRecNo) {
            *pRecNo++ = '\0';
            if (!stol((char const **)&pRecNo, &recNo) || (recNo > 0xFFFF)) {
               subCode = 5;       // error: record number too big
            }else if (recNo <= 0) {
               if (*pRecNo == '\0') { // that was an extraneous '/'
                  pRecNo = 0;         // forgive it
               }else {
                  subCode = 6;    // error: record number is zero
               }
            }else if (*pRecNo) {
               subCode = 7;       // error: extraneous data
            }
         }
         lenCategory = strlen(pCategory);
         if (!lenCategory) {
            pCategory = 0;
         }else if (!subCode && (lenCategory >= dmCategoryLength)) {
            subCode = 4;          // error: category too long
         }
      }
   }
   if (subCode != 0) {
      throw URI::MalformedException(URI::HANDLER_ERROR, subCode);
   }else {
      DbmPath * p = new DbmPath;
      if (!pCreator || (p->creator=c2int(pCreator), !p->creator)) {
         p->creator = 'Rexx';
      }
      if (!pType || (p->type=c2int(pType), !p->type)) {
         p->type = 'data';
      }
      p->recNo = recNo;
      p->category = pCategory;
      uri.user = p;
   }
}

/*----------------------------------------PalmDbmSchemeHandler::Rep::onDelete-+
|                                                                             |
+----------------------------------------------------------------------------*/
void PalmDbmSchemeHandler::Rep::onDelete(URI::Data & uri) {
   delete (DbmPath *)uri.user;
}

/*--------------------------------------PalmDbmSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * PalmDbmSchemeHandler::Rep::makeStream(
   URI const & uri, ios__openmode om
) {
   DbmPath const * p = (DbmPath const *)uri.getUser();
   char const * host = uri.getHost();         // For Future Use
   int cardNo = uri.getPort();
   char const * dbName = 1 + uri.getPath();   // hide leading fwd slash

   if (host[0] == '\0') host = "localhost";
   if (cardNo == -1) cardNo = 0;
   return new PalmDbmRecStream(
      dbName,
      (om & ios::in)? dmModeReadOnly : dmModeReadWrite,
      p->recNo,
      cardNo,
      p->creator,
      p->type,
      p->category,
      200        // incrt size (new records)
   );
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
