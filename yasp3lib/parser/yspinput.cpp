/*
* $Id: yspinput.cpp,v 1.8 2002-02-28 10:17:17 pgr Exp $
*
* Internal routines to manage input from the SGML document entity and
* any embedded entities
*/

/*---------+
| Includes |
+---------*/
#include "../yasp3.h"
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"
#include "../entity/entsolve.h"

/*-----------------------------------------------------------------------open-+
| Effects:                                                                    |
|    Open any entity                                                          |
| Returns:                                                                    |
|    Success flag.                                                            |
+----------------------------------------------------------------------------*/
bool Yasp3::open(Entity const * pEnt, bool isReportErrors)
{
   if (!pEnt) {
      return false;
   }else if (pEnt->isExternal()) {
      return entMgr.openExternal(*pEnt, *pSolver, isReportErrors);
   }else {
      return entMgr.openInternal(*pEnt, sdcl.delimList(), isReportErrors);
   }
}

/*---------------------------------------------------------------openDocument-+
| Effects:                                                                    |
|    Start up a new document file.  Called after YSPnew sets up the syntax    |
|    and basic data structures, to actually begin parsing a file              |
|    (said file may begin with a DTD, which may change the syntax).           |
|    The outermost file is not automatically closed, hence closeDocument.     |
| Returns:                                                                    |
|    Success flag.                                                            |
+----------------------------------------------------------------------------*/
bool Yasp3::openDocument(EntitySolverBase & solverArg)
{
   if (!bParserOk || (docIsOpen && bMoreToParse)) {
      return false;
   }
   pSolver = &solverArg;
   Entity const * pEnt = entMgr.defineEntityDocument(
      docEntName, 0, value(Delimiter::IX_LITA)
   );
   if (!pEnt || !entMgr.openExternal(*pEnt, *pSolver)) {
      return false;
   }
   docIsOpen = true;
   bMoreToParse = true;
   evlst += YSP__PARSE_START;
   return true;
}

/*----------------------------------------------------------openDtdAsDocument-+
| This calls permits to open a single DTD as the main document.               |
| Neither DTD start, nor DTD_REQUEST will be issued.                          |
| However, DTD_END is issued.                                                 |
+----------------------------------------------------------------------------*/
bool Yasp3::openDtdAsDocument(EntitySolverBase & solverArg)
{
   if (!openDocument(solverArg)) {
      return false;
   }
   return true;
}

/*-------------------------------------------------------parseEntityReference-+
| Effects:                                                                    |
|   Parse an entity reference, indicates how much was parsed.                 |
| Returns:                                                                    |
|   Pointer to the dereferenced entity, or 0 if failed.                       |
+----------------------------------------------------------------------------*/
Entity const * Yasp3::parseEntityReference(bool isGeneral, int & iLen)
{
   UCS_2 entName[1+NAMELEN_MAX];
   iLen = peekRefc(peekName(entName, sdcl.charset().isCaseEntity()));
   if (isGeneral) {
      return entMgr.inqEntityGeneralOrExtend(entName, true);
   }else {
      return entMgr.inqEntityParameter(entName, true);
   }
}

/*--------------------------------------------------------------dereferEntity-+
| Effects:                                                                    |
|   Dereference an entity, opening it.                                        |
| Returns:                                                                    |
|   Pointer to the dereferenced entity, or 0 if failed.                       |
+----------------------------------------------------------------------------*/
Entity const * Yasp3::dereferEntity(bool isGeneral)
{
   int iLen;
   Entity const * pEnt = parseEntityReference(isGeneral, iLen);
   oec.skip(iLen);
   if (open(pEnt, true)) {
      return pEnt;
   }else {
      return 0;
   }
}

/*--------------------------------------------------------registerEntityStart-+
| Effects:                                                                    |
|   Register the logical start of an entity, after all implied events         |
|   have been generated                                                       |
+----------------------------------------------------------------------------*/
void Yasp3::registerEntityStart()
{
   assert  (evStartEntity.inqType() != YSP__noType);
   assert  (evStartEntity.inqEntity() == oec.inqEntity());
   oec.setElementDepth(elmMgr.inqDepth());
   if (options == YSP__synchroElmWithEnt) {                           //@015
      elmMgr.setFloorDepth(oec.inqElementDepth());
   }
   evlst += evStartEntity;
   evStartEntity = YSP__noType;
}

