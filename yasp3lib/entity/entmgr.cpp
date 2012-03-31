/*
* $Id: entmgr.cpp,v 1.12 2002-04-16 17:06:09 jlatone Exp $
*
*/

/*--------------+
| Include Files |
+--------------*/
#include <assert.h>
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"
#include "../syntax/sdcl.h"
#include "entsolve.h"
#include "entstrea.h"
#include "entmgr.h"

/*-----------------------------------------------------OpenEntity::OpenEntity-+
|                                                                             |
+----------------------------------------------------------------------------*/
OpenEntity::OpenEntity(
   Entity const * pEnt,
   istream * pIstProd,
   Encoder & encoder,
   bool doCatchSoftEof,
   ostream * pOstClone
) :
   entptr(pEnt),
   eStreamIn(pIstProd),
   eStreamOut(pOstClone),
   eOffset(0),                                // offset starts at 0
   eLine(1),                                  // lines, however, count from 1
   eCol(0),
   iElementDepth(0)
{
   pUsb = new MultibyteRWBuffer(
      pIstProd->rdbuf(), encoder, doCatchSoftEof, pOstClone
   );
}

OpenEntity::OpenEntity(                       // UCS encoding
   Entity const * pEnt,
   istream * pIstProd
) :
   entptr(pEnt),
   eStreamIn(pIstProd),
   eStreamOut(0),
   eOffset(0),                                // offset starts at 0
   eLine(1),                                  // lines, however, count from 1
   eCol(0),
   iElementDepth(0)
{
   pUsb = new UnicodeRWBuffer(pIstProd->rdbuf());
}

/*----------------------------------------------------OpenEntity::~OpenEntity-+
|                                                                             |
+----------------------------------------------------------------------------*/
OpenEntity::~OpenEntity()
{
   delete pUsb;
   delete eStreamIn;
   delete eStreamOut;
}

/*-----------------------------------------------------------OpenEntity::gets-+
| Effects:                                                                    |
|    Get next (iSize-1) UC characters into target or less if EOF reached.     |
|    Add an ending 0.                                                         |
| Returns:                                                                    |
|    The actual length (not including ending zero).                           |
+----------------------------------------------------------------------------*/
int OpenEntity::gets(UCS_2 *target, int iSize)
{
   if (iSize < 1) {             // defense!
      if (iSize <= 0) return 0;
      iSize = 0;
   }else {
      --iSize;
      if (EOF_SIGNAL == pUsb->peek(iSize-1)) {
         assert  (pUsb->in_avail() <= iSize);
         iSize = pUsb->in_avail();
      }
      UCS_2 const * pUcBuf = pUsb->str(iSize);
      if (pUcBuf) {
         memcpy(target, pUcBuf, iSize * sizeof (UCS_2));
      }else {
         iSize = 0;
      }
   }
   target[iSize] = 0;
   return iSize;
}

/*--------------------------------------------------OpenEntity::peekNextChars-+
| Effects:                                                                    |
|   Returns at most iLenMax characters pending from input stream, without     |
|   repositioning, nor reading.  iLenMax == 0 means: grab all what you can.   |
+----------------------------------------------------------------------------*/
UnicodeString OpenEntity::peekNextChars(int iLenMax)
{
   int const iLenAvail = pUsb->in_avail();
   if ((!iLenMax) || (iLenMax > iLenAvail)) {
      iLenMax = iLenAvail;             // Grab what fits
   }
   return UnicodeString(pUsb->str(iLenMax, 0), iLenMax);
}

/*--------------------------------------------------OpenEntity::peekPrevChars-+
| Effects:                                                                    |
|   Returns at most the iLenMax last chars read from input stream, without    |
|   repositioning.  iLenMax == 0 means: grab all what you can.                |
+----------------------------------------------------------------------------*/
UnicodeString OpenEntity::peekPrevChars(int iLenMax)
{
   int const iLenAvail = pUsb->in_back();
   if ((!iLenMax) || (iLenMax > iLenAvail)) {
      iLenMax = iLenAvail;             // Grab what fits
   }
   return UnicodeString(pUsb->str(iLenMax, -iLenMax), iLenMax);
}

