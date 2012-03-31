/*
* $Id: ysptag.cpp,v 1.11 2002-04-01 10:21:23 pgr Exp $
*
* Low-level routines to parse tags
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include "../yasp3.h"
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"
#include "../syntax/delim.h"

/*----------------------------------------------------------YaspSignalEventCB-+
| Call back from ElmManager                                                   |
| to report the start of an element                                           |
+----------------------------------------------------------------------------*/
class YaspSignalEventCB : public ElmManager::SignalEventCB {
public:
   YaspSignalEventCB(Yasp3 * pYaspArg) : pYasp(pYaspArg) {}
   void startTag(Tag const * pTag, bool isEmpty);
   void endTag(Tag const * pTag);

   Yasp3 * pYasp;
};

/*---------------------------------------------------------------startElement-+
| Effects:                                                                    |
|    Push a given element on the ELM stack - no check / no call back          |
+----------------------------------------------------------------------------*/
bool Yasp3::startElement(UnicodeString const & ucsGi)
{
   Element const * pElm = elmMgr.inqElement(ucsGi);
   if (pElm) {
      RefdItem tagptr(new Tag(pElm, pElm->inqAttlist()));
      ElmManager::SignalEventCB cb;
      elmMgr.startElement(tagptr, cb);
      dlmfnd.setConMode(
         elmMgr.inqRecognitionMode(), elmMgr.isNetEnabled(), entMgr.inqDepth()
      );
      return true;
   }
   return false;
}

/*-----------------------------------------------------------------endElement-+
| Effects:                                                                    |
|    Pop an element from the top of the ELM stack - no check / no reporting   |
+----------------------------------------------------------------------------*/
bool Yasp3::endElement()
{
   ElmManager::SignalEventCB cb;
   if (elmMgr.endElement(cb)) {
      dlmfnd.setConMode(
         elmMgr.inqRecognitionMode(), elmMgr.isNetEnabled(), entMgr.inqDepth()
      );
      return true;
   }
   return false;
}

/*-------------------------------------------------YaspSignalEventCB::startTag-+
| This callback is used by Elm::startElement/endElement/welcomeElement        |
| to report the start of an element                                           |
+----------------------------------------------------------------------------*/
void YaspSignalEventCB::startTag(Tag const * pTag, bool isEmpty)
{

   //>>>PGR: i need to get rid of Entity* in Tag but it is required
   //        during the migration from Yasp to Yasp3.  So...
   /*
   | Get the first entity from any attribute ENTITY specif
   */
   UCS_2 entName[1+NAMELEN_MAX];

   if (pTag->inqAttlist().getFirstEntity(
         entName, pYasp->sdcl.charset().inqUcSpace()
      )
   ) {
      // following cast violates constness - and it *is* violated
      ((Tag *)pTag)->setEntity(
         pYasp->entMgr.inqEntityGeneral(entName, false)
      );
   }
   if (pTag->isInclusion()) {
      pYasp->rcdbnd.tellStartInclusion();
   }else {
      if (pYasp->rcdbnd.tellStartProper()) {
         //>>>PGR: currently, YSP does not report RE's
         //        Keep this during the migration from Yasp to Yasp3.
         if (pYasp->options == YSP__killRERS) {
            pYasp->evlst += YaspEvent(
               new TextData(pYasp->sdcl.charset().SPACEstr()),
               YSP__TEXT
            );
         }else {
            pYasp->evlst += YaspEvent(
               // line seps are RS (FIX?)
               new TextData(pYasp->sdcl.charset().RSstr()),
               YSP__TEXT
            );
         }
      }
   }
   {
      YaspType tType;
      if (isEmpty) {
         tType = YSP__TAG_EMPTY;
         if (pTag->isInclusion()) {
            pYasp->rcdbnd.tellEndInclusion();
         }else {
            pYasp->rcdbnd.tellEndProper();
         }
      }else {
         tType = YSP__TAG_START;
      }
      pYasp->evlst += YaspEvent(pTag, tType);
   }

   /* Include info on where we started, for better error reporting.
   */
   /* >>>PGR: to do!
   |  oelm.startEntity = entityStack->eTheEntPtr;
   |  oelm.startLine   = entityStack->eLine;
   */
}

/*---------------------------------------------------YaspSignalEventCB::endTag-+
| This callback is used by Elm::startElement/endElement/welcomeElement        |
| to report the end of an element                                             |
+----------------------------------------------------------------------------*/
void YaspSignalEventCB::endTag(Tag const * pTag)
{
   if (pTag->isInclusion()) {
      pYasp->rcdbnd.tellEndInclusion();
   }else {
      pYasp->rcdbnd.tellEndProper();
   }
   pYasp->evlst += YaspEvent(pTag, YSP__TAG_END);

   /* Include info on where we ended, for better error reporting.
   */
   /* >>>PGR: to do!
   |  oelm.pElm->lastSeenAt = (s.iDepth >= 0)? entityStack->eLine : 0L;
   */
}

