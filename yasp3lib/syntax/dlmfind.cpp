/*
* $Id: dlmfind.cpp,v 1.4 2011-07-29 10:26:40 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include "../entity/entmgr.h"
#include "sdcl.h"
#include "dlmbuild.h"
#include "dlmfind.h"

static UCS_2 const ucsNil[1] = { 0 };
VectorItem const VectorItem::Nil = {0, 0, (UCS_2 *)ucsNil};
VectorItemModePtr const VectorItemModePtr::Nil;

/*---------------------------------------VectorItemModePtr::VectorItemModePtr-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline VectorItemModePtr::VectorItemModePtr(
   DelimBuilder & builder, e_RecognitionMode rmode
) :
   pVi(builder.build(rmode))
{}

/*--------------------------------------VectorItemModePtr::~VectorItemModePtr-+
| Destruct the contents of apViEntries                                        |
+----------------------------------------------------------------------------*/
VectorItemModePtr::~VectorItemModePtr()
{
   if (*this != VectorItemModePtr::Nil) {
      UCS_2 * pUcBuffer = pVi->f3.str;
      if (*pUcBuffer) --pUcBuffer;
      delete [] pUcBuffer;
      delete [] (VectorItem *)pVi;  // cast away constness when deleting
   }
}

/*-----------------------------------------------VectorItemModePtr::operator=-+
| Assignement                                                                 |
+----------------------------------------------------------------------------*/
inline VectorItemModePtr & VectorItemModePtr::operator=(
   VectorItemModePtr const & source
) {
   if (pVi != source.pVi) {
      this->~VectorItemModePtr();
      new(this) VectorItemModePtr(source);
   }
   return *this;
}

/*-------------------------------------------VectorItemArray::VectorItemArray-+
| Construct the array of Vector Items                                         |
+----------------------------------------------------------------------------*/
inline VectorItemArray::VectorItemArray(
   DelimList const & dlmlst, Features const & ft, UnicodeComposer & erh
) {
   bool isOk;
   DelimBuilder builder(dlmlst, ft, erh, isOk);
   if (isOk) {
      for (int j=0; j < NBR_OF_MODES; ++j) {
         if ((e_RecognitionMode)j == MODE_VOID) continue;
         new(apVi + j) VectorItemModePtr(builder, (e_RecognitionMode)j);
         if (apVi[j] == VectorItemModePtr::Nil) {
            while (j--) {
               apVi[j] = VectorItemModePtr::Nil;
            }
            break;
         }
      }
   }
}

/*-------------------------------------------VectorItemArray::resetDelimiters-+
| Re-build the array of Vector Items                                          |
+----------------------------------------------------------------------------*/
inline void VectorItemArray::resetDelimiters(
   DelimList const & dlmlst, Features const & ft, UnicodeComposer & erh
) {
   for (int j=0; j < NBR_OF_MODES; ++j) {  // delete all apVi entries
       apVi[j] = VectorItemModePtr::Nil;
   }
   new(this) VectorItemArray(dlmlst, ft, erh);
}

/*---------------------------------------------------DelimFinder::DelimFinder-+
| DelimFinder constructor                                                     |
+----------------------------------------------------------------------------*/
DelimFinder::DelimFinder(
   SgmlDecl const & sdcl, UnicodeComposer & erh
) :
   chs(sdcl.delimList().charset()),
   apViEntry(sdcl.delimList(), sdcl.features(), erh),
   iMaxBseqLen(sdcl.quantity()[Quantity::IX_BSEQLEN])
{
   reset();
}

/*-----------------------------------------------DelimFinder::resetDelimiters-+
| DelimFinder resetDelimiters                                                 |
+----------------------------------------------------------------------------*/
void DelimFinder::resetDelimiters(
   SgmlDecl const & sdcl, UnicodeComposer & erh
)
{
   pushMode(MODE_VOID);                           // to set correct pUcBase
   ((VectorItemArray*)&apViEntry)->resetDelimiters(  // cast away constness
      sdcl.delimList(), sdcl.features(), erh
   );
   popMode();
}

/*---------------------------------------------------------DelimFinder::reset-+
| Reset all states                                                            |
+----------------------------------------------------------------------------*/
void DelimFinder::reset()
{
   pUcsBase = apViEntry[rmode = MODE_INI]->f3.str;
   iLenDelim = 0;
   ixdl = Delimiter::IX_NOT_FOUND;
   ixMode = 0;
   iDepthMsrcd = 0;
   iDepthDcrcd = 0;
   iMarkedSectionDepth = 0;
   isNetEnabled = false;
}

