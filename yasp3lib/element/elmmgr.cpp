/*
* $Id: elmmgr.cpp,v 1.10 2011-07-29 10:26:39 pgr Exp $
*
* Open-Element stack, element lists and validation functions.
*
* Routines to manage the DTD and document structure.
* Includes storing and matching declarations from the DTD,
* and keeping a stack of open elements, with needed parsing info.
*
* Overview:
*
* These routines support information learned from parsing the DTD,
* and testing the later document against it.
*
* This is where further validation functions will be tied in.
*
* Also in dtd.c is code for maintaining the stack of open elements
* while parsing, which is used to insert omitted end-tags, to detect
* certain kinds of tagging errors (such as closing an element which was
* never opened), to interpret EMPTY tags properly, etc.
*
* Whenever a start-tag is found or implied, the tag type is stacked.
* Whenever an end-tag is found or implied, the stack is popped.
* The fact that tag types are on the stack allows YSP to test whether a
* correct end-tag has been found.
*
* If an end-tag is found for something open, but not the topmost thing,
* we presume that any intervening end-tags were left omitted,
* and will supply them to the caller (having to return the omitted tag
* as an event means that the actual tag found becomes a pendingEvent
* and will be returned on a later call).
*/

/*---------+
| Includes |
*---------*/
#include <new>
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"
#include "../syntax/sdcl.h"
#include "elmmgr.h"
#include "element.h"

UnicodeString const ElmManager::ucsRootName("#ROOT");
OpenElement const OpenElement::Nil;

/*---------------------------------------------------OpenElement::OpenElement-+
|                                                                             |
+----------------------------------------------------------------------------*/
OpenElement::OpenElement(
   Tag const * pTag
) :
   tagptr(pTag),
   lctr(pTag->inqElement().inqModel()),
   isDataAllowed(false)         // until welcomeElement(0) is called
{
}

/*--------------------------------------------------ElmManager::SignalEventCB-+
| Virtual (have no actions)                                                   |
+----------------------------------------------------------------------------*/
ElmManager::SignalEventCB::~SignalEventCB() {
}
void ElmManager::SignalEventCB::startTag(Tag const *, bool) {
}
void ElmManager::SignalEventCB::endTag(Tag const *) {
}

/*-----------------------------------------ElmManager::State::setFirstElement-+
| Change the values of the first element, aka #ROOT.                          |
+----------------------------------------------------------------------------*/
bool ElmManager::State::setFirstElement(ModelTree const & model)
{
   // following cast violates constness.  we have to change #ROOT
   Element & elm = (Element &)stack.first()->inqTag().inqElement();
   ElmContent content(model, elm.inqInclusions(), elm.inqExclusions());
   elm.attachContent(content, false, false);

   stack.first()->lctr = ModelLocator(elm.inqModel());
   stack.first()->isDataAllowed = false;
   return true;
}

/*----------------------------------------------------ElmManager::State::push-+
| Effects:                                                                    |
|    Push a new open element onto the stack.                                  |
+----------------------------------------------------------------------------*/
inline bool ElmManager::State::push(
   Tag const * pTag,
   SignalEventCB & cb,
   bool isEmpty
) {
   if (!isEmpty) {                         // don't push empty elements!
      if (pTag->isNetStart()) ++iNetActives;
      stack += new OpenElement(pTag);
      assert (stack);
   }
   cb.startTag(pTag, isEmpty);
   return true;
}

/*-----------------------------------------------------ElmManager::State::pop-+
| Effects:                                                                    |
|    Pop the last open element out of the stack.                              |
|                                                                             |
| If isForce is false, the no error are permitted, and, if one occurs         |
| the stack is unchanged.  However, caller needs to restore the locator.      |
|                                                                             |
| Record the 'last tag closed' since it's needed for SHORTTAG                 |
| to assign the right GI for null start tags.                                 |
+----------------------------------------------------------------------------*/
bool ElmManager::State::pop(
   SignalEventCB & cb, bool isForce, Tag::Minimize mini
) {
   OpenElement * pOelm = inqLastOpenElement();
   ModelLocator & lctr = pOelm->lctr;
   Tag * pTag = pOelm->inqTagPtr();
   *pTag << mini;
   switch (lctr.isValidEnd()) {
   case MDLLCTR_CLOSED:
      break;
   default: // case MDLLCTR_UNCLOSABLE, MDLLCTR_MISSING
      if (!isForce) return false;  // but caller needs to restore the locator!
      (*pTag) << Tag::InvalidEnd;
      break;                       // pursue, anyway
   }
   pElmLastClosed = pTag->inqElementPtr();
   if (pTag->isNetStart()) --iNetActives;
   cb.endTag(pTag);
   --stack;
   return true;
}

