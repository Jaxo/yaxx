/*
* $Id: entmgr.h,v 1.14 2011-07-29 10:26:39 pgr Exp $
*
* Entity Manager
*/

#if !defined ENTMGR_HEADER && defined __cplusplus
#define ENTMGR_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include <new>
#include "../parser/yspenum.h"
#include "../../toolslib/RWBuffer.h"
#include "../../toolslib/mbstring.h"
#include "../../toolslib/tplistwh.h"
#include "entity.h"

class SgmlDecl;
class EntManager;
class DelimList;
// class istream;
// class ostream;
class UnicodeComposer;
class EntitySolverBase;

/*-----------------------------------------------------------------EntityList-+
| List of all entities declared for this DTD or added on the fly in instance. |
| (3 lists: general, parameter, extended)                                     |
+----------------------------------------------------------------------------*/
class YASP3_API EntityList : public TpListWithHash {  // entlst
public:
   EntityList() {}
   EntityList(EntityList const & source) : TpListWithHash(source) {}
   TP_MakeKeyCollection(Entity);         // a collection of (keyed) Entities
   TP_MakeSequence(Entity);              // ordered
};

/*---------------------------------------------------------------NotationList-+
| List of all notations declared for this DTD or added on the fly in instance.|
+----------------------------------------------------------------------------*/
class YASP3_API NotationList : public TpListWithHash {  // ncblst
public:
   NotationList() {}
   NotationList(NotationList const & source) : TpListWithHash(source) {}
   TP_MakeKeyCollection(Notation);       // a collection of (keyed) Notations
   TP_MakeSequence(Notation);            // ordered
};

/*-----------------------------------------------------------------OpenEntity-+
| Make a unicode stream out of an entity in the entity library,               |
| and keeps state information for it.                                         |
+----------------------------------------------------------------------------*/
class OpenEntity {
   friend class OpenEntityCache;
public:
   OpenEntity(
      Entity const * pEnt,
      istream * pIstProd,
      Encoder & encoder,
      bool doCatchSoftEof,
      ostream * pOstClone
   );
   OpenEntity(
      Entity const * pEnt,
      istream * pIstProd
   );
   ~OpenEntity();
   operator void *() const;
   bool operator!() const;

   int skip();
   int skip(int iToSkip);
   int get();
   int gets(UCS_2 *target, int iSize);
   int peek(int iOffset);
   UCS_2 const * peekChars(int iLen, int iOffset);
   UnicodeString peekNextChars(int iLenMax);
   UnicodeString peekPrevChars(int iLenMax);

   bool isCloned() const;
   void enableClone(bool isToEnable);
   void writeClone(UCS_2 const * pUc, Encoder & enc);
   bool setEncoder(Encoder const & encoder);
   Encoder const * inqEncoder() const;
   Entity const * inqEntity() const;
   int inqOffset() const;
   int inqLine() const;
   int inqColumn() const;
   int inqSize() const;

   streampos inqPos();                                                 /*@016*/
   UnicodeString getInputBackAt(streampos iStartingPos);               /*@016*/

private:
   UnicodeRWBuffer * pUsb;
   RefdKey const entptr;    // Who am (constantly) I?
   istream * eStreamIn;          // entity stream
   ostream * eStreamOut;         // clone stream
   long eOffset;                 // Count of characters read
   int eLine;                    // What line?
   int eCol;                     // What column?
   int iElementDepth;            // At what element depth did I start?
};