/*----------------------------------------------------------DelimFinder::find-+
| Try to decode a delimiter on input, return its code (Delimiter::e_Ix)       |
|                                                                             |
| The recognition mode must have been properly set (set_mode)                 |
|                                                                             |
| IMPLEMENTATION NOTE:                                                        |
|      Due to the complexity of the Delimiters Recognition rules, with        |
|      B-sequence imbedded, the following code is a very tough one.           |
|      Change it with care!  Sloppy goto's are used to optimize               |
|      execution and (paradoxically) for readibility !                        |
|                                                                             |
| CODE ANNOTATIONS:                                                           |
|                                                                             |
| (1) Re: "A BSEQ cannot be preceded/followed by a blank"                     |
|     if pViAlt was set, this means that more than one alternate was          |
|     found, in violation of the rule stated above.                           |
|                                                                             |
| (2) Re: "only one BSEQ is allowed"                                          |
|     The BSEQ can be descended only one time, so we have no scruples         |
|     to chain the previous VI; this also acts as a protection for (3);       |
|                                                                             |
| (3) At this point, "in_bseq_recur" is always false, so we cannot decode     |
|     a B-seq more than 2 times.  Ad absurbo:                                 |
|     - if we were flunked in the search of the first BSEQ PCL,               |
|       the scanned char is reputed to be non-blank                           |
|       (all blanks have been exhausted  during the B-seq descent).           |
|       Then: "is_blank(curr_char)" is false.                                 |
|     - All other suceeding PCLs are reputed *not* to contain any B-seq,      |
|       since only one single B-seq is allowed.                               |
|       Then: "pVi->isBseq" is false.                                         |
|     Note: if the single B-seq Rule was not existing, this would have implied|
|           the stacking of prev_count, prev_ixdl, and pViPrev values.        |
|                                                                             |
+----------------------------------------------------------------------------*/
Delimiter::e_Ix DelimFinder::find(
   OpenEntityCache & oec, UCS_2 const * pUc
) {
   VectorItem const * pViCurr = apViEntry[rmode];
   VectorItem const * pViPrv = 0;             // Previous
   VectorItem const * pViAlt = 0;             // Alternate
   VectorItem const * pViFnd = 0;             // Found
   VectorItem const * pViFndBefore;           // Previously Found
   unsigned int iPegAlt;
   unsigned int iPegFnd = 0;
   unsigned int iPegFndBefore = 0;
   unsigned int iPegAfterBseq = 0;
   unsigned int iCount = 0;
   bool isRecurBseq = false;

prep_char_matches: //--------------------------------------------------------
   /*-----------------------------------------------------+
   | Found in the PCL: set up any alternate, and continue |
   +-----------------------------------------------------*/
   if (                                       // B conflict? (alternate found)
      pViCurr->f1.h.isBseq &&                 // see note (3)
      chs.isBlank(oec.peek(iCount)) &&
      !pViAlt                                 // PROTECTION: see Note (1)
   ) {
      pViAlt = pViCurr;                       // yes, set alternate path
      iPegAlt = iCount;                       // remember where
   }
   ++iCount;                                  // increase peek count
   pViCurr += 1 + (pUc - pViCurr->f3.str);    // get 2nd node

examine_cont_string: //------------------------------------------------------
   /*--------------------------------------+
   | Check the continuation string matches |
   +--------------------------------------*/
   pUc = pViCurr->f3.str;                     // point continuation string
   while (*pUc && (*pUc == oec.peek(iCount))) { // leave at eos
      ++pUc;
      ++iCount;                               // increase unread count
   }
   if (*pUc) {                                // eos not found (it's bad):
      if (isRecurBseq) goto try_another_B_dv;
      goto break_it;                          // no recursion: leave
   }                                          // eos found (it's ok):
   if (pViCurr->f2.ixdl != Delimiter::IX_NOT_FOUND) { // largest delim gotten
      if (check_contextual_constraint(pViCurr->f2.ixdl, oec, iCount)) {
         pViFnd = pViCurr;                    // get its VectorItem
         iPegFnd = iCount;
      }                                       // else, as not found...
      if (isRecurBseq) goto try_another_B_dv;
      goto break_it;                          // and leave...
   }
   if (
      ((pViCurr=pViCurr->f1.next)->f2.ixdl != Delimiter::IX_NOT_FOUND) &&
      check_contextual_constraint(pViCurr->f2.ixdl, oec, iCount)
   ) {                                        // may be found...
      pViFnd = pViCurr;                       // set may_be VectorItem
      iPegFnd = iCount;                       // peg unread here
   }
   /*----------------------------------------+
   | Examine the preponderant character list |
   +----------------------------------------*/
   {
      UCS_2 const uc = oec.peek(iCount);
      pUc = pViCurr->f3.str;
      do {
         if (*pUc == uc) goto prep_char_matches; // loop
      }while (*pUc++);
   }

   if (isRecurBseq) goto try_another_B_dv;    // see note (2)
   /*----------------------------------+
   | Not found in the PCL, first time: |
   | try a blank seq recognition       |
   +----------------------------------*/
   if (
      (pViCurr->f1.h.isBseq) &&               // an alternate path
      (chs.isBlank(oec.peek(iCount)))         // and Bseq,
   ) {
      goto decode_Bseq;                       // see note (3)
   }
   goto break_it;

try_another_B_dv: //---------------------------------------------------------
   /*-----------------------------------------------------------------+
   | Not found: while doing blank sequence nodes examinition,         |
   | go to another previous B-dv if needed and possible (see note (2) |
   | For a same count, prev_F is always more specific..               |
   +-----------------------------------------------------------------*/
   if (iPegFnd > iPegFndBefore) {             // between both...
      iPegFndBefore = iPegFnd;                // choose the longest
      pViFndBefore = pViFnd;
   }
   pViCurr = pViPrv = (pViPrv+1)->f1.previous;
   if (!pViCurr) {                            // not possible
      iPegFnd = iPegFndBefore;                // set final answer
      pViFnd = pViFndBefore;
      goto break_it;
   }
   iCount = iPegAfterBseq;                    // back where we were
   goto examine_cont_string;

decode_Bseq: //--------------------------------------------------------------
   for (;;) {
      /*--------------------------------------------+
      | descend thru the maximum of B-sequence node |
      +--------------------------------------------*/
      pViCurr += pViCurr->f1.h.count + 1;     // point BSEQ node
      unsigned int iMinBseq = (pViCurr+1)->f2.iLenBseq; // get min BSEQ length
      unsigned int iMaxBseq = iCount + iMaxBseqLen;
                                              // skip minimum blanks
      while (chs.isBlank(oec.peek(++iCount)) && --iMinBseq)
         ;
      switch(iMinBseq) {
      case 0:                                 // more than min blanks found
         if ((pViCurr+1)->f3.isRecurse) {     // more B specificity:
            pViCurr = pViCurr->f1.next;       // chain next node
            continue;                         // try it...
         }                                    // no more B-sequence node:
                                              // skip all others blanks
         while (chs.isBlank(oec.peek(++iCount)) && (iCount < iMaxBseq))
            ;
         break;
      case 1:                                 // exactly min blanks found
         break;
      default:                                // less than min blanks found
         pViCurr = (pViCurr+1)->f1.previous;  // back to previous
         break;                               // we'll check pViCurr later
      }
      break;
   }
   if ((pViPrv = pViCurr) != 0) {
      isRecurBseq = true;
      iPegFndBefore = iPegFnd;                // save F_peg
      pViFndBefore = pViFnd;                  // and pViFnd
      iPegAfterBseq = iCount;                 // peg unread(s) here
      goto examine_cont_string;
   }

break_it: //-----------------------------------------------------------------
   if (pViAlt) {                              // Alternate Bseq co-exists
      iCount = iPegAlt;
      pViCurr = pViAlt;                       // go try it
      pViAlt = 0;
      goto decode_Bseq;                       // one time only....
   }
   if (pViFnd) {                              // delimiter found:
      oec.skip(iLenDelim = iPegFnd);          // iPegFnd has the length
      ixdl = pViFnd->f2.ixdl;
   }else {
      ixdl = Delimiter::IX_NOT_FOUND;
   }
   return ixdl;
}