/*-------------------------------------------------ElmManager::State::cleanup-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ElmManager::State::cleanup()
{
   pElmLastClosed = 0;
   iNetActives = 0;
   while (stack.count() && stack.removeLast())
      ;
}

/*---------------------------------------------ElmManager::State::copyLocator-+
| Kinda copy on write                                                         |
+----------------------------------------------------------------------------*/
inline ModelLocator & ElmManager::State::copyLocator(
   bool & isTrueBackup
) {
   OpenElement * pOelm = stack.last();
   if (
      (isTrueBackup = stack.isItemShared(pOelm), isTrueBackup) &&
      !stack.replaceAt(pOelm = new OpenElement(*pOelm), stack.count()-1)
   ) {
      return (ModelLocator &)ModelLocator::Nil; // cast away constness
   }
   return pOelm->lctr;
}

/*-----------------------------------------ElmManager::State::checkExceptions-+
| Inclusion/Exclusion:                                                        |
| If the element starting has never been declared,                            |
| assume it is valid but not proper (force inclusion).                        |
+----------------------------------------------------------------------------*/
inline void ElmManager::State::checkExceptions(
   Element const * pElm, bool & isExclusion, bool & isInclusion
) const {
   isExclusion = false;
   isInclusion = false;
   if (pElm) {             // #PCDATA cannot be excluded/included
      if (!pElm->isDefined()) {
         isInclusion = true;
      }else {
         int iExcept = stack.last()->inqTag().checkExceptions(pElm);
         if (iExcept < 0) {
            isInclusion = true;
         }else if (iExcept > 0) {
            isExclusion = true;
         }
      }
   }
}

/*-------------------------------------------ElmManager::State::endWouldAllow-+
| Check if ending the last element would allow "ixElmNew".                    |
|                                                                             |
| When entering this routine, we know that ixElmNew is not allowed            |
| by the content model of the current element.                                |
|                                                                             |
| 1) ask the parent of this current element if the isElmNew would             |
|    be the best candidate.  Ig yes, we can safely close the current          |
|    element: the new element is its best brother.                            |
|    Return WCOND_HIT.                                                        |
|                                                                             |
| 2) if this doesn't work, see if ixElmNew is again the same element          |
|    than the one which is current.  If so, verify if the current element     |
|    was started invalidly, because if couldn't fit at any place on.          |
|    the stack of open elements.  If it was invalid, then caller can close    |
|    the current element, and restart the new one which is as invalid         |
|    as its brother was!                                                      |
|    Return WCOND_FORCED.                                                     |
|                                                                             |
|    Example:                                                                 |
|       <doc><chap><foo>   is a new foo allowed?                              |
|       Current <foo> was not allowed (it was an invalid start):              |
|       close it and pretend the new <foo> is OK                              |
|       (but turn on its "invalid start" bit.)                                |
|                                                                             |
|    If we didn't do this, then we might end in zillions of <foo>             |
|    stacked one over each other (EPSqa22118)                                 |
|                                                                             |
| 3) If this doesn't work,  caller needs to pursue its search                 |
|    by closing enough opened element, until one fits.                        |
|    Return WCOND_INVALID.                                                    |
|                                                                             |
+----------------------------------------------------------------------------*/
inline ElmManager::e_WelcomeCond ElmManager::State::endWouldAllow(
   int ixElmNew
) const {
   int const ixAntePenultiemElm = stack.count() - 2;
   Tag const * pTag;
   if ((ixAntePenultiemElm > 0) &&
      (stack[ixAntePenultiemElm]->lctr.isBestCandidate(ixElmNew))
   ) {
      return WCOND_HIT;
   }else if (
      pTag = stack[stack.count() - 1]->inqTagPtr(),
      ((ixElmNew == pTag->inqElement().inqIx()) && pTag->isInvalidStart())
   ) {
      return WCOND_FORCED;
   }else {
      return WCOND_INVALID;
   }
}