/*-------------------------------------------------------------endAllElements-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Yasp3::endAllElements(int iMax)
{
   YaspSignalEventCB cb(this);
   if (elmMgr.endAllElements(cb, iMax)) {
      dlmfnd.setConMode(
         elmMgr.inqRecognitionMode(), elmMgr.isNetEnabled(), entMgr.inqDepth()
      );
      return true;
   }
   return false;
}

/*----------------------------------------------------------------startPcData-+
| Return true if pending events have been generated (caller should stop)      |
+----------------------------------------------------------------------------*/
bool Yasp3::startPcData()
{
   RecordBinder rcdbndBackup(rcdbnd);
   YaspSignalEventCB cb(this);

   if (ElmManager::WCOND_HIT != elmMgr.welcomeElement(0, cb)) {
      rcdbnd = rcdbndBackup;
      evlst.reset();
   }else if (evlst.isEventWaiting()) {
      if (!sdcl.features().isOmittag()) {
        erh << ECE__ERROR << _YSP__INVTAGOMIT << endm;
      }
      return true;
   }
   return false;
}

/*-------------------------------------------------------------handleStartTag-+
| Effects:                                                                    |
|    This routine is called to parse a start tag                              |
|                                                                             |
| When entering:                                                              |
|      - curr_char is after the STAGO delimiter                               |
|                                                                             |
| When returning:                                                             |
|      - curr_char is:                                                        |
|          - either the character that follows a NET or a TAGC                |
|          - or *AT* the ETAGOs, STAGOs delimiters                            |
|      - events might have been stacked.                                      |
+----------------------------------------------------------------------------*/
void Yasp3::handleStartTag()
{
   bool isConrefSpec = false;
   Element const * pElm;
   Attlist attlst;
   Tag::Flag tagFlag;

   switch (dlmfnd.delimFound()) {
   case Delimiter::IX_STAGO:
      /*
      | Regular tag: parse it, including the attributes
      */
      {
         UCS_2 giName[1+NAMELEN_MAX];
         if (!grabNameGeneral(giName)) {
            closeTag(false);
            evCurr = YSP__ERROR;
            return;
         }
         if (pElm = elmMgr.inqElement(giName), !pElm) {
            if (options == YSP__validate) {
               erh << ECE__ERROR << _YSP__INVSTGNAME << giName << endm;
            }
            pElm = elmMgr.defineElementIfNotThere(giName);
         }else {
            attlst = pElm->inqAttlist();
         }
      }
      dlmfnd.pushMode(MODE_TAG);
      closeTag(parseAttSpecList(attlst, isConrefSpec));
      if (dlmfnd.delimFound() == Delimiter::IX_NESTC){
         if (options == YSP__validate) {
            if (sdcl.isXML()) {
               /*
               | XML is allowing a net-enabling start-tag
               | only when immediately followed by a null end-tag.
               */
               int iOffset = 0;
               if (!peek(value(Delimiter::IX_NET), false, iOffset)) {
                  //>>PGR: produce a better message, when SHR freeze lifted.
                  erh << ECE__ERROR << _YSP__NOTAGC << endm;
               }
            }else if (
               !sdcl.features().isShorttag() || pElm->inqModel().isEmpty()
            ) {
               erh << ECE__ERROR << _YSP__INVNETMIN << endm;
            }
         }
         tagFlag = Tag::Net;
      }else {
         tagFlag = Tag::Regular;
      }
      dlmfnd.popMode();
      break;

   default:                                       // STAGO_TAGC "<>"
      /*
      | By 7.4.1.1, a GI specification is implied for an
      | empty start tag before to determine whether
      | any tags were omitted before it
      | a) if (OMITTAG==YES) <> == most recently started
      | b) if (OMITTAG==NO)  <> == most recently ended
      | c) or the base document element
      */
      if (elmMgr.inqDepth() > 1) {
         if (sdcl.features().isOmittag()) {       // 7.4.1.1 a)
            pElm = elmMgr.inqElementLastOpened();
         }else {                                  // 7.4.1.1 b)
            pElm = elmMgr.inqElementLastClosed();
         }
      }else {                                     // 7.4.1.1 c)
         pElm = elmMgr.inqElementBase();
      }
      if (!pElm) {
         erh << ECE__ERROR << _YSP__INVSTGEMPTY << endm;
         evCurr = YSP__ERROR;
         return;
      }
      attlst = pElm->inqAttlist();
      tagFlag = Tag::Empty;
      break;
   }
   /*
   | parsing job now finished: open the element
   */
   RecordBinder rcdbndBackup(rcdbnd);
   YaspSignalEventCB cb(this);

   switch (elmMgr.welcomeElement(pElm, cb)) {
   case ElmManager::WCOND_INCLUDED:
      tagFlag << Tag::Inclusion;
      if (!pElm->isDefined()) {          // faked inclusion
         tagFlag << Tag::InvalidStart;
      }
      break;
   case ElmManager::WCOND_HIT:
      break;
   case ElmManager::WCOND_FORCED:        // forced to be valid
      tagFlag << Tag::InvalidStart;
      break;
   case ElmManager::WCOND_INVALID:
      tagFlag << Tag::InvalidStart;
      rcdbnd = rcdbndBackup;
      evlst.reset();
      break;
   default:
      assert (false);
      rcdbnd = rcdbndBackup;
      evlst.reset();
      evCurr = YSP__ERROR;
      return;
   }
   if ((options == YSP__validate) &&
      evlst.isEventWaiting() && !sdcl.features().isOmittag()
   ) {
      erh << ECE__ERROR << _YSP__NOTAGOMIT << endm;
   }
   if (evStartEntity.inqType() != YSP__noType) { // entity start still pending
       registerEntityStart();
   };
   RefdItem tagptr(new Tag(pElm, attlst, tagFlag));
   if (!elmMgr.startElement(tagptr, cb, isConrefSpec)) { // == isEmpty
      evCurr = YSP__ERROR;     // YSP__FATAL???
   }else {
      dlmfnd.setConMode(
         elmMgr.inqRecognitionMode(),
         elmMgr.isNetEnabled(),
         entMgr.inqDepth(),
         pElm->inqGi()
      );
      evlst.next(evCurr);
   }
   /*
   | The EMPTYNRM feature allows EMPTY tags to be followed by an end tag.
   | Look ahead and see if the current empty tag is followed by its
   | matching end tag.  If so: discard it.  This will keep the tag
   | reporting consistent: end of EMPTY tags is never reported.
   */
   if (
      sdcl.features().isEmptynrm() &&
      (pElm->inqModel().isEmpty() || isConrefSpec)
   ) {
      int iOffset = 0;
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_TAGC:
         if (!peek(value(Delimiter::IX_ETAGO), false, iOffset)) {
            break;
         }
         /* fall thru */
      case Delimiter::IX_ETAGO:
         if (peek(pElm->inqGi(), sdcl.charset().isCaseGeneral(), iOffset)) {
            oec.skip(iOffset);
            dlmfnd.pushMode(MODE_TAG);
            skip_S();
            closeTag(true);
            if (dlmfnd.delimFound() == Delimiter::IX_NESTC) {
               erh << ECE__ERROR << _YSP__INVDLM << endm;
            }
            dlmfnd.popMode();
         }
         break;
      case Delimiter::IX_NESTC:
         if (peek(value(Delimiter::IX_NET), false, iOffset)) {
            oec.skip(iOffset);
         }
         break;
      case Delimiter::IX_ETAGO_TAGC:
         oec.skip(dlmfnd.delimLength());
         break;
      default:
         break;
      }
   }
}