/*PRIVATE----------------------------DelimFinder::check_contextual_constraint-+
| Check contextual constraints for a delimiter to be recognized.              |
| `ixdl' identifies the delimiter the contextual constraints of which need    |
| to be checked.                                                              |
| if (the delimiter is recognizable) return true  else return false;          |
|                                                                             |
| When the current element has a declared content of RCDATA, the mode         |
| is RCD_MO and `cn_ent_level' records the entity nesting level               |
| reached at the time the current element started.                            |
|                                                                             |
| Within a Marked Section containing RCDATA, the mode is RCDMS_MO.            |
| `ms_ent_level' records the entity nesting level reached at the time         |
| the Marked Section started.                                                 |
|                                                                             |
+----------------------------------------------------------------------------*/
bool DelimFinder::check_contextual_constraint(
   Delimiter::e_Ix ixdlArg, OpenEntityCache & oec, int iCount
) const
{
   switch (ixdlArg) {

   case Delimiter::IX_CRO:                      // CREF: digit or namestart
      if (chs.isCref(oec.peek(iCount))) return true;
      return false;

   case Delimiter::IX_HCRO:                     // Hex CREF: hex digit
      if (UnicodeExtractor::isHexDigit(oec.peek(iCount))) return true;
      return false;

   case Delimiter::IX_ERO:                      // NMS: namestart
   case Delimiter::IX_MDO:                      // DCL: namestart
   case Delimiter::IX_PERO:                     // NMS: namestart
      if (!chs.isNamestart(oec.peek(iCount))) {
         return false;
      }
      return true;

   case Delimiter::IX_STAGO:                    // GI: namestart, or 7.4.1.2
      /*---------------------------------------------+
      | if in MODE_TAG                               |
      |    always true (no constraints)              |
      |    (see ISO 8879, 7.4.1.2 and 7.4.5.2)       |
      | else (CON_MO, RCD_MO, FCD_MO)                |
      |    true iff followed by a name_start char    |
      |    except for RCDATA-content within a nested |
      |    entity.                                   |
      |                                              |
      | Note that STAGO is dl_found only for CON_MO  |
      |      (not for RCD_MO, neither FCD_MO)        |
      +---------------------------------------------*/
      if (rmode == MODE_TAG) return true;
      if (
         (!(chs.isNamestart(oec.peek(iCount)))) ||
         ((rmode == MODE_CON_RCDATA) && (oec.iDepth > iDepthDcrcd))
      ) {
         return false;
      }
      return true;

   case Delimiter::IX_ETAGO:                    // GI: namestart, or 7.4.5.2
      if (rmode == MODE_TAG) return true;
      if (
         (!(chs.isNamestart(oec.peek(iCount)))) ||
         ((rmode == MODE_CON_RCDATA) && (oec.iDepth > iDepthDcrcd))
      ) {
         return false;
      }
      if ((rmode == MODE_CON_CDATA) || (rmode == MODE_CON_RCDATA)) {
         return giMatch(oec, iCount);
      }
      return true;

   case Delimiter::IX_ETAGO_TAGC:
   case Delimiter::IX_ETAGO_GRPO:
      /*---------------------------------------------+
      | true except for RCDATA-content, within a     |
      | nested entity.                               |
      +---------------------------------------------*/
      if (rmode == MODE_TAG) return true;
      if ((rmode == MODE_CON_RCDATA) && (oec.iDepth > iDepthDcrcd)) {
         return false;
      }
      return true;

   case Delimiter::IX_NET:                      // ELEM (special)
      /*-------------------------------------------------+
      | NESTC is in MODE_TAG only and has no constraints |
      |    (see ISO 8879, 7.4.1.2 and 7.4.5.2)           |
      | NET:                                             |
      | (MODE_CON || MODE_CON_RCDATA || MODE_CON_CDATA)  |
      |  true iff within an element whose                |
      |  start-tag was a net-enabling start tag          |
      |  except for MODE_RCD (RCDATA elem content)       |
      |  within a nested entity.                         |
      +-------------------------------------------------*/
      if ((rmode == MODE_CON_RCDATA) && (oec.iDepth > iDepthDcrcd)) {
         return false;
      }
      if (!isNetEnabled) return false;
      return true;

   case Delimiter::IX_MSC_MDC:
      /*-----------------------------------------------+
      | MSC_MDC cannot be recognized as such if it     |
      | occurs in an RCDATA marked section at a higher |
      | entity level than the one the marked section   |
      | started with.                                  |
      +-----------------------------------------------*/
      if ((rmode == MODE_MS_RCDATA) && (oec.iDepth > iDepthMsrcd)) {
         return false;
      }
      return true;

   default:
      return true;                              // any character type
   }
}