/*--------------------------------------------------------OpenEntity::inqSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int OpenEntity::inqSize() const
{
   streampos const posCurrent = eStreamIn->tellg();
   eStreamIn->seekg(0, ios::end);
   streampos const posEnd = eStreamIn->tellg();
   eStreamIn->seekg(posCurrent, ios::beg);
   return (int)posEnd;
}

/*-------------------------------------------------OpenEntity::getInputBackAt-+
| Make a UnicodeString with current stream, starting back at iStartingPos     |
+----------------------------------------------------------------------------*/
UnicodeString OpenEntity::getInputBackAt(streampos iStartingPos)
{
   UnicodeString ucsResult;
   if ((int)iStartingPos == EOF_SIGNAL) {
      return ucsResult;
   }
   pUsb->sync();
   streampos iCurrentPos = eStreamIn->tellg();
   if (!eStreamIn->seekg(iStartingPos).fail()) {
      int const iMbLen = (int)iCurrentPos - (int)iStartingPos;
      if (pUsb->inqEncoder()) {     // true multibytes
         char * achBuf = new char[iMbLen];
         if (
            #ifdef _WIN32  // Visual C++ bug fix
               ansiRead(*eStreamIn, achBuf, iMbLen)
            #else
               !eStreamIn->read(achBuf, iMbLen).fail()
            #endif
         ) {
            ucsResult = UnicodeString(achBuf, *(pUsb->inqEncoder()), iMbLen);
         }
         delete [] achBuf;
      }else {                      // already unicode!
         int const iUcLen = (iMbLen+1)/(sizeof(UCS_2)); // round up
         UCS_2 * aucBuf = new UCS_2[iUcLen];
         if (
            #ifdef _WIN32  // Visual C++ bug fix
               ansiRead(*eStreamIn, (char *)aucBuf, iMbLen)
            #else
               !eStreamIn->read((char *)aucBuf, iMbLen).fail()
            #endif
         ) {
            ucsResult = UnicodeString(aucBuf, iUcLen);
         }
         delete [] aucBuf;
      }
   }
   eStreamIn->seekg(iCurrentPos);
   return ucsResult;
}

/*-----------------------------------------OpenEntityLinked::OpenEntityLinked-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline OpenEntityLinked::OpenEntityLinked(
   Entity const * pEnt,
   istream * pIstProd,
   Encoder & encoder,
   bool doCatchSoftEof,
   ostream * pOstClone,
   OpenEntityLinked * pOentlkPrev
) :
   OpenEntity(pEnt, pIstProd, encoder, doCatchSoftEof, pOstClone),
   pPrev(pOentlkPrev)
{
}

inline OpenEntityLinked::OpenEntityLinked(
   Entity const * pEnt,
   istream * pIstProd,
   OpenEntityLinked * pOentlkPrev
) :
   OpenEntity(pEnt, pIstProd),
   pPrev(pOentlkPrev)
{
}

/*-----------------------------------------------------EntManager::EntManager-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
EntManager::EntManager(
   UnicodeComposer & erhArg,
   OpenEntityCache * pCacheArg
) :
   erh(erhArg),
   pCache(pCacheArg),
   iDepth(0),
   iFloorDepth(1),          // cannot normally under this depth
   pOentlkTop(0),
   iTotalOffset(0)          // Track total doc size
{

   EntityFlag bFlag;
   bFlag.external = 1;
   bFlag.rcdata = 1;
   bFlag.specialDoc = 1;
   kiEntDocument = new EntityExt(
      UnicodeString::Nil, bFlag, ENTK_SGML, ExternId::Nil, DataNotation::Nil
   );
   bFlag.specialDoc = 0;
   bFlag.specialDtd = 1;
   kiEntDoctype =  new EntityExt(
      UnicodeString::Nil, bFlag, ENTK_SGML, ExternId::Nil, DataNotation::Nil
   );
}

/*----------------------------------------------------EntManager::~EntManager-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
EntManager::~EntManager()
{
   while (pOentlkTop) {
      OpenEntityLinked * pOentlkToClose = pOentlkTop;
      pOentlkTop = pOentlkTop->pPrev;
      delete pOentlkToClose;
   }
   iDepth = 0;
}

/*---------------------------------------------------EntManager::defineEntity-+
| Effects:                                                                    |
|    Add an internal entity to the general or parameter entity library.       |
|    Warning: this can *not* be used for special entities (doc/dtd)           |
|                                                                             |
| Returns:                                                                    |
|    1  if the entity was properly inserted                                   |
|    -1 if the entity is a duplicate                                          |
|    0  if the entity cannot be inserted (error)                              |
|                                                                             |
| According to section 10.5.1 of the SGML standard, it is an ignored          |
| non-error to redefine an entity.                                            |
| Note: if the entity cannot be inserted, it is (magically) deleted.          |
|                                                                             |
+----------------------------------------------------------------------------*/
int EntManager::defineEntity(Entity const * pEntNew)
{
   if (pEntNew) {
      assert  (!(pEntNew->isSpecialDoc() || pEntNew->isSpecialDtd()));
      RefdKey kiEntNew(pEntNew);
      if (pEntNew->isDefault()) {
         if (!kiEntDefault.isPresent()) {
            kiEntDefault = kiEntNew;
            return 1;
         }
      }else if (pEntNew->isParameter()) {
         if (entlstParam[pEntNew->key()]) return -1;    // duplicate
         if (entlstParam.insert(pEntNew)) return 1;     // ok
      }else {
         if (entlstGeneral[pEntNew->key()]) return -1;  // duplicate
         if (entlstGeneral.insert(pEntNew)) return 1;   // ok
      }
   }
   return 0;
}