/*---------------------------------------------------------------handleEndTag-+
| Effects:                                                                    |
|    This routine is called to parse an end tag                               |
|                                                                             |
| When entering:                                                              |
|      - curr_char is after the ETAGO delimiter                               |
|                                                                             |
| When returning:                                                             |
|      - curr_char is:                                                        |
|          - either the character that follows the TAGC                       |
|          - or *AT* the ETAGOs, STAGOs delimiters                            |
|      - events might have been stacked.                                      |
+----------------------------------------------------------------------------*/
void Yasp3::handleEndTag()
{
   YaspSignalEventCB cb(this);

   switch (dlmfnd.delimFound()) {
   case Delimiter::IX_ETAGO_TAGC:                 // ETAGO_TAGC "</>"
      if (!elmMgr.endElement(cb)) {
         erh << ECE__ERROR << _YSP__INVETGEMPTY << endm;
         evCurr = YSP__ERROR;
         return;
      }
      break;

   case Delimiter::IX_ETAGO:   // regular tag: parse it
      {
         UCS_2 giName[1+NAMELEN_MAX];
         bool isOk = true;

         /*
         | PGR (02/25/02) - The rule described below is wierd.
         | The Delimiter Recognition no more recognizes an ETAGO
         | when, in DCDATA, the end tag doesn't match the tag  that
         | started the PCDATA.
         */
         /*
         | endElement should be called in any case: if the current element
         | is DcData, even "</invalid" ends it (7.6 p24, last para)
         */
         if (!grabNameGeneral(giName)) {
            elmMgr.endElement(0, cb);
            isOk = false;
         }else {
            Element const * pElm = elmMgr.inqElement(giName);
            if (!elmMgr.endElement(pElm, cb)) {
               if (!pElm) {
                  erh << _YSP__INVETGNAME;
               }else {
                  erh << _YSP__INVETGOOC;
               }
               erh << ECE__ERROR << giName << endm;
               isOk = false;
            }
         }
         dlmfnd.pushMode(MODE_TAG);
         skip_S();
         closeTag(isOk);
         if (dlmfnd.delimFound() == Delimiter::IX_NESTC){
            erh << ECE__ERROR << _YSP__INVDLM << endm;
         }
         dlmfnd.popMode();
         if (!isOk) {          // in case DcData ended, reset recgn mode
            dlmfnd.setConMode(
               elmMgr.inqRecognitionMode(),
               elmMgr.isNetEnabled(),
               entMgr.inqDepth()
            );
            evCurr = YSP__ERROR;
            return;
         }
      }
      break;
   default: /* case Delimiter::IX_NET */   // NET "/" or ">" (Web)
      if (!elmMgr.endElementNet(cb)) {
         assert (false);                   // cannot be (context constraint)
         evCurr = YSP__ERROR;
         return;
      }
      break;
   }
   if ((options == YSP__validate) &&
      (evlst.count() > 1) && !sdcl.features().isOmittag()
   ) {
      erh << ECE__ERROR << _YSP__NOTAGOMIT << endm;
   }
   dlmfnd.setConMode(
      elmMgr.inqRecognitionMode(), elmMgr.isNetEnabled(), entMgr.inqDepth()
   );
   evlst.next(evCurr);
}