/*-------------------------------------------------------DelimFinder::giMatch-+
| Find if the GI following a ETAGO is a candidate to end declared content     |
| started by the tag the GI of which is in "giData."                          |
| In clear: find if the two GI's match.                                       |
+----------------------------------------------------------------------------*/
bool DelimFinder::giMatch(OpenEntityCache & oec, int iCount) const
{
   if (giData.empty()) return true;
   int iMax = 1+giData.length();
   UCS_2 const * pGiData = giData;
   while (--iMax) {
      if (chs.uppercase(oec.peek(iCount++)) != *pGiData++) {
         return false;
      }
   }
   return !chs.isName(oec.peek(iCount));
}

/*----------------------------------------------------DelimFinder::setConMode-+
| Set Data Content modes (#PCDATA, or declared element content)               |
+----------------------------------------------------------------------------*/
void DelimFinder::setConMode(
   e_RecognitionMode rmodeNew,
   bool isNetToEnable,
   int iDepth,
   UnicodeString const & gi
) {
   assert  (
      (rmodeNew == MODE_CON)        ||  // at init time, only!
      (rmodeNew == MODE_CON_XML)    ||  // at init time, only!
      (rmodeNew == MODE_CON_RCDATA) ||
      (rmodeNew == MODE_CON_CDATA)
   );
   isNetEnabled = isNetToEnable;
   switch (rmodeNew) {
   case MODE_CON_RCDATA:
      modeStack[ixMode++] = rmode;  // push!
      iDepthDcrcd = iDepth;
      giData = gi;
      break;
   case MODE_CON_CDATA:
      modeStack[ixMode++] = rmode;  // push!
      giData = gi;
      break;
   case MODE_CON_XML:
      switch (rmode) {
      case MODE_CON_XML:
         if (!isNetToEnable) return;
         rmodeNew = MODE_CON_XML_WNET;
         break;
      case MODE_CON_XML_WNET:
         if (isNetToEnable) return;
         // rmodeNew = MODE_CON_XML;
         break;
      case MODE_MS_INCLUDE:
         if (!isNetToEnable) return;
         rmodeNew = MODE_CON_NOSR;
         break;
      case MODE_CON_NOSR:
         if (isNetToEnable) return;
         rmodeNew = MODE_MS_INCLUDE;
         break;
      case MODE_CON_RCDATA:         // pop!
      case MODE_CON_CDATA:          // pop!
         rmodeNew = modeStack[--ixMode];
         break;
      case MODE_CON:
         rmodeNew = MODE_CON;
      default:
         break;
      }
   default: // case MODE_CON:
      break;
   }
   pUcsBase = apViEntry[rmode = rmodeNew]->f3.str;
}