/* -- INLINES -- */
inline OpenEntity::operator void *() const {
   if (eStreamIn && !eStreamIn->fail() && pUsb && pUsb->isOk()) {
      return (void *)this;
   }else {
      return 0;
   }
}
inline bool OpenEntity::operator!() const {
   if (!eStreamIn || eStreamIn->fail() || !pUsb || !pUsb->isOk()) {
      return true;
   }else {
      return false;
   }
}
inline int OpenEntity::skip() {
   int iSkipped = pUsb->skip_n_count((UCS_2)'\n', eLine, eCol);
   eOffset += iSkipped;
   return iSkipped;
}
inline int OpenEntity::skip(int iToSkip) {
   int iSkipped = pUsb->skip_n_count(iToSkip, (UCS_2)'\n', eLine, eCol);
   eOffset += iSkipped;
   return iSkipped;
}
inline int OpenEntity::get() {
   int i = pUsb->sgetUc();
   if (i != EOF_SIGNAL) {
      if ((UCS_2)i == (UCS_2)'\n') {
        ++eLine, eCol = 0;
      }else {
        ++eCol;
      }
      ++eOffset;
   }
   return i;
}
inline int OpenEntity::peek(int iOffset) {
   return pUsb->peek(iOffset);
}
inline UCS_2 const * OpenEntity::peekChars(int iLen, int iOffset) {
   return pUsb->str(iLen, iOffset);
}
inline bool OpenEntity::isCloned() const {
   return pUsb->isCloned();
}
inline void OpenEntity::enableClone(bool isToEnable) {
   pUsb->sync();
   pUsb->enableClone(isToEnable);
}
inline void OpenEntity::writeClone(UCS_2 const * pUc, Encoder & enc) {
   pUsb->sync();
   if (eStreamOut) enc.sputnMb(eStreamOut->rdbuf(), pUc, strlength(pUc));
}
inline bool OpenEntity::setEncoder(Encoder const & enc) {
   return pUsb->resetEncoding(enc);
}
inline Encoder const * OpenEntity::inqEncoder() const {
   return pUsb->inqEncoder();
}
inline Entity const * OpenEntity::inqEntity() const {
   return (Entity const *)(entptr.inqData());
}
inline int OpenEntity::inqOffset() const {
   return eOffset;
}
inline int OpenEntity::inqLine() const {
   return eLine;
}
inline int OpenEntity::inqColumn() const {
   return eCol;
}
inline streampos OpenEntity::inqPos() {                                /*@016*/
   return pUsb->seekoff(0, ios::cur, ios::in);
}

/*------------------------------------------------------------OpenEntityCache-+
| Optimization: this cache allows direct access to the open entity.           |
| Restrictions apply to these calls:                                          |
|   - stay in the same entity,                                                |
|   - don't go backward (except backup())                                     |
+----------------------------------------------------------------------------*/
class YASP3_API OpenEntityCache {
public:
   OpenEntityCache(OpenEntity * pOent = 0, int iDepth = 0);
   int get();
   int peek();                      // Peek current
   int peek(int iOffset);           // Peek forward
   void skip();                     // Skip 1, no tests!
   void skip(int iToSkip);          // Skip forward, no tests!
   bool skip(UnicodeString const & ucs); // skip over this string
   void backup(int iToBackup);
   void flush();
   int const iDepth;

   Entity const * inqEntity() const;
   int inqOffset();
   int inqLine();
   int inqColumn();

   streampos inqPos();                                                 /*@016*/
   UnicodeString getInputBackAt(streampos iStartingPos);               /*@016*/

   void setElementDepth(int iDepth);
   int inqElementDepth() const;

private:
   OpenEntity * pOent;
   UnicodeRWBuffer * pUsb;
   int iCachedSkip;
};