/*-------------------------------------------------------------------closeTag-+
| Effects:                                                                    |
|    Get to the close delimiter for either a start or end tag.                |
|    Things are quite similar for start and end: avoid duplicating same code! |
| When returning:                                                             |
|    current char is:                                                         |
|      - either the character that follows a NET or a TAGC                    |
|      - or *AT* the ETAGOs, STAGOs delimiters                                |
|    delimFound() has still the last value found: this trick allows to        |
|        recognize a NET-enabling start tag                                   |
| Notes:                                                                      |
|    Operates in "TAG" mode.                                                  |
+----------------------------------------------------------------------------*/
void Yasp3::closeTag(bool isReportErrors)
{
   for (;;) {
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_TAGC:                // OK.
      case Delimiter::IX_NESTC:               // a NET-enabling start_tag
         return;
      case Delimiter::IX_ETAGO_GRPO:
      case Delimiter::IX_STAGO_GRPO:
      case Delimiter::IX_ETAGO:
      case Delimiter::IX_ETAGO_TAGC:
      case Delimiter::IX_STAGO:
      case Delimiter::IX_STAGO_TAGC:
         oec.backup(dlmfnd.delimLength());    // Could never be recognized:
         if (!sdcl.features().isShorttag() && // rule 7.4.1 and 7.5.1
            (options == YSP__validate)
         ) {
            erh << ECE__ERROR << _YSP__NOTAGCMIN << endm;
         }
         return;                              // else, no contextual constraints
      case Delimiter::IX_NOT_FOUND:
         if (!entMgr.skip()) {
            return;
         }
         break;
      default:
         break;
      }
      if (isReportErrors) {
         erh << ECE__ERROR << _YSP__NOTAGC << endm;
         isReportErrors = false;
      }
      dlmfnd.find(oec);
   }
}