/*----------------------------------------------------DelimFinder::pushMsMode-+
| Set Marked Section modes                                                    |
+----------------------------------------------------------------------------*/
void DelimFinder::pushMsMode(
   e_RecognitionMode rmodeNew, int iDepth
) {
   assert  (ixMode < MAX_MODE_LVL);
   assert  (
      (rmodeNew == MODE_MS_RCDATA)  ||
      (rmodeNew == MODE_MS_CDATA)   ||
      (rmodeNew == MODE_MS_IGNORE)  ||
      (rmodeNew == MODE_MS_INCLUDE)
   );
   ++iMarkedSectionDepth;
   switch (rmodeNew) {
   case MODE_MS_RCDATA:
      modeStack[ixMode++] = rmode;  // push!
      iDepthMsrcd = iDepth;
      break;
   case MODE_MS_INCLUDE:
      // MODE_MS_INCLUDE really is a mode where
      // no NET, no ShortRefs, but "]]>" is recognized.
      switch (rmode) {
      case MODE_CON_XML:
         // rmodeNew = MODE_MS_INCLUDE;
         break;
      case MODE_CON_XML_WNET:
         rmodeNew = MODE_CON_NOSR;
         break;
      default:
         return;
      }
      break;
   default:     // CDATA, IGNORE
      modeStack[ixMode++] = rmode;  // push!
   }
   pUcsBase = apViEntry[rmode = rmodeNew]->f3.str;
}

/*-----------------------------------------------------DelimFinder::popMsMode-+
| Pop Marked Section modes                                                    |
+----------------------------------------------------------------------------*/
bool DelimFinder::popMsMode()
{
   if (!iMarkedSectionDepth) return false;

   if (--iMarkedSectionDepth == 0) {
      switch (rmode) {
      case MODE_MS_INCLUDE:
         rmode = MODE_CON_XML;
         break;
      case MODE_CON_NOSR:
         rmode = MODE_CON_XML_WNET;
         break;
      case MODE_MS_RCDATA:          // pop!
      case MODE_MS_CDATA:           // pop!
      case MODE_MS_IGNORE:          // pop!
         rmode = modeStack[--ixMode];
         break;
      default:
         return true;
      }
      pUcsBase = apViEntry[rmode]->f3.str;
   }
   return true;
}
/*===========================================================================*/