/* -- INLINES -- */
inline void OpenEntityCache::flush() {
   if (iCachedSkip) {
      int iSkipped = pUsb->skip_n_count(
        iCachedSkip, (UCS_2)'\n', pOent->eLine, pOent->eCol
      );
      pOent->eOffset += iSkipped;
      iCachedSkip = 0;
   }
}
inline OpenEntityCache::OpenEntityCache(
   OpenEntity * pOentArg, int iDepthArg
) :
   pOent(pOentArg),
   pUsb(pOentArg? pOentArg->pUsb : 0),
   iCachedSkip(0),
   iDepth(iDepthArg)
{
}
inline int OpenEntityCache::get() {
   int i = pUsb->peekForward(iCachedSkip++);
   if (i == EOF_SIGNAL) {              // too much peeks?
      --iCachedSkip;
      flush();
      i = pUsb->peek();                // try again
      if (i != EOF_SIGNAL) iCachedSkip = 1;
   }
   return i;
}
inline int OpenEntityCache::peek() {
   int i = pUsb->peekForward(iCachedSkip);
   if (i == EOF_SIGNAL) {              // too much peeks?
      flush();
      i = pUsb->peek();                // try again
   }
   return i;
}
inline int OpenEntityCache::peek(int iOffset) {
   int i = pUsb->peekForward(iCachedSkip + iOffset);
   if (i == EOF_SIGNAL) {              // too much peeks?
      flush();
      i = pUsb->peek(iOffset);         // try again
   }
   return i;
}
inline void OpenEntityCache::skip() {
   ++iCachedSkip;
}
inline void OpenEntityCache::skip(int iToSkip) {
   iCachedSkip += iToSkip;
}
inline bool OpenEntityCache::skip(UnicodeString const & ucs) {
   int const iLen = ucs.length();
   int iOffset;
   for (iOffset = 0; iOffset < iLen; ++iOffset) {
     if (ucs[iOffset] != peek(iOffset)) return false;
   }
   skip(iOffset);
   return true;
}
inline void OpenEntityCache::backup(int iToBackup) {
   iCachedSkip -= iToBackup;
   if (iCachedSkip < 0) {
      flush();
   }
}
inline Entity const * OpenEntityCache::inqEntity() const {
   return pOent->inqEntity();
}
inline int OpenEntityCache::inqOffset() {
   flush();
   return pOent->eOffset;
}
inline int OpenEntityCache::inqLine() {
   flush();
   return pOent->eLine;
}
inline int OpenEntityCache::inqColumn() {
   flush();
   return pOent->eCol;
}
inline streampos OpenEntityCache::inqPos() {                           /*@016*/
   flush();
   return pOent->inqPos();
}
inline UnicodeString OpenEntityCache::getInputBackAt(                  /*@016*/
   streampos iStartingPos
) {
   flush();
   return pOent->getInputBackAt(iStartingPos);
}
inline void OpenEntityCache::setElementDepth(int iElementDepth) {
   pOent->iElementDepth = iElementDepth;
}
inline int OpenEntityCache::inqElementDepth() const {
   return pOent->iElementDepth;
}

/*-----------------------------------------------------------OpenEntityLinked-+
| Instances of this class constitute a stack of open entities, which          |
| is pushed by EntManager::open, and popped on end-of-entity or directly,     |
| in either case via EntManager::close.                                       |
|                                                                             |
| NOTE: The private pointer pOentlkTop of the EntManager::Stack object        |
|       points to the topmost entity, and the rest of the entities            |
|       are singly-linked from there, out to the document.                    |
+----------------------------------------------------------------------------*/
class OpenEntityLinked : public OpenEntity {
   friend class EntManager;
   friend class EntIterator;

   OpenEntityLinked(
      Entity const * pEnt,
      istream * pIstProd,
      Encoder & encoder,
      bool doCatchSoftEof,
      ostream * pOstClone,
      OpenEntityLinked * pOentlkPrev
   );
   OpenEntityLinked(
      Entity const * pEnt,
      istream * pIstProd,
      OpenEntityLinked * pOentlkPrev
   );
   OpenEntityLinked * pPrev;   // Who referenced me?
};

/*-----------------------------------------------------------------EntManager-+
| EntManager class                                                            |
+----------------------------------------------------------------------------*/
class YASP3_API EntManager {
   friend class EntIterator;

public:
   EntManager(UnicodeComposer & erh, OpenEntityCache * pCacheArg = 0);
   ~EntManager();