/*--------------------------------------------------------------grabValueSpec-+
| Get (and check) an Attribute Value Specification                            |
|                                                                             |
|  [33] attribute value specification =                                       |
|       attribute value | attribute value literal                             |
|                                                                             |
|  [34] attribute value literal =                                             |
|       (lit, replaceable character data*, lit) |                             |
|       (lita, replaceable character data*, lita)                             |
|                                                                             |
|  [35] attribute value =                                                     |
|         character data |                                                    |
|         general entity name | general entity name list |                    |
|         id value |                                                          |
|         id reference value | id reference list |                            |
|         name | name list |                                                  |
|         name token | name token list |                                      |
|         notation name |                                                     |
|         number | number list |                                              |
|         number token | number token list                                    |
|                                                                             |
|                                                                             |
| On ENTRY:                                                                   |
|    `curr_token/curr_char' points the first character of the AVS,            |
|     pAtt points an Attribute that defines the attribute,                    |
|          or is a NULL pointer if none where found                           |
|                                                                             |
| When RETURNING:                                                             |
|   - "curr_char" is at next unknown character.                               |
|                                                                             |
| The attribute value or attribute value literal is parsed as explained       |
| in 7.9.3 of ISO8879-1986(E).                                                |
| `is.cdata' tells if we need to keep blanks sequences, leading and           |
| trailing blanks.                                                            |
|                                                                             |
| Recognized in MODE_LITC:  CRO, LIT, LITA, ERO, ERO_GRPO                     |
|                                                                             |
| Implementation Note:                                                        |
|   this routine is very similar to getParameterLiteral in yspgrab.cc         |
|   but no enough close so we can merge both into a unique routine            |
|   (at least now now, 03/08/96 - PGR)                                        |
|   If bugs are found, you might want to examine the sister routine!          |
+----------------------------------------------------------------------------*/
UnicodeString Yasp3::grabValueSpec(
   Attribute const * pAtt,        // Attribute or NULL
   Delimiter::e_Ix & dlmStartRet  // (rtn) what delimiter started the literal
) {
   Charset::e_Class clsFirstChar = Charset::CLS_NAMESTRT;
   Charset::e_Class clsOtherChar = Charset::CLS_NAME;
   bool isPlural = false;
   bool isCdata = false;
   bool isEntity = false;
   e_AttDclVal dclvalToCheck = ADCLV_INVALID;  // nothing to check

   if (!pAtt) {
      isCdata = true;
      clsFirstChar = Charset::CLS_NAME;
   }else {
      switch (pAtt->inqTypeDeclaredValue()) {
      case ADCLV_ENUM:
         clsFirstChar = Charset::CLS_NAME;
         /* fall thru */
      case ADCLV_NOTATION:
         dclvalToCheck = ADCLV_ENUM;     // checkValueInList;
         break;
      case ADCLV_ENTITIES:
         isPlural = true;
         /* fall thru */
      case ADCLV_ENTITY:
         dclvalToCheck = ADCLV_ENTITY;   // checkEntity;
         isEntity = true;
         break;
      case ADCLV_CDATA:                  // nothing to check
      case ADCLV_INVALID:
         isCdata = true;
         clsFirstChar = Charset::CLS_NAME;
         break;
      case ADCLV_NMTOKENS:
         isPlural = true;
         /* fall thru */
      case ADCLV_NMTOKEN:
         clsFirstChar = Charset::CLS_NAME;
         break;
      case ADCLV_NUTOKENS:
         isPlural = true;
         /* fall thru */
      case ADCLV_NUTOKEN:
         clsFirstChar = Charset::CLS_DIGIT;
         break;
      case ADCLV_NUMBERS:
         isPlural = true;
         /* fall thru */
      case ADCLV_NUMBER:
         clsFirstChar = clsOtherChar = Charset::CLS_DIGIT;
         break;
      case ADCLV_ID:
         dclvalToCheck = ADCLV_ID;       // checkIdUnique;
         break;
      case ADCLV_IDREFS:
      case ADCLV_NAMES:
         isPlural = true;
         break;
      default:   /* ADCLV_NAME(s), ADCLV_IDREF(s) */
         break;
      }
   }
   bool isNameCase;
   if (isCdata) {
      isNameCase = false;
   }else if (isEntity) {
      isNameCase = sdcl.charset().isCaseEntity();
   }else {
      isNameCase = sdcl.charset().isCaseGeneral();
   }

   UnicodeBuffer buf;
   bool isReportErrors = (options == YSP__validate)? true : false;
   int const iLineStart = oec.inqLine();

   Delimiter::e_Ix dlmStart = dlmfnd.delimFound();
   if ((dlmStart == Delimiter::IX_NESTC) && (options == YSP__HTML)) {
      dlmStart = Delimiter::IX_NOT_FOUND;
      oec.backup(dlmfnd.delimLength());
   }
   switch (dlmStart) {
   case Delimiter::IX_LIT:
   case Delimiter::IX_LITA:
      {
         int const iDepthStart = oec.iDepth;
         /*------------------------------------------------------------+
         | Derive the attribute value from the attribute value literal |
         +------------------------------------------------------------*/
         bool isReNow = false;
         bool isReBefore = false;
         bool isSpaceWaiting = false;
         bool isMssNow = false;
         bool isMssBefore = false;

         dlmfnd.pushMode(MODE_LITC);
         for (;;) {
            int iCur = oec.peek();
            if (iCur == EOF_SIGNAL) {
               // BACKUP NEEDED
               dlmfnd.setMode(MODE_LITC);     // mode might have been disabled!
               if (oec.iDepth > iDepthStart) {
                  entMgr.skip();              // Ignore Ee (7.9.3)
                  continue;                   // pursue
               }
               erh << ECE__ERROR << _YSP__UBLNLIT << iLineStart << endm;
               isReportErrors = false;
               break;                         // exit: unbalanced LIT's
            }
            Delimiter::e_Ix dlmFound;
            isReBefore = isReNow;
            isReNow = false;
            if (isMssBefore = isMssNow, isMssBefore == true) {
               isMssNow = false;
               dlmFound = Delimiter::IX_NOT_FOUND;
            }else {
               dlmFound = dlmfnd.find(oec, iCur);
               if ((dlmFound == dlmStart) && (oec.iDepth == iDepthStart)) {
                  break;                      // exit: all is ok
               }
            }
            switch (dlmFound) {

            case Delimiter::IX_CRO:
               {
                  Charset::e_CharType ct;
                  int iLen;
                  iCur = dereferRegCharRef(ct, iLen);
                  oec.skip(iLen);
               }
               break;                  //>>PGR: examine the char type!

            case Delimiter::IX_HCRO:
               {
                  int iLen;
                  iCur = tryDereferHexCharRef(iLen, false);
                  oec.skip(iLen);
               }
               break;                  //>>PGR: examine the char type!

            case Delimiter::IX_NOT_FOUND:
               oec.skip();
               switch (sdcl.charset().charType(iCur)) {

               case Charset::TYPE_SEPA_RS:
                  // Standard says ignore RS, but take it as an RE+RS
                  // if no RE precedes.  As in the case of a UNIX file,
                  // it is the only line-separator we get.
                  if (isReBefore) {
                     continue;                // ignore RS
                  }
                  /* fall thru */
               case Charset::TYPE_SEPA_RE:
                  isReNow = true;
                  /* fall thru */
               case Charset::TYPE_SEPA_TAB:
               case Charset::TYPE_SEPA_SPACE:
               case Charset::TYPE_SEPA_OTHER:
                  if (!isCdata) {             // RE, SPACE, SEPCHAR
                     if (buf.length()) isSpaceWaiting = true;
                     continue;                // reduce SPACE sequences
                  }
                  iCur = sdcl.charset().inqUcSpace(); // don't reduce
                  break;
               case Charset::TYPE_MSO:        // 9.7
                  if (!isMssBefore) dlmfnd.setMode(MODE_VOID);
                  break;
               case Charset::TYPE_MSI:
                  dlmfnd.setMode(MODE_LITC);
                  break;
               case Charset::TYPE_MSS:
                  isMssNow = true;
                  break;
               default:  // TYPE_REGULAR, TYPE_NON_SGML
                  break;
               }
               break;

            case Delimiter::IX_ERO:           // LITC_MODE only
               {
                  Entity const * pEnt = dereferEntity(true);
                  if (pEnt) {
                     switch (pEnt->inqKind()) {
                     case ENTK_SDATA:
                     case ENTK_CDATA:
                     case ENTK_NDATA:             // who knows?
                        dlmfnd.setMode(MODE_VOID);
                        break;
                     default:
                        break;
                     }
                  }
               }
               continue;

            default:                          // ERO_GRPO (see 9.4.4)
               if (isReportErrors) {
                  erh << ECE__ERROR << _YSP__INVDLM << endm;
                  isReportErrors = false;
               }
               /* fall thru */

            case Delimiter::IX_LIT:
            case Delimiter::IX_LITA:
               if (dlmfnd.delimLength() > 1) {
                  oec.backup(dlmfnd.delimLength()-1);
               }
               break;
            }

            /*--------------------------------------------------+
            | If a token just ended, make first specific checks |
            +--------------------------------------------------*/
            if (isSpaceWaiting) {             // never happens if isCdata
               isSpaceWaiting = false;
               if (isReportErrors) {
                  if (!isPlural) {
                     erh << ECE__ERROR << _YSP__BIGTKLIST << endm;
                     isReportErrors = false;
                  }else if (
                     buf.lengthCurrentToken() >
                     sdcl.quantity()[Quantity::IX_NAMELEN]
                  ) {
                     erh << ECE__ERROR << _YSP__BIGNAME
                         << sdcl.quantity()[Quantity::IX_NAMELEN]
                         << endm;
                     isReportErrors = false;
                  }else if (!checkDclVal(
                        dclvalToCheck, pAtt, buf.getCurrentToken()
                  )) {
                     isReportErrors = false;
                  }
               }
               buf.endCurrentToken(sdcl.charset().inqUcSpace());
            }
            /*-----------------------------+
            | Then enter the new character |
            +-----------------------------*/
            if (isCdata) {
               buf << iCur;
            }else {
               if (
                  isReportErrors && !sdcl.charset().isClass(
                     iCur, buf.isFirstChar()? clsFirstChar : clsOtherChar
                  )
               ) {
                  erh << ECE__ERROR << _YSP__INVATTSPEC << endm;
                  isReportErrors = false;
               }
               if (isNameCase) {
                  buf << sdcl.charset().uppercase(iCur);
               }else {
                  buf << iCur;
               }
            }
         }
         /*---------------------+
         | End of literal found |
         +---------------------*/
         if (isReportErrors && !isCdata && !buf.lengthCurrentToken()) {
            erh << ECE__ERROR << _YSP__INVEMPTYLIT << endm;
            isReportErrors = false;
         }
         dlmfnd.popMode();
      }
      break;
   case Delimiter::IX_NOT_FOUND:
      if (options == YSP__HTML) {             // get anything 'til MDC or SPACE
         int iCur;
         dlmfnd.pushMode(MODE_BYP_MD);
         while (
               iCur = oec.peek(),
               // BACKUP NEEDED
               (iCur != EOF_SIGNAL) &&
               (dlmfnd.find(oec, iCur) == Delimiter::IX_NOT_FOUND) &&
               !sdcl.charset().isSeparator(iCur)
         ) {
            buf << iCur;
            oec.skip();
         }
         dlmfnd.popMode();
         if (dlmfnd.delimFound() != Delimiter::IX_NOT_FOUND) {
            oec.backup(dlmfnd.delimLength());         // Not for us!
         }
      }else {
         int iCur = oec.peek();
         if (
            !sdcl.charset().isClass(iCur, clsFirstChar) ||     // 7.9.3.1 a
            (pastDOCTYPE && !sdcl.features().isShorttag())
         ) {
            if (isReportErrors) {
               erh << ECE__ERROR << _YSP__INVATTSPEC << endm;
               isReportErrors = false;
            }
            /* get the invalid First, and pursue anyway
            |  not breaking is OK: clsFirst is more restrictive than any Other
            */
            clsOtherChar = Charset::CLS_NAME;
         }
         while (sdcl.charset().isClass(iCur, clsOtherChar)) {
            if (isNameCase) {
               buf << sdcl.charset().uppercase(iCur);
            }else {
               buf << iCur;
            }
            oec.skip();
            iCur = oec.peek();
         }
      }
      break;
   default:
      isReportErrors = false;
      if (options != YSP__HTML) {
         erh << ECE__ERROR << _YSP__INVATTSPEC << endm;
      }
      oec.backup(dlmfnd.delimLength());      // Not for us!
      dlmStart = Delimiter::IX_NOT_FOUND;
      break;
   }
   assert (buf);
   if (buf.length() > sdcl.quantity()[Quantity::IX_LITLEN]) {
      erh << ECE__ERROR << _YSP__BIGAVL << iLineStart << entMgr.inqLine()
          << sdcl.quantity()[Quantity::IX_LITLEN]
          << endm;
   }else if (isReportErrors) {
      if (
         !isCdata &&
         (buf.lengthCurrentToken() > sdcl.quantity()[Quantity::IX_NAMELEN])
      ) {
         erh << ECE__ERROR << _YSP__BIGNAME
             << sdcl.quantity()[Quantity::IX_NAMELEN] << endm;
      }
      checkDclVal(dclvalToCheck, pAtt, buf.getCurrentToken());
   }
   dlmStartRet = dlmStart;

   return buf.getResult();
}