/*-----------------------------------------------------ElmManager::ElmManager-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
ElmManager::ElmManager(
   UnicodeComposer & erhArg
) :
   erh(erhArg), bOk(true), ixBase(-1), iFloorDepth(0)
{
   if (!initRoot()) bOk = false;
}

/*-------------------------------------------------------ElmManager::initRoot-+
| define #ROOT element and open an instance on the stack                      |
| later, setRootModel sets its content model to: <.. (doctypeGi)>             |
| Note:                                                                       |
|   defineElementIfNotThere is used in case of xple openDocument              |
+----------------------------------------------------------------------------*/
Element * ElmManager::initRoot()
{
   s.cleanup();
   SignalEventCB cbDummy;
   Element * pElm = defineElementIfNotThere(ucsRootName);
   bOk = (pElm && s.push(new Tag(pElm, pElm->inqAttlist()), cbDummy, false));
   assert (bOk);
   return pElm;
}

/*----------------------------------------------------------ElmManager::reset-+
| Reset Elm to handle a new instance                                          |
+----------------------------------------------------------------------------*/
bool ElmManager::reset()
{
   if (!bOk) {                             // not ok?
      new(this) ElmManager(erh);           // try again...
   }else {
      Element * pElmRoot = initRoot();
      if (pElmRoot) {
         s.inqLastOpenElement()->lctr = ModelLocator(pElmRoot->inqModel());
      }
   }
   return bOk;
}

/*--------------------------------------------------ElmManager::defineElement-+
| Effects:                                                                    |
|    Adds a GI and its associated Element structure to the list.              |
|                                                                             |
| Notes:                                                                      |
|   defineElement is called:                                                  |
|                                                                             |
|   1) When an ELEMENT is defined.                                            |
|      If the GI has already been seen AND if it was because of               |
|      another ELEMENT decl, then scream ELM__ALREADY_DEFINED.                |
|                                                                             |
|   2) To create a temporary place holder (by defineElementIfNotThere)        |
|      For example, when an ATTLIST is attached (defined).                    |
|      Before to call, the caller *must* insure that the Element structure    |
|      does *not* already exist.  For example, find out DUPLICATE_ATTLIST.    |
|                                                                             |
| Returns:                                                                    |
|    The Element structure (0 if bad)                                         |
+----------------------------------------------------------------------------*/
Element * ElmManager::defineElement(
   UnicodeString const & ucsGi, // Tag name to add
   ElmContent const & content,
   bool omitStart,           // true = omittable
   bool omitEnd
) {
   Element * pElm = elmlst[ucsGi];
   if (!pElm) {                 // brand new!
      pElm = new Element(ucsGi, elmlst.count(), content, omitStart, omitEnd);
      elmlst.insert(pElm);
   }else {                      // GI already seen (ELEMENT, ATTLIST, or other)
      if (pElm->isDefined()) return 0;
      pElm->attachContent(content, omitStart, omitEnd);
   }
   return pElm;
}

Element * ElmManager::defineElement(
   int ix,                      // Index in list
   ElmContent const & content,
   bool omitStart,           // true = omittable
   bool omitEnd
) {
   Element * pElm = elmlst[ix];
   if (pElm->isDefined()) return 0;
   pElm->attachContent(content, omitStart, omitEnd);
   return pElm;
}

/*----------------------------------------ElmManager::defineElementIfNotThere-+
| Effects:                                                                    |
|    Find or Define an Element structure associated to a GI.                  |
|                                                                             |
| "defineElementIfNotThere" checks that the Element structure does not        |
| already exists (see notes in "defineElement".)                              |
| It is called:                                                               |
| - for each GI attached to an ATTLIST                                        |
| - for each GI in an exclusion/inclusion list -- so to get the id's and      |
|   condense the exception list into an array of ids.                         |
| - for each undefined element found in instance -- so we do no more fiddle   |
|   dangerously with null pointers and OpenGIs on the element stack.          |
+----------------------------------------------------------------------------*/
Element * ElmManager::defineElementIfNotThere(
   UnicodeString const & ucsGi            // Tag name to add
) {
   Element * pElm = elmlst[ucsGi];
   if (!pElm) {
      pElm = defineElement(ucsGi, ElmContent::Undefined, true, true);
   }
   return pElm;
}

/*-PRIVATE------------------------------------------ElmManager::isEnoughDepth-+
| Effects:                                                                    |
|    Check if we do not pass under the floor...                               |
+----------------------------------------------------------------------------*/
inline bool ElmManager::isEnoughDepth() const {
   if (iFloorDepth && (s.inqCount() <= iFloorDepth)) {
      return false;
   }else {
      return true;
   }
}