   //--- Define to the Entity Library ---
   int defineEntity(                      // 0: error; 1: OK; -1: duplicate;
      Entity const * pEnt
   );
   Entity const * defineEntityDoctype(
      UCS_2 const * pUcDoctypeName = 0,
      UCS_2 const * pUcSystemId = 0,
      UCS_2 const * pUcDelimSystemId = 0,
      UCS_2 const * pUcPublicId = 0,
      UCS_2 const * pUcDelimPublicId = 0
   );
   Entity const * defineEntityDocument(
      UCS_2 const * pUcDoctypeName = 0,
      UCS_2 const * pUcSystemId = 0,
      UCS_2 const * pUcDelimSystemId = 0,
      UCS_2 const * pUcPublicId = 0,
      UCS_2 const * pUcDelimPublicId = 0
   );
   Notation * defineNotation(
      UnicodeString const ucsName,      // Notation name
      ExternId const & exidArg          // External identifier
   );
   Notation * defineNotationIfNotThere(
      UnicodeString const ucsName       // Notation name
   );

   //--- Inquiries from the Entity Library ---
   Entity const * inqEntityParameter(
      UCS_2 const * pUcName,
      bool isReportErrors = false
   ) const;
   Entity const * inqEntityGeneral(
      UCS_2 const * pUcName,
      bool isReportErrors = false
   ) const;
   Entity const * inqEntityGeneralOrExtend(
      UCS_2 const * pUcName,
      bool isReportErrors = false
   );
   Entity const * inqEntityDefault(bool isReportErrors = false) const;
   Entity const * inqEntityDocument(bool isReportErrors = false) const;
   Entity const * inqEntityDoctype(bool isReportErrors = false) const;

   EntityList const & inqListEntityGeneral() const;
   EntityList const & inqListEntityParameter() const;
   NotationList const & inqListNotation() const;

   //--- Inquiries for the current stream ----
   Entity const & inqEntity() const;
   Encoder const & inqEncoderCurrent() const;
   int inqDepth() const;
   int inqLine() const;
   int inqColumn() const;
   long inqOffset() const;
   long inqSize() const;                // this routine is time consuming
   long inqTotalOffset() const;
   bool isCloned() const;
   bool isOpened(Entity const * pEnt, bool isReportErrors = false) const;

   //--- Stream Management ---
   bool openInternal(
      Entity const & ent,
      DelimList const & dlmlst,
      bool isReportErrors = false
   );
   bool openExternal(
      Entity const & ent,
      EntitySolverBase & solver,
      bool isReportErrors = false
   );
   bool close();
   bool closeAll();

   // these functions should be used in new code (NO C api's are provided)
   int peek(int iOffset = 0);  // returns UCS_2 castable int, or EOF_SIGNAL
   int get();                  // returns UCS_2 castable int, or EOF_SIGNAL
   bool skip(int iToSkip);  // skip, closing any opened entity if needed
   bool skip();             // as above for only one character
   int backup(int iToBack);    // skip back

   // get next (iSize-1) characters into target or less if EOF reached. 0 ended
   int gets(UCS_2 *target, int iSize);

   // Peeks at the iLen pending from input stream, without repositioning.
   UCS_2 const * peekChars(int iLen, int iOffset = 0);

   // Peeks at most iLen pending/passed, without repositioning, *nor reading*
   UnicodeString peekNextChars(int iLen = 0);
   UnicodeString peekPrevChars(int iLen = 0);

   //--- Miscellaneous ---
   void freezeFloorDepth();
   void thawFloorDepth();
   int inqFloorDepth() const;
   void enableClone(bool isToEnable = true);
   void writeClone(UCS_2 const * pUc);
   bool setEncodingCurrent(EncodingModule::Id encoding);
   bool setLists(
      EntityList const & entlstGeneralArg,
      EntityList const & entlstParamArg,
      NotationList const & ncblstArg,
      RefdKey const & kiEntDoctypeArg,
      RefdKey const & kiEntDefaultArg
   );
   void extract(Writer & out, SgmlDecl const & sdcl) const;
   bool checkSanity() const;

private:
   UnicodeComposer & erh;              // where to report errors?
   OpenEntityLinked * pOentlkTop;  // top of stack of open entities
   OpenEntityCache * pCache;       // optimizer (not required)
   int iDepth;                     // how many entities are open?
   int iFloorDepth;
   long iTotalOffset;              // total number of characters read
   RefdKey kiEntDefault;
   RefdKey kiEntDocument;
   RefdKey kiEntDoctype;
   EntityList entlstGeneral;
   EntityList entlstParam;
   EntityList entlstExtended;
   NotationList ncblst;