/*--------------------------------------------EntManager::defineEntityDoctype-+
| Define the DTD entity.  Default unset values to the current DTD entity, if  |
| such entity exists.                                                         |
|                                                                             |
| Special entities however are allowed to be redefined: it's intended to      |
| permit loading of successive documents.                                     |
+----------------------------------------------------------------------------*/
Entity const * EntManager::defineEntityDoctype(
   UCS_2 const * pUcDoctypeName,
   UCS_2 const * pUcSystemId,
   UCS_2 const * pUcDelimSystemId,
   UCS_2 const * pUcPublicId,
   UCS_2 const * pUcDelimPublicId
) {
   EntityFlag bFlag;
   bFlag.external = 1;
   bFlag.rcdata = 1;
   bFlag.specialDtd = 1;
   Entity * pEntDoctype;
   if (kiEntDoctype.isPresent()) {
      pEntDoctype = (Entity *)kiEntDoctype.inqData();
      if (!pUcDoctypeName) {
         pUcDoctypeName = pEntDoctype->key();
      }
      if (!pUcSystemId) {
         pUcSystemId = pEntDoctype->inqExternId().inqSystemId();
         pUcDelimSystemId = pEntDoctype->inqExternId().inqDelimSystemId();
      }
      if (!pUcPublicId) {
         pUcPublicId = pEntDoctype->inqExternId().inqPublicId();
         pUcDelimPublicId = pEntDoctype->inqExternId().inqDelimPublicId();
      }
   }
   pEntDoctype = new EntityExt(
      pUcDoctypeName, bFlag, ENTK_SGML,
      ExternId(
         pUcSystemId, pUcDelimSystemId,
         pUcPublicId, pUcDelimPublicId,
         ExternId::INVALID            // does mean: irrelevant
      ),
      DataNotation::Nil
   );
   if (pEntDoctype) kiEntDoctype = pEntDoctype;
   return pEntDoctype;
}

/*-------------------------------------------EntManager::defineEntityDocument-+
| Define the DOCUMENT entity.  Reset all values.                              |
+----------------------------------------------------------------------------*/
Entity const * EntManager::defineEntityDocument(
   UCS_2 const * pUcDocumentName,
   UCS_2 const * pUcSystemId,
   UCS_2 const * pUcDelimSystemId,
   UCS_2 const * pUcPublicId,
   UCS_2 const * pUcDelimPublicId
) {
   EntityFlag bFlag;
   bFlag.external = 1;
   bFlag.rcdata = 1;
   bFlag.specialDoc = 1;
   Entity * pEntDocument = new EntityExt(
      pUcDocumentName, bFlag, ENTK_SGML,
      ExternId(
         pUcSystemId, pUcDelimSystemId,
         pUcPublicId, pUcDelimPublicId,
         ExternId::INVALID                  // does mean: irrelevant
      ),
      DataNotation::Nil
   );
   if (pEntDocument) kiEntDocument = pEntDocument;
   return pEntDocument;
}