/*-------------------------------------------------------------------checkXXX-+
| Check functions are called for each token listed in an attribute value spec |
+----------------------------------------------------------------------------*/
inline bool Yasp3::checkIdUnique(
   Attribute const *, UCS_2 const *
) const {
   return true;
}
inline bool Yasp3::checkValueInList(
   Attribute const * pAtt, UCS_2 const * pUcToken
) const {
   if ((pAtt->findToken(pUcToken)).good()) {
      return true;
   }else {
      return false;
   }
}
inline bool Yasp3::checkEntity(
   Attribute const *, UCS_2 const *
) const {
   return true;
}
bool Yasp3::checkDclVal(
   e_AttDclVal dclvalToCheck,
   Attribute const * pAtt,
   UCS_2 const * pUcToken
) const {
   switch (dclvalToCheck) {
   case ADCLV_ENUM:    return checkValueInList(pAtt, pUcToken);
   case ADCLV_ENTITY:  return checkEntity(pAtt, pUcToken);
   case ADCLV_ID:      return checkIdUnique(pAtt, pUcToken);
   default:            return true;
   }
}

/*----------------------------------------------------------makeHtmlAttribute-+
| In HTML, whith an attlist such as: <!ATTLIST foo    blah NAME bozo>         |
| <foo blah>  means  <foo blah=bozo>                                          |
| When entering:                                                              |
|    attlst is the attribute list,                                            |
|    spec has the attribute name ('blah')                                     |
| When returning:                                                             |
|    - if success:                                                            |
|          spec has the default value (bozo)                                  |
|          a pointer to the right attribute in attlst is returned             |
|    - otherwise:                                                             |
|          0 is returned                                                      |
+----------------------------------------------------------------------------*/
static Attribute const * makeHtmlAttribute(
   Attlist const & attlst, UnicodeString & spec
) {
   Attribute const * pAttSource = attlst.inqAttributePtr(spec);
   if (pAttSource) {
      UnicodeString defValue = pAttSource->inqValue();
      if (defValue.good()) {
         spec = defValue;
         return pAttSource;
      }
   }
   return 0;
}