   Encoder encDefault;
   Encoder encCurrent;

   static istream * getInternalProducer(
      Entity const & ent,
      DelimList const & dlmlst
   );
   bool open(
      Entity const & ent,
      istream * pIstProd,               // istream Producer
      bool isReportErrors,
      EncodingModule::Id encoding = EncodingModule::ISO_8859,
      ostream * pOstClone = 0           // ostream clone (if any)
   );
   void setCache();
   void flushCache() const;
   EntManager& operator=(EntManager const& source);          // no!
   EntManager(EntManager const& source);                     // no!
   operator void *() const;                                  // no!
   bool operator!() const;                                // no!
};

/*-- INLINES --*/
inline void EntManager::setCache() {
   if (pCache) new(pCache) OpenEntityCache(pOentlkTop, iDepth);
}
inline void EntManager::flushCache() const {
   if (pCache) pCache->flush();
}
inline Entity const * EntManager::inqEntityDefault(bool) const {
  if (kiEntDefault.isPresent()) return (Entity *)(kiEntDefault.inqData()); else return 0;
}
inline Entity const * EntManager::inqEntityDocument(bool) const {
  if (kiEntDocument.isPresent()) return (Entity *)(kiEntDocument.inqData()); else return 0;
}
inline Entity const * EntManager::inqEntityDoctype(bool) const {
  if (kiEntDoctype.isPresent()) return (Entity *)(kiEntDoctype.inqData()); else return 0;
}
inline EntityList const & EntManager::inqListEntityGeneral() const {
   return entlstGeneral;
}
inline EntityList const & EntManager::inqListEntityParameter() const {
   return entlstParam;
}
inline NotationList const & EntManager::inqListNotation() const {
   return ncblst;
}
inline Entity const & EntManager::inqEntity() const {
   if (pOentlkTop) {
      return * pOentlkTop->inqEntity();
   }else {
      return Entity::Nil;
   }
}
inline Encoder const & EntManager::inqEncoderCurrent() const {
   return encCurrent;
}
inline int EntManager::inqDepth() const {
   return iDepth;
}
inline int EntManager::inqLine() const {
   flushCache();
   return pOentlkTop? pOentlkTop->inqLine() : 0;
}
inline int EntManager::inqColumn() const {
   flushCache();
   return pOentlkTop? pOentlkTop->inqColumn() : 0;
}
inline long EntManager::inqOffset() const {
   flushCache();
   return pOentlkTop? pOentlkTop->inqOffset() : 0;
}
inline long EntManager::inqSize() const {
   return pOentlkTop? pOentlkTop->inqSize() : 0;
}
inline long EntManager::inqTotalOffset() const {
   flushCache();
   return iTotalOffset + inqOffset();
}
inline bool EntManager::isCloned() const {
   if (pOentlkTop) return pOentlkTop->isCloned(); else return false;
}
inline void EntManager::freezeFloorDepth() {
   iFloorDepth = iDepth;
}
inline void EntManager::thawFloorDepth() {
   iFloorDepth = 1;
}
inline int EntManager::inqFloorDepth() const {
   return iFloorDepth;
}
inline int EntManager::peek(int iOffset) {
   flushCache();
   return pOentlkTop->peek(iOffset);
}
inline int EntManager::get()
{
   flushCache();
   int i = pOentlkTop->get();
   // BACKUP NEEDED
   if ((i == EOF_SIGNAL) && (iDepth >= iFloorDepth)) close();
   return i;
}
inline int EntManager::backup(int iToBack) {
   flushCache();
   return -pOentlkTop->skip(-iToBack);
}
inline int EntManager::gets(UCS_2 *target, int iSize) {
   flushCache();
   return pOentlkTop->gets(target, iSize);
}
inline UCS_2 const * EntManager::peekChars(int iLen, int iOffset) {
   flushCache();
   if (pOentlkTop) return pOentlkTop->peekChars(iLen, iOffset); else return 0;
}
inline UnicodeString EntManager::peekNextChars(int iLenMax) {
   flushCache();
   if (pOentlkTop) {
      return pOentlkTop->peekNextChars(iLenMax);
   }else {
      return UnicodeString::Nil;
   }
}
inline UnicodeString EntManager::peekPrevChars(int iLenMax)
{
   flushCache();
   if (pOentlkTop) {
      return pOentlkTop->peekPrevChars(iLenMax);
   }else {
      return UnicodeString::Nil;
   }
}
inline void EntManager::enableClone(bool isToEnable) {
   flushCache();
   if (pOentlkTop) pOentlkTop->enableClone(isToEnable);
}
inline void EntManager::writeClone(UCS_2 const * pUc) {
   flushCache();
   pOentlkTop->writeClone(pUc, encCurrent);
}
/*----------------------------------------------------------------EntIterator-+
| Iterator within the stack of open entities.                                 |
| Allows safe access to the data contained in a Entity object.                |
+----------------------------------------------------------------------------*/
class EntIterator {
public:
   EntIterator(EntManager const * theEm);
   OpenEntity const * top();
   OpenEntity const * previous();