/*-------------------------------------------------EntManager::defineNotation-+
| Effects:                                                                    |
|    Add a notation to the notation list.                                     |
|                                                                             |
| Notes:                                                                      |
|   defineNotation is called:                                                 |
|                                                                             |
|   1) When a NOTATION is defined.                                            |
|      If the notation name has already been seen AND if it was because of    |
|      another NOTATION decl, then scream DUPLICATE_NOTATION                  |
|                                                                             |
|   2) To create a temporary place holder (by defineNotationIfNotThere)       |
|      This is a recovery routine when DataNotation is found before the       |
|      notation is declared.                                                  |
|                                                                             |
| Returns:                                                                    |
|    Notation pointer or 0 if unable to define                                |
|                                                                             |
+----------------------------------------------------------------------------*/
Notation * EntManager::defineNotation(
   UnicodeString const ucsName,      // Notation name
   ExternId const & exid             // External identifier
) {
   Notation * pNcb = ncblst[ucsName];
   if (pNcb) {
      if (pNcb->inqExternId()) {
         erh << ECE__ERROR << _ENT__DUPNOTNAME << ucsName << endm;
      }else {
         pNcb->setExternId(exid);
      }
   }else {
      pNcb = new Notation(ucsName, exid);
      if (!ncblst.insert(pNcb)) return 0;
   }
   return pNcb;
}

/*---------------------------------------EntManager::defineNotationIfNotThere-+
| Effects:                                                                    |
|    Add a notation to the notation list if it doesn't already exist.         |
|                                                                             |
| Returns:                                                                    |
|    Entity pointer or 0 if unable to define                                  |
+----------------------------------------------------------------------------*/
Notation * EntManager::defineNotationIfNotThere(
   UnicodeString const ucsName       // Notation name
) {
   Notation * pNcb = ncblst[ucsName];
   if (!pNcb) {
      pNcb = new Notation(ucsName, ExternId::Nil);
      if (!ncblst.insert(pNcb)) return 0;
   }
   return pNcb;
}

/*---------------------------------------------EntManager::inqEntityParameter-+
| Effects:                                                                    |
|   Find a parameter entity                                                   |
| Returns:                                                                    |
|  Pointer to the Entity object, or 0 if not found                            |
+----------------------------------------------------------------------------*/
Entity const * EntManager::inqEntityParameter(
   UCS_2 const * pUcName, bool isReportErrors
) const {
   Entity const * pEnt = entlstParam[pUcName];
   if (!pEnt && isReportErrors) {
      erh << ECE__ERROR << _ENT__NOENTPARDEF << pUcName << endm;
   }
   return pEnt;
}

/*------------------------------------------------EntManager::inqEntityGeneral-+
| Effects:                                                                     |
|   Find a general entity                                                      |
| Returns:                                                                     |
|  Pointer to the Entity object, or 0 if not found                             |
+-----------------------------------------------------------------------------*/
Entity const * EntManager::inqEntityGeneral(
   UCS_2 const * pUcName, bool isReportErrors
) const {
   Entity const * pEnt = entlstGeneral[pUcName];
   if (!pEnt) {
      pEnt = inqEntityDefault();
   }
   if (!pEnt && isReportErrors) {
      erh << ECE__ERROR << _ENT__NOENTGENDEF << pUcName << endm;
   }
   return pEnt;
}

/*---------------------------------------EntManager::inqEntityGeneralOrExtend-+
| Effects:                                                                    |
|   Find a general entity.                                                    |
|   If not found, create one in the extended list, so we don't moan each time.|
|   If later the entity is really defined, it will go in the regular list.    |
| Note:                                                                       |
|   General entity references are allowed in prolog for attribute value       |
|   literal                                                                   |
| Returns:                                                                    |
|   Pointer to the Entity structure, or 0 if troubles.                        |
+----------------------------------------------------------------------------*/
Entity const * EntManager::inqEntityGeneralOrExtend(
   UCS_2 const * pUcName, bool isReportErrors
) {
   Entity * pEnt = entlstGeneral[pUcName];
   if ((!pEnt) && (pEnt = entlstExtended[pUcName], !pEnt)) {
      Entity const * pEntDefault = inqEntityDefault();
      if (pEntDefault) {
         if (pEntDefault->isExternal()) {
            pEnt = new EntityExt(
               pUcName, pEntDefault->inqFlag(), pEntDefault->inqKind(),
               pEntDefault->inqExternId(), pEntDefault->inqDataNotation()
            );
         }else {
            pEnt = new EntityInt(
               pUcName, pEntDefault->inqFlag(), pEntDefault->inqKind(),
               pEntDefault->inqLiteral(), pEntDefault->inqDelim()
            );
         }
      }else {
         if (isReportErrors) {
            erh << ECE__ERROR << _ENT__NOENTGENDEF << pUcName << endm;
         }
         EntityFlag flag;
         flag.rcdata = 1;
         pEnt = new Entity(pUcName, flag, ENTK_INVALID);
      }
      if (!entlstExtended.insert(pEnt)) pEnt = 0;
   }
   return pEnt;
}