/*-PRIVATE-----------------------------------------ElmManager::endLastElement-+
| Effects:                                                                    |
|    Close the last element.                                                  |
|                                                                             |
| Disabled (pgr, 7/26/96): if missing element, recovery was:                  |
| case MDLLCTR_MISSING:          // an element is required                    |
|    {                                                                        |
|       Element const * pElm = elmlst[lctr.peekMissing()];                    |
|       RefdItem tagptr(new Tag(pElm, pElm->inqAttlist()));                  |
|       startElement(tagptr, cb);                                             |
|    }                                                                        |
|    if (iCount > 0) ++iCount;   // bug! empty element not on the stack!      |
|    lctr.hitMissing();          // Hit it!                                   |
|    break;                                                                   |
+----------------------------------------------------------------------------*/
inline bool ElmManager::endLastElement(
   SignalEventCB & cb, Tag::Minimize mini
) {
   if (!isEnoughDepth()) {
      erh << ECE__ERROR << _ELM__ENDNOTSYNCH
         << inqElementLastOpened()->key() << endm;
      return false;
   }else {
      s.pop(cb, true, mini);
      return true;
   }
}

/*---------------------------------------------------ElmManager::startElement-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool ElmManager::startElement(
   RefdItem tagptr,
   SignalEventCB & cb,
   bool isEmpty                         // because CONREF. default is false
) {
   Tag * pTag = (Tag *)tagptr.inqData();
   OpenElement * pOelmCurrent = s.inqLastOpenElement();

   pOelmCurrent->isDataAllowed = false;    // reset flag
   pTag->setParent(pOelmCurrent->inqTag());
   ModelTree const & mdl = pTag->inqElement().inqModel();
   if (mdl.isEmpty()) {
      isEmpty = true;                      // could already be true if CONREF
   }
   return s.push(pTag, cb, isEmpty);
}

/*-------------------------------------------------ElmManager::endAllElements-+
| End all elements (back to #ROOT)                                            |
+----------------------------------------------------------------------------*/
bool ElmManager::endAllElements(SignalEventCB & cb, int iMax)
{
   int iToEnd = s.inqCount()-1;
   if (iToEnd > 0) {
      if (iMax) {
        if ((iMax > iToEnd) || (iMax < 0)) {
           assert  (false);
           return false;
        }
        iToEnd = iMax;
      }
      do {
         if (!endLastElement(cb)) {
            return false;
         }
      }while (--iToEnd);
      assert  ((iMax != 0) || (s.inqCount() == 1));
      return true;
   }else {
      return false;
   }
}

/*-----------------------------------------------------ElmManager::endElement-+
| End a given element                                                         |
+----------------------------------------------------------------------------*/
bool ElmManager::endElement(
   Element const * pElmGiven,
   SignalEventCB & cb
) {
   int iToEnd = 0;
   if (s.inqCount() > 1) {
      Tag * pTag = s.inqLastOpenElement()->inqTagPtr();
      iToEnd = pTag->checkEndElement(pElmGiven);
      if ((!iToEnd) && (pTag->inqElement().inqModel().isDcdata())) {
         /*
         | Handle invalid end tag if the current element
         | is DCDATA.  See: 7.6, p.24, last paragraph
         */
         (*pTag) << Tag::FakedEnd;
         endLastElement(cb);               // so it ends ones
         return false;
      }
   }
   if (iToEnd) {
      while (--iToEnd) {
         if (!endLastElement(cb)) {
            return false;
         }
      }
      return endLastElement(cb, Tag::Regular);
   }else {
      return false;
   }
}

/*-----------------------------------------------------ElmManager::endElement-+
| End the last element                                                        |
+----------------------------------------------------------------------------*/
bool ElmManager::endElement(SignalEventCB & cb)
{
   if (s.inqCount() > 1) {
      return endLastElement(cb, Tag::Empty);
   }else {
      return false;
   }
}

/*--------------------------------------------------ElmManager::endElementNet-+
| End most recent NET enabling start-tag                                      |
+----------------------------------------------------------------------------*/
bool ElmManager::endElementNet(
   SignalEventCB & cb
) {
   int iToEnd = 0;
   if (s.inqCount() > 1) {
      iToEnd = s.inqLastOpenElement()->inqTagPtr()->checkEndNet();
   }
   if (iToEnd) {
      while (--iToEnd) {
         if (!endLastElement(cb)) {
            return false;
         }
      }
      return endLastElement(cb, Tag::Net);
   }else {
      return false;
   }
}