/*---------------------------------------------------------------handleEntity-+
| Effects:                                                                    |
|   Handle entity dereference.                                                |
|                                                                             |
| Returns:                                                                    |
|  true:  an event has been posted in evCurr                                  |
|  false: no event generated                                                  |
+----------------------------------------------------------------------------*/
bool Yasp3::handleEntity(bool isGeneral)
{
   int iLen;
   Entity const * pEnt = parseEntityReference(isGeneral, iLen);
   oec.skip(iLen);
   if (pEnt) {
      /*
      | isExpanded should go away: it is up to the Solver to answer
      | this question.
      */
      bool isOpenable = isExpanded[pEnt->inqKind()];
      if (isOpenable && !pSolver->isOpenable(*pEnt)) {
         isOpenable = false;
      }
      if (pEnt->isRcdata()) {
         /*
         | You should never asked me not to open a reparseable entity.
         | How much do you know about the parse state and what
         | is required to make the entity valid in context?
         | I need to read it, you don't want me to...
         | Tant pis pour vous.
         */
         if (isGeneral && !isOpenable) {
            evStartEntity = YaspEvent::Nil;
            evCurr = YaspEvent(pEnt, YSP__ENTITY_REF);
            return true;
         }
         /*
         | Report only where these is no evStartEntity pending.
         | Otherwise, delay the reporting until time is appropriate.
         | This allow to report event synchroneously: every events
         | implied by the entity content will be reported first.
         */
         if (open(pEnt, true)) {
            if (evStartEntity.inqType() == YSP__noType) {
               evCurr = YaspEvent(pEnt, YSP__ENTITY_START);
               return true;
            }else {
               /*
               | return false: no events generated.
               | someone will issue registerEntityStart
               | at the appropriate time
               */
               return false;
            }
         }
      }else {                        // NDATA, SDATA, CDATA, PI, SUBDOC
         /*
         | Here, the parser state is ready to receive data:
         | grabText() did startPcData as soon as the entity was recognized
         | Because we don't need to read non RcData to know if it is
         | or it is not data: it IS data.
         | No ENTITY_START event is pending.
         | Hence, we now can return REF or START without bothering
         | with event synchronism: the events have already been
         | generated.  We are in synch.
         */
         switch (pEnt->inqKind()) {  // PI only if parameter entity
         case ENTK_NDATA:
            evCurr = YaspEvent(pEnt, YSP__ENTITY_REF);
            return true;

         case ENTK_SDATA:
            if (!isOpenable) {
               evCurr = YaspEvent(pEnt, YSP__ENTITY_REF);
               return true;
            }
            if (open(pEnt, false)) {
               evCurr = YaspEvent(pEnt, YSP__SDATA_START);
               return true;
            }
            break;

         case ENTK_CDATA:
            if (!isOpenable) {
               evCurr = YaspEvent(pEnt, YSP__ENTITY_REF);
               return true;
            }
            if (open(pEnt, false)) {
               evCurr = YaspEvent(pEnt, YSP__CDATA_START);
               return true;
            }
            break;

         case ENTK_SUBDOC:
            evCurr = YaspEvent(pEnt, YSP__ENTITY_REF);
            return true;

         case ENTK_PI:
            // handlePiEntity(pEnt);  discarded (temporarly?)
            if (!isOpenable) {
               evCurr = YaspEvent(pEnt, YSP__ENTITY_REF);
               return true;
            }
            if (open(pEnt, false)) {
               evCurr = YaspEvent(pEnt, YSP__ENT_PI_START);
               return true;
            }

         default:
            evCurr = YSP__ERROR;
            return true;
         }
      }
   }
   evCurr = YaspEvent(pEnt, YSP__ENTITY_CANT);
   return true;
}

/*-------------------------------------------------------resumePreviousEntity-+
|  If we hit the end of an entity, ignore it unless:                          |
|  - it's The end.                                                            |
|  - entity ends should be reported, that is:                                 |
|    1. we're in a parm ent and not in the middle of a decl                   |
|    2. we're in a general entity                                             |
+----------------------------------------------------------------------------*/
bool Yasp3::resumePreviousEntity()
{
   bool rc = false;
   if (entMgr.inqDepth() > entMgr.inqFloorDepth()) {
      if (options == YSP__synchroElmWithEnt) {                           //@015
         // subtract the actual depth from the one that should be reached
         endAllElements(elmMgr.inqDepth() - oec.inqElementDepth());
      }
      if (options == YSP__reportEntityEvents) {
         evlst += YaspEvent(&entMgr.inqEntity(), YSP__ENTITY_END);
      }else {
         rc = true;                // no needs to stop
      }
      entMgr.skip();               // resume in previous entity
      if (options == YSP__synchroElmWithEnt) {                           //@015
         elmMgr.setFloorDepth(oec.inqElementDepth());
      }
   }
   return rc;
}