/*-------------------------------------------------------EntManager::isOpened-+
| Test if an entity is already in the stack (recursive invocation)            |
+----------------------------------------------------------------------------*/
bool EntManager::isOpened(
   Entity const * pEnt,
   bool isReportErrors
) const {
   for (OpenEntityLinked * p = pOentlkTop; p; p = p->pPrev) {
      if (p->inqEntity() == pEnt) {
         if (isReportErrors) {
            erh << ECE__ERROR << _ENT__INVRECURSE << pEnt->inqName() << endm;
         }
         return true;
      }
   }
   return false;
}

/*--------------------------------------------EntManager::getInternalProducer-+
| Effects:                                                                    |
|    Get an appropriate istream to read an internal entity.                   |
| Returns:                                                                    |
|    Pointer to the stream or 0                                               |
+----------------------------------------------------------------------------*/
istream * EntManager::getInternalProducer(
   Entity const & ent,
   DelimList const & dlmlst
) {
   istream * pIst = 0;

   switch(ent.inqKind()) {
   case ENTK_STARTTAG:
      pIst =  new EntStreamMemBracketed(
         dlmlst[Delimiter::IX_STAGO],
         ent.inqLiteral(),
         dlmlst[Delimiter::IX_TAGC]
      );
      break;
   case ENTK_ENDTAG:
      pIst =  new EntStreamMemBracketed(
         dlmlst[Delimiter::IX_ETAGO],
         ent.inqLiteral(),
         dlmlst[Delimiter::IX_TAGC]
      );
      break;
   case ENTK_MS:
      pIst =  new EntStreamMemBracketed(
         dlmlst[Delimiter::IX_MDO_DSO],
         ent.inqLiteral(),
         dlmlst[Delimiter::IX_MSC_MDC]
      );
      break;
   case ENTK_MD:
      pIst =  new EntStreamMemBracketed(
         dlmlst[Delimiter::IX_MDO],
         ent.inqLiteral(),
         dlmlst[Delimiter::IX_MDC]
      );
      break;
   default:
      pIst = new EntStreamMem(ent.inqLiteral());
      break;
   }
   return pIst;
}

/*-----------------------------------------------------------EntManager::open-+
| Effects:                                                                    |
|    Open an entity, then push the stack of open entities                     |
+----------------------------------------------------------------------------*/
bool EntManager::openInternal(
   Entity const & ent,
   DelimList const & dlmlst,
   bool isReportErrors
) {
   assert  (ent.isInternal());
   return open(
      ent,
      getInternalProducer(ent, dlmlst),
      isReportErrors
   );
}

bool EntManager::openExternal(
   Entity const & ent,
   EntitySolverBase & solver,
   bool isReportErrors
) {
   assert  (ent.isExternal());
   return open(
      ent,
      solver.getExternalProducer(ent),
      isReportErrors,
      solver.inqEncoding(ent),
      solver.getClone(ent)
   );
}