   operator void *() const;
   bool operator!() const;
   Entity const & inqEntity() const;
   UCS_2 const * inqName() const;
   int inqLine() const;
   int inqColumn() const;
   long inqOffset() const;
   long inqSize() const;

private:
   OpenEntityLinked const * const pOentlkTop;
   OpenEntityLinked const * pOentCurr;  // open entity currently iterated
   int iDepth;                          // current depth - #PRIMARY is 1;

   EntIterator& operator=(EntIterator const& source);          // no!
   EntIterator(EntIterator const& source);                     // no!
};

/* -- INLINES -- */
inline EntIterator::EntIterator(EntManager const * theEm) :
   pOentlkTop(theEm? theEm->pOentlkTop : 0),
   pOentCurr(pOentlkTop),
   iDepth(theEm? theEm->inqDepth() : 0)
{
   theEm->flushCache();
}
inline OpenEntity const * EntIterator::top() {
   if (pOentCurr != pOentlkTop) {
      iDepth = 0;
      pOentCurr = pOentlkTop;
      for (OpenEntityLinked const * p = pOentlkTop; p; p = p->pPrev) {
         ++iDepth;
      }
   }
   return pOentCurr;
}
inline OpenEntity const * EntIterator::previous() {
   if (iDepth) {
      --iDepth;
      pOentCurr = pOentCurr->pPrev;
   }
   return pOentCurr;
}
inline EntIterator::operator void *() const {
   if (pOentlkTop) return (void *)this; else return 0;
}
inline bool EntIterator::operator!() const {
   if (pOentlkTop) return false; else return true;
}
inline Entity const & EntIterator::inqEntity() const {
   if (pOentCurr) return *(pOentCurr->inqEntity()); else return Entity::Nil;
}
inline UCS_2 const * EntIterator::inqName() const {
   if (pOentCurr) return pOentCurr->inqEntity()->key(); else return 0;
}
inline int EntIterator::inqLine() const {
   if (pOentCurr) return pOentCurr->inqLine(); else return 0;
}
inline int EntIterator::inqColumn() const {
   if (pOentCurr) return pOentCurr->inqColumn(); else return 0;
}
inline long EntIterator::inqOffset() const {
   if (pOentCurr) return pOentCurr->inqOffset(); else return 0;
}
inline long EntIterator::inqSize() const {
   if (pOentCurr) return pOentCurr->inqSize(); else return 0;
}

#endif /* ENTMGR_HEADER =====================================================*/