/*--------------------------------------------------OpenEntityCache_Extractor-+
| Effects:                                                                    |
|   Peek at the stream + iLen to look for a 4 digit (hexa)? decimal number.   |
| Returns:                                                                    |
|   true if OK, false otherwise                                               |
| Sets:                                                                       |
|   - the value of the number                                                 |
|   - the length parsed                                                       |
|                                                                             |
+----------------------------------------------------------------------------*/
class OpenEntityCache_Extractor : public UnicodeExtractor {
public:
   OpenEntityCache_Extractor(
      OpenEntityCache & oecArg, int & skippedArg
   ) :
      oec(oecArg), skipped(skippedArg)
   {}
private:
   int peek(int offset) { return oec.peek(skipped + offset); }
   int skip(int toSkip) {
      skipped += toSkip;
      return toSkip;
   }
   OpenEntityCache & oec;
   int & skipped;
};

/*-------------------------------------------------------tryDereferHexCharRef-+
| Effects:                                                                    |
|   Peek at a hexa-numeric character ref, after Delimiter::IX_HCRO found.     |
|   If isKeepUcCharRef is true, the entity may be not derefrencable in-line:  |
|   then it is a real try, and iLen can be 0 so the caller must stop.         |
|                                                                             |
| Returns:                                                                    |
|   the actual single character represented by the reference                  |
|   sets the length parsed                                                    |
+----------------------------------------------------------------------------*/
UCS_2 Yasp3::tryDereferHexCharRef(int & iLen, bool isKeepUcCharRef)
{
   iLen = 0;
   UCS_2 ucValue;
   OpenEntityCache_Extractor extract(oec, iLen);
   extract >> hex >> ucValue;
   assert  (extract.good());

   if (isKeepUcCharRef) {
      /*
      | Try to get a replacement entity for a Unicode Character Entity:
      | such as: &#x2022;.
      | If "x2022" has been defined as a non-replaceable (!isRcData, which is
      | CDATA / SDATA or NDATA), then we must stop.
      */
      UCS_2 entName[1+NAMELEN_MAX];
      entName[0] = (UCS_2)'x';
      int i;
      for (i=0; i < iLen; ++i) {
         entName[i+1] = oec.peek(i);
      }
      entName[i+1] = 0;
      Entity const * pEnt =  entMgr.inqEntityGeneral(entName, false);
      if (pEnt && (pEnt->inqKind() != ENTK_INVALID) && !pEnt->isRcdata()) {
         iLen = 0;                  // must stop!
         return 0;
      }
   }
   iLen = peekRefc(iLen);
   return ucValue;
}

/*----------------------------------------------------------dereferRegCharRef-+
| Effects:                                                                    |
|   Peek at a character reference, after Delimiter::IX_CRO has been found.    |
| Returns:                                                                    |
|   the actual single character represented by the reference                  |
| Sets:                                                                       |
|   - the length parsed                                                       |
|   - its type (TYPE_NON_SGML if invalid NAMED reference.                     |
|     This is non ambiguous, because a numeric char ref cannot be non-sgml    |
+----------------------------------------------------------------------------*/
UCS_2 Yasp3::dereferRegCharRef(Charset::e_CharType & ct, int & iLen)
{
   UCS_2 ucValue;
   iLen = 0;
   OpenEntityCache_Extractor extract(oec, iLen);
   extract >> ucValue;
   if (extract.good()) {               // Numeric Character Reference?
      ct = Charset::TYPE_REGULAR;
   }else {                             // Named Character Reference?
      UCS_2 buf[1+NAMELEN_MAX];

      iLen = peekName(buf, sdcl.charset().isCaseGeneral());
      if (iLen && (ucValue = sdcl.charset().funchar(buf), ucValue)) {
         ct = sdcl.charset().charType(ucValue);
      }else {
         erh << ECE__ERROR << _YSP__INVNAMCREF << buf << endm;
         ct = Charset::TYPE_NON_SGML;
         ucValue = 0;
      }
   }
   iLen = peekRefc(iLen);
   return ucValue;
}

/*===========================================================================*/