bool EntManager::open(
   Entity const & ent,
   istream * pIstProd,               // istream Producer
   bool isReportErrors,
   EncodingModule::Id encoding,
   ostream * pOstClone               // ostream clone (if any)
) {
   if (!pIstProd || pIstProd->fail() || isOpened(&ent, isReportErrors)) {
      if (isReportErrors) {
         if (ent.isSpecialDoc()) {
            erh << _ENT__BADOPENDOC;
         }else {
            if (ent.isSpecialDtd()) {
               erh << _ENT__BADOPENDTD;
            }else {
               erh << _ENT__BADOPEN;
            }
            erh << ent.inqName();
         }
         erh << ECE__ERROR << endm;
      }
      delete pIstProd;
      delete pOstClone;
      return false;
   }
   if (pOentlkTop) {
      flushCache();
      iTotalOffset += pOentlkTop->inqOffset();
   }
   if (ent.isExternal()) {
      // even if an encoding was found, we need to eat the BOM, if any
      EncodingModule::Id encodingFromStream = Encoder::interpretByteMark(
         pIstProd->rdbuf(), "<?"
      );
      if (encoding) {
         if (
            (encoding == EncodingModule::UCS_2S) &&
            (encodingFromStream == EncodingModule::UCS_2R)
         ) {
            encCurrent = EncodingModule::UCS_2R;
         }else {
            encCurrent = encoding;
         }
      }else if (encodingFromStream == EncodingModule::ISO_8859) {
         encCurrent = encDefault;
      }else if (encodingFromStream) {
         encCurrent = encodingFromStream;
      }
      // else, inherit encoding from father entity
      pOentlkTop = new OpenEntityLinked(
         &ent,
         pIstProd,
         encCurrent,
         true,                       // do catch soft Eof
         pOstClone,
         pOentlkTop
      );
   }else {
      pOentlkTop = new OpenEntityLinked(&ent, pIstProd, pOentlkTop); // UCS
   }
   assert (pOentlkTop);
   ++iDepth;
   setCache();
   return true;
}

/*----------------------------------------------------------EntManager::close-+
| Effects:                                                                    |
|  - pop the stack of opened entities                                         |
|  - reset "encCurrent" for the possibly nested entities -- see comments in   |
|    setEncodingCurrent()                                                     |
|  - close the current entity                                                 |
+----------------------------------------------------------------------------*/
bool EntManager::close()
{
   OpenEntityLinked * pOentlkToClose = pOentlkTop;
   assert (pOentlkToClose && (iDepth > 0));

   --iDepth;
   flushCache();
   iTotalOffset += pOentlkToClose->inqOffset();
   pOentlkTop = pOentlkTop->pPrev;
   if (pOentlkTop) iTotalOffset -= pOentlkTop->inqOffset();
   setCache();
   delete pOentlkToClose;

   for (OpenEntityLinked * pOentlk = pOentlkTop; ; pOentlk = pOentlk->pPrev) {
      if (!pOentlk) {
         encCurrent = encDefault;
         break;
      }
      Encoder const * pEnc = pOentlk->inqEncoder();
      if (pEnc) {
         encCurrent = *pEnc;
         break;
      }
   }
   return true;
}

/*-------------------------------------------------------EntManager::closeAll-+
| Effects:                                                                    |
|    Force all open entities closed, including the document instance.         |
| Returns:                                                                    |
|    Success flag.                                                            |
+----------------------------------------------------------------------------*/
bool EntManager::closeAll()
{
   while (iDepth) close();
   iTotalOffset = 0;
   iFloorDepth = 1;
   return true;
}

/*-----------------------------------------------------------EntManager::skip-+
| Effects:                                                                    |
|    Skip 'iToSkip' Unicode characters, closing as many entities as needed    |
| Returns:                                                                    |
|    true if the character was skipped.  false if end of document reached.    |
+----------------------------------------------------------------------------*/
bool EntManager::skip(int iToSkip)
{
   flushCache();
   while (iDepth) {
      int iSkipped = pOentlkTop->skip(iToSkip);
      if (iSkipped == iToSkip) {
         return true;
      }
      if (iDepth <= iFloorDepth) return false;
      iToSkip -= iSkipped;
      close();
      if (!--iToSkip) return true;  // consider EOF_SIGNAL as a character
   }
   return false;
}

/*-----------------------------------------------------------EntManager::skip-+
| Same as above for only one character                                        |
+----------------------------------------------------------------------------*/
bool EntManager::skip()
{
   if (iDepth) {
      flushCache();
      int iSkipped = pOentlkTop->skip();
      if (!iSkipped) {
         if (iDepth <= iFloorDepth) return false;
         close();
      }
      return true;
   }
   return false;
}