/*-----------------------------------------------------------parseAttSpecList-+
| Parse an Attribute Specification List.                                      |
| MODE_TAG or MODE_ASL is assumed to be active!                               |
|                                                                             |
|  [31][32] attribute specification list =                                    |
|      (s*, (name, s*, vi, s*)?, attribute value specification)*              |
|                                                                             |
+----------------------------------------------------------------------------*/
bool Yasp3::parseAttSpecList(
   Attlist & attlst,         // i/o: attlist
   bool & isConrefSpec
) {
   isConrefSpec = false;
   UCS_2 name[1+NAMELEN_MAX];

   // Parse one attribute (name=value or value pair) per pass.

   while (skip_S(), dlmfnd.delimFound() == Delimiter::IX_NOT_FOUND) {
      if (!grabNameToken(name)) { // name or value?
         return false;
      }
      skip_S();
      Attribute * pAtt;
      Attribute const * pAttSource;
      Delimiter::e_Ix dlmLit;
      UnicodeString ucsValSpec;
      if (dlmfnd.delimFound() == Delimiter::IX_VI) {   // then, it's a name
         /*-------------------------------------+
         | Attribute name found, followed by VI |
         +-------------------------------------*/
         pAttSource = attlst.inqAttributePtr(name);
         if (!pAttSource) {                      // no such name:
            if (options == YSP__validate) {
               erh << ECE__ERROR << _YSP__NOATTVAL << name << endm;
            }
            pAtt = attlst.defineAttr(name);      // create one on the fly
         }else {                                 // else, do a copy
            if ((options == YSP__validate) && pAttSource->isSpecified()) {
               erh << ECE__ERROR << _YSP__DUPATTSPEC  // duplicate!
                   << pAttSource->key() << endm;
            }
            pAtt = attlst.replace(pAttSource);
         }
         assert  (pAtt);
         skip_S();
         ucsValSpec = grabValueSpec(pAtt, dlmLit);
         /*
         | These lines commented out by PGR (03/01/02)
         | grabValueSpec backs up for delimiter that it doesn't recognize.
         | This assumes the skip_S() at the top of this loop will be
         | re-doing a dlmfind().  If we return false now, then the
         | backup will never be redone.  Hence: we don't return.
         | Seen from the outside, it's a very light change,
         | which results in returning true instead of false.
         | And returning true or false is just cosmetics for error
         | reporting (I even think it is best to let the error reporting on.)
         */
//       if (!ucsValSpec && (dlmLit == Delimiter::IX_NOT_FOUND)) {
//          return false;
//       }
      }else {
         /*---------------------------------+
         | AVS found without att name, VI.  |
         | Find if any token in any GROUP   |
         | or NOTATION could match it       |
         +---------------------------------*/
         oec.backup(dlmfnd.delimLength());      // delimiter not for us
         if ((options == YSP__validate) && !sdcl.features().isShorttag()) {
            erh << ECE__ERROR << _YSP__INVATTSPEC << endm;
         }
         pAttSource = attlst.findToken(name);
         dlmLit = Delimiter::IX_NOT_FOUND;
         ucsValSpec = UnicodeString(name);       // not a name: value!
         if (!pAttSource && (                    // no such value:
               (options != YSP__HTML) ||
               (pAttSource=makeHtmlAttribute(attlst, ucsValSpec), !pAttSource)
            )
         ) {
            if (options == YSP__validate) {
               erh << ECE__ERROR << _YSP__NOATTVAL << name << endm;
            }
            pAtt = attlst.defineAttr("");        // create unnamed attribute
         }else {                                 // else, do a copy
            if (pAttSource->isSpecified()) {     // duplicate!
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__DUPATTSPEC
                      << pAttSource->key() << endm;
               }
            }
            pAtt = attlst.replace(pAttSource);
         }
         assert  (pAtt);
      }
      pAtt->setValueSpec(ucsValSpec, dlmLit, true);
      if (pAttSource) switch (pAttSource->inqTypeDefaultValue()) {
      case ADFTV_FIXED:
         if (
            (options == YSP__validate) &&
            pAttSource->inqValue().compare(ucsValSpec)
         ) {
            erh << ECE__ERROR << _YSP__INVFIXED
                << pAttSource->key() << pAttSource->inqValue()
                << endm;
         }
         break;
      case ADFTV_CONREF:
         isConrefSpec = true;
         break;
      case ADFTV_CURRENT:
         // following cast violates constness
         ((Attribute *)pAttSource)->setValueSpec(ucsValSpec, dlmLit);
         break;
      default:
         break;
      }
   }
   /*
   | Make further checks
   */
   if ((options == YSP__validate) && attlst.isRequired()) {
      int const ixLast = attlst.count();
      for (int ixAtt=0; ixAtt < ixLast; ++ixAtt) {
         Attribute const * pAtt = attlst.inqAttributePtr(ixAtt);
         if (
            (pAtt->inqTypeDefaultValue() == ADFTV_REQUIRED) &&
            !pAtt->isSpecified()
         ) {
            erh << ECE__ERROR << _YSP__NOREQUIRED << pAtt->key() << endm;
         }
      }
   }
   return true;
}

/*===========================================================================*/