/*-------------------------------------------------ElmManager::welcomeElement-+
| This prepares a valid context to enter an element.                          |
| This does NOT enter the element itself (but can stack events)               |
|                                                                             |
| If the element is zero, this rather is the <#PCDATA> tag.                   |
|                                                                             |
| Each intruded start or end tag is signaled thru the fctReport function.     |
|                                                                             |
| Returns:  WCOND_...                                                         |
|   HIT:        everything is now fine                                        |
|   FORCED:     just another occurence of an already reported invalid element |
|               (read comments in endWouldAllow()                             |
|   INCLUDED:   the element is valid as an inclusion                          |
|   INVALID:    the element is not valid.                                     |
+----------------------------------------------------------------------------*/
ElmManager::e_WelcomeCond ElmManager::welcomeElement(
   Element const * pElmNew,        // can be 0
   SignalEventCB & cb
) {
   int const ixElmNew = pElmNew? pElmNew->inqIx() : -1;
   e_WelcomeCond wcond = WCOND_HIT;
   e_MdllctrStatus st;
   ModelLocator lctrBackup;
   {
      /* OPTIMIZATION
      | Quick processing of the 2 most frequently found cases:
      | - an immediate hit, or an inclusion
      | - the valid repetition of an (unclosed) start tag
      */
      ModelLocator & lctrActive = s.inqLastOpenElement()->lctr;
      lctrBackup = lctrActive;
      {
         bool isExclusion, isInclusion;
         s.checkExceptions(pElmNew, isExclusion, isInclusion);
         st = lctrActive.isValidStart(ixElmNew, isExclusion, isInclusion);
      }
      switch (st) {
      case MDLLCTR_CLOSED:
         wcond = s.endWouldAllow(ixElmNew);
         if ((wcond == WCOND_INVALID) || !isEnoughDepth()) {
            break;                                // pursue the quest
         }
         s.pop(cb, true);
         assert  (
            (wcond == WCOND_FORCED) || (
               (wcond == WCOND_HIT) &&
               (MDLLCTR_HIT ==
                  s.inqLastOpenElement()->lctr.isValidStart(ixElmNew, 0, 0)
               )
            )
         );
         /* fall thru */
      case MDLLCTR_HIT:
         if (ixElmNew == -1) {
            s.inqLastOpenElement()->isDataAllowed = true;
         }
         return wcond;
      case MDLLCTR_INCLUDED:
         lctrActive = lctrBackup;               // restore active model
         return WCOND_INCLUDED;
      default:
         break;
      }
   }
   bool isFirstTime = true;
   MsgTemplateId ercode = _ELM__NOERROR;
   ElmManager::State sBackup(s);
   for (;;) {
      bool isTrueBackup;
      ModelLocator & lctrActive = s.copyLocator(isTrueBackup);
      if (isFirstTime) {                        // lctrActive already set
         assert  (isTrueBackup);
         wcond = WCOND_INVALID;
         sBackup.inqLastOpenElement()->lctr = lctrBackup; // restore to backup
         isFirstTime = false;
      }else {
         lctrBackup = lctrActive;               // needed for INCLUDED
         bool isExclusion, isInclusion;
         s.checkExceptions(pElmNew, isExclusion, isInclusion);
         st = lctrActive.isValidStart(ixElmNew, isExclusion, isInclusion);
      }
      switch (st) {
      case MDLLCTR_MISSING:
         {
            Element const * pElmMissing = elmlst[lctrActive.peekMissing()];
            RefdItem tagptr(new Tag(pElmMissing, pElmMissing->inqAttlist()));
            int kind = pElmMissing->inqModel().inqKind();
            if (
               ((kind & (ModelTree::Cdata | ModelTree::Rcdata)) && (pElmNew)) ||
               (kind & ModelTree::Empty)
            ) {
               /*
               | If a CDATA/RCDATA tag is missing, the STAGO of pElmNew
               | won't be recognized.   EMPTY tags cannot be implied.
               */
               erh << pElmMissing->key();
               if (kind & ModelTree::Empty) {
                  ercode = _ELM__INVSTGEMPTY;
               }else {
                  ercode = _ELM__INVSTGCDATA;
               }
            }else {
               lctrActive.hitMissing();         // Hit the missing element
               startElement(tagptr, cb);
               continue;
            }
         }
         break;                                 // exit: invalid.

      case MDLLCTR_UNCLOSABLE:
         ercode = _ELM__INVSTGCLOSE;
         erh << inqElementLastOpened()->key();
         break;                                 // exit: invalid.

      case MDLLCTR_CLOSED:
         if (s.inqCount() <= 2) {               // no more elements to try
            ercode = _ELM__INVSTGOOC;
         }else if (!isTrueBackup) {
            ercode = _ELM__INVSTGNOCNT;
            erh << inqElementLastOpened()->key();
         }else if (!isEnoughDepth()) {
            ercode = _ELM__STGNOTSYNCH;
            erh << inqElementLastOpened()->key();
         }else {
            Tag const & tag = inqTagLastOpened();
            if (
               (ixElmNew == tag.inqElement().inqIx()) && tag.isInvalidStart()
            ) {
               s.pop(cb, true);
               wcond = WCOND_FORCED;            // see endWouldAllow()
            }else {
               s.pop(cb, true);
               continue;                        // try again
            }
         }
         break;                                 // exit: invalid, or forced

      case MDLLCTR_EXCLUDED:
         lctrActive = lctrBackup;               // restore active model
         if (!isEnoughDepth()) {
            ercode = _ELM__STGNOTSYNCH;
            erh << inqElementLastOpened()->key();
         }else if (!s.pop(cb, false)) {         // try to end it...
            ercode = _ELM__INVSTGCLOSE;         // UNCLOSABLE or MISSING
            erh << inqElementLastOpened()->key();
         }else {
            continue;                           // success!
         }
         break;                                 // exit: invalid.

      case MDLLCTR_INCLUDED:
         wcond = WCOND_INCLUDED;
         lctrActive = lctrBackup;               // restore active model
         break;                                 // exit: success.

      case MDLLCTR_HIT:
         wcond = WCOND_HIT;
         break;                                 // exit: success.

      default:  // MDLLCTR_INTERNAL_ERROR:
         assert (false);
         break;
      }
      break;
   }
   if (wcond == WCOND_INVALID) {
      s = sBackup;             // restore stack
   }
   if (ercode != _ELM__NOERROR) {
      if (!pElmNew) {          // PCDATA: use slightly different message
         ercode = (MsgTemplateId)((int)ercode + 1);
      }else {
         erh << pElmNew->key();
      }
      erh << ECE__ERROR << ercode << endm;
   }
   if (ixElmNew == -1) {
      s.inqLastOpenElement()->isDataAllowed = true;
   }
   return wcond;
}