/*---------------------------------------------EntManager::setEncodingCurrent-+
| Effects:                                                                    |
|   If an entity is currently opened, and if this entity is external,         |
|   do an immediate reset of the codeset for the rest of this entity          |
|   (including the bytes still in the read buffer, and not yet parsed.)       |
|                                                                             |
|   Set the codeset for any entity nested within the current entity,          |
|   or within the main document if nothing has yet been opened.               |
|                                                                             |
| Implementation Notes:                                                       |
|   This call sets the value of the "encCurrent" encoder.                     |
|   "encCurrent" is the encoder used when opening an external entity.         |
|   When an external entity is closed, "encCurrent" is reset to the           |
|   value of the most recently stacked external entity.                       |
|   If none, "entCurrent" gets the value of "entDefault".                     |
|   "entDefault" is either:                                                   |
|      - the encoding set before the main document started -- i.e:            |
|        by a call to setEncodingCurrent while no entity was opened           |
|      - LOCALE, if "setEncodingCurrent" wasn't call at the time the          |
|        parse of the main document started.                                  |
+----------------------------------------------------------------------------*/
bool EntManager::setEncodingCurrent(EncodingModule::Id encoding)
{
   encCurrent = encoding;
   if (!pOentlkTop) {
      encDefault = encCurrent;   // re-activated after the doc ends
   }else {
      if (
         (pOentlkTop->inqEntity()->isExternal()) &&
         (flushCache(), !pOentlkTop->setEncoder(encCurrent))
      ) {
         return false;
      }
   }
   return true;
}

/*-------------------------------------------------------EntManager::setLists-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool EntManager::setLists(
   EntityList const & entlstGeneralArg,
   EntityList const & entlstParamArg,
   NotationList const & ncblstArg,
   RefdKey const & kiEntDoctypeArg,
   RefdKey const & kiEntDefaultArg
) {
   entlstGeneral = entlstGeneralArg;
   entlstParam = entlstParamArg;
   ncblst = ncblstArg;
   kiEntDoctype = kiEntDoctypeArg;
   kiEntDefault = kiEntDefaultArg;
   return true;
}

/*--------------------------------------------------------EntManager::extract-+
|                                                                             |
+----------------------------------------------------------------------------*/
void EntManager::extract(Writer & out, SgmlDecl const & sdcl) const
{
   UnicodeString const & ucsSpace = sdcl.charset().inqUcSpace(); // = for MSVC
   UCS_2 pUcSeparator[3];  // RS(nl) for UNIX, RE(cr)/RS(lf) for DOS
   #ifdef unix
      pUcSeparator[0] = sdcl.charset().RS();
      pUcSeparator[1] = 0;
   #else
      pUcSeparator[0] = sdcl.charset().RE();
      pUcSeparator[1] = sdcl.charset().RS();
      pUcSeparator[2] = 0;
   #endif

   if (inqEntityDoctype()) {
      inqEntityDoctype()->extract(out, sdcl);
   }else {
      out << sdcl.delimList()[Delimiter::IX_MDO]
          << sdcl.reservedName()[ReservedName::IX_DOCTYPE]
          << ucsSpace << "unknown";
   }
   out << ucsSpace << sdcl.delimList()[Delimiter::IX_DSO] << pUcSeparator;

   if (inqEntityDefault()) {
       inqEntityDefault()->extract(out, sdcl);
       out << pUcSeparator;
   }
   int const iLast = entlstGeneral.count();
   for (int ix = 0; ix < iLast; ++ix) {
      Entity const * pEnt = entlstGeneral[ix];
      if (pEnt) {
         pEnt->extract(out, sdcl);
         out << pUcSeparator;
      }
   }
}

/*----------------------------------------------------EntManager::checkSanity-+
| Effects:                                                                    |
|    Test the data structures and return whether they seem ok.                |
| Returns:                                                                    |
|    true if everything checks out o.k.                                       |
+----------------------------------------------------------------------------*/
bool EntManager::checkSanity() const
{
   OpenEntityLinked * pOentlk = pOentlkTop;
   int i = 0;

   while (i++ < iDepth) {
      if (!pOentlk) {
         return false;
      }
      pOentlk = pOentlk->pPrev;
   }
   if (pOentlk) {
      return false;
   }
   return true;
}
/*===========================================================================*/