/*---------------------------------------------------ElmManager::setRootModel-+
| Reset the dummy element to <!ELEMENT #ROOT - - (doctype)> + lctr starting   |
+----------------------------------------------------------------------------*/
bool ElmManager::setRootModel(UnicodeString const & ucsDoctype)
{
   ixBase = defineElementIfNotThere(ucsDoctype)->inqIx();

   ModelTree model;
   model << Delimiter::IX_GRPO << ixBase << Delimiter::IX_GRPC << endm;

   return s.setFirstElement(model);
}

/*---------------------------------------------ElmManager::inqRecognitionMode-+
| Effects:                                                                    |
|    Get the appropriate recognition associated to the current element        |
+----------------------------------------------------------------------------*/
e_RecognitionMode ElmManager::inqRecognitionMode() const
{
   if (s.inqCount()) {
      ModelTree const & mdl = inqTagLastOpened().inqElement().inqModel();
      if (mdl.isDcdata()) {
         if (mdl.isCdata()) {  // non replaceable
            return MODE_CON_CDATA;
         }else {
            return MODE_CON_RCDATA;
         }
      }else {
         return MODE_CON_XML; // delimfind knows if its real CON
      }
   }else {
      return MODE_INI;
   }
}

/*-------------------------------------------------ElmManager::setListElement-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ElmManager::setListElement(
   RefdKey const & doctype,
   ElementList const & source
) {
   elmlst = source;
   setRootModel(doctype.key());
}

/*-----------------------------------------------ElmManager::debugElementList-+
| Effects:                                                                    |
|    Show the element list                                                    |
| TEMPORARY - it has to be operator<<                                         |
+----------------------------------------------------------------------------*/
void ElmManager::debugElementList(ostream & out) const {
#if !defined MWERKS_NEEDS_Q_FIX
   int const iLast = elmlst.count();
   for (int i=0; i < iLast; ++i) {
       out << elmlst[i]->key() << ' ' << elmlst[i]->inqIx() << endl;
   }
#endif
}
/*===========================================================================*/
