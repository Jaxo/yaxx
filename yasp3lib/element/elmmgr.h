/*
* $Id: elmmgr.h,v 1.11 2002-04-14 23:26:04 jlatone Exp $
*
* Element Manager of YSP
*/

#if !defined ELMMGR_HEADER && defined __cplusplus
#define ELMMGR_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "element.h"
#include "../../toolslib/tplistwh.h"
#include "../model/mdllctr.h"

class UnicodeComposer;

/*----------------------------------------------------------------ElementList-+
| List of all elements declared for this DTD or added on the fly in instance. |
+----------------------------------------------------------------------------*/
class YASP3_API ElementList : public TpListWithHash { // elmlst
public:
   ElementList() {}
   ElementList(ElementList const & source) : TpListWithHash(source) {}
   TP_MakeKeyCollection(Element);           // collection of (keyed) Element's
   TP_MakeSequence(Element);                // ordered
};

/*----------------------------------------------------------------OpenElement-+
| This is a node of the open-element stack.                                   |
| To support OMITTAG or validation, we need this extra info beyond            |
| just what element types are open.                                           |
+----------------------------------------------------------------------------*/
class OpenElement : public RefdItemRep {  // oelm
public:
   OpenElement();
   OpenElement(Tag const * pTag);
   OpenElement(OpenElement const & source);

   Tag const & inqTag() const;
   Tag * inqTagPtr() const;

   ModelLocator lctr;           // Locator into model tree
   bool isDataAllowed;       // Short cut to welcomeElement(0)
   static const OpenElement Nil;

private:
   RefdItem tagptr;            // Tag specification
};
inline OpenElement::OpenElement() : isDataAllowed(false) {
}
inline OpenElement::OpenElement(OpenElement const & source) :
   tagptr(source.tagptr),
   isDataAllowed(source.isDataAllowed),
   lctr(source.lctr)            // deep copy
{
}
inline Tag const & OpenElement::inqTag() const {
   Tag const * pTag = (Tag const *)tagptr.inqData();
   if (pTag) return *pTag; else return Tag::Nil;
}
inline Tag * OpenElement::inqTagPtr() const {
   return (Tag *)tagptr.inqData();
}


/*-----------------------------------------------------------------ElmManager-+
| Main structure                                                              |
+----------------------------------------------------------------------------*/
class YASP3_API ElmManager {
public:
   enum e_WelcomeCond {
      WCOND_HIT,      // everything worked fine
      WCOND_FORCED,   // another occurence of already reported invalid element
      WCOND_INCLUDED, // the element is valid as an inclusion
      WCOND_INVALID   // the element is not valid.
   };
   class SignalEventCB {
   public:
      virtual ~SignalEventCB();
      virtual void startTag(Tag const * pTag, bool isEmpty);
      virtual void endTag(Tag const * pTag);
   };

   ElmManager(UnicodeComposer & erh);

   Element * defineElement(
      UnicodeString const & ucsGi,  // Generic Identifier
      ElmContent const & content,
      bool omitStart,            // true = omittable
      bool omitEnd
   );
   Element * defineElement(
      int ix,                       // Index in list
      ElmContent const & content,
      bool omitStart,            // true = omittable
      bool omitEnd
   );
   Element * defineElementIfNotThere(
      UnicodeString const & ucsGi   // Generic Identifier
   );
   bool startElement(
      RefdItem tagptr,
      SignalEventCB & cb,
      bool isEmpty = false       // override model by CONREF rule
   );
   bool endElement(              // regular
      Element const * pElmGiven,
      SignalEventCB & cb
   );
   bool endElement(              // STAGO_TAGC (</>)
      SignalEventCB & cb
   );
   bool endElementNet(           // end most recent NET enabling tag
      SignalEventCB & cb
   );
   e_WelcomeCond welcomeElement(    // what conditions to open this element?
      Element const * pElmNew,
      SignalEventCB & cb
   );
   bool endAllElements(
      SignalEventCB & cb,
      int iMax = 0
   );
   void setFloorDepth(int iDepth);  // cannot pop under this limit

   bool reset();                    // to process a new instance
   bool setRootModel(UnicodeString const & ucsDoctype);
   Element const * inqElement(UnicodeString const & ucsGi) const;
   Element const * inqElement(int ix) const;

   // inquires from current status
   Tag const & inqTagLastOpened() const;
   Element const * inqElementLastOpened() const;
   Element const * inqElementLastClosed() const;
   Element const * inqElementBase() const;
   bool isDataAllowed() const;
   bool isNet() const;
   e_RecognitionMode inqRecognitionMode() const;
   bool isNetEnabled() const;
   int inqDepth() const;

   ElementList const & inqListElement() const;

   void debugElementList(ostream & out) const;
   void setListElement(RefdKey const & doctype, ElementList const & source);

   operator void *() const;            // isOk?
   bool operator!() const;             // is not ok?

private:
   class YASP3_API State {            // what to backup?
   public:
      State();                         // copy and assignment uses default.
      ~State();
      bool operator!();
      operator void *();

      bool setFirstElement(ModelTree const & model);

      bool push(Tag const *, SignalEventCB &, bool isEmpty);
      bool pop(
         SignalEventCB & cb,
         bool isForce = true,
         Tag::Minimize mini = Tag::Omitted
      );
      void cleanup();

      int inqCount() const;
      OpenElement * inqLastOpenElement() const;
      Element const * inqElementLastClosed() const;
      bool isNetEnabled() const;
      Element const & inqFirstElement() const;
      e_WelcomeCond endWouldAllow(int ixElmNew) const;
      void checkExceptions(
         Element const * pElm, bool & isExclusion, bool & isInclusion
      ) const;
      ModelLocator & copyLocator(bool & isTrueBackup);

   private:
      class YASP3_API Stack : public TpList {
      public:
         TP_MakeSequence(OpenElement);
      };
      Stack stack;                     // stack of open elements
      Element const * pElmLastClosed;  // last element ended
      int iNetActives;                 // how many NET-enabling starts open?
   };

   Element * initRoot();
   bool isEnoughDepth() const;
   bool endLastElement(
      SignalEventCB & cb, Tag::Minimize mini = Tag::Omitted
   );

   /*-------------+
   | Private Data |
   +-------------*/
   static UnicodeString const ucsRootName;
   UnicodeComposer & erh;              // where to report errors?
   bool bOk;                           // working instance
   int ixBase;                         // index of the document element
   int iFloorDepth;                    // no pops under this limit
   ElementList elmlst;
   State s;
};

/* -- INLINES -- */
inline ElmManager::State::State() :
   pElmLastClosed(0),
   iNetActives(0)
{}
inline ElmManager::State::~State() {
   cleanup();
}
inline int ElmManager::State::inqCount() const {
   return stack.count();
}
inline OpenElement * ElmManager::State::inqLastOpenElement() const {
   return stack.last();
}
inline bool ElmManager::State::isNetEnabled() const {
   if (iNetActives) return true;  else return false;
}
inline Element const * ElmManager::State::inqElementLastClosed() const {
   return pElmLastClosed;
}
inline Element const & ElmManager::State::inqFirstElement() const {
   return stack.first()->inqTag().inqElement();
}
inline bool ElmManager::State::operator!() {
  if (stack) return false; else return true;
}
inline ElmManager::State::operator void *() {
  if (stack) return (void *)this; else return 0;
}

inline void ElmManager::setFloorDepth(int iDepth) {
   iFloorDepth = iDepth;
}
inline Element const * ElmManager::inqElement(
   UnicodeString const & ucsGi
) const {
   return elmlst[ucsGi];
}
inline Element const * ElmManager::inqElement(int ix) const {
   return elmlst[ix];
}
inline Tag const & ElmManager::inqTagLastOpened() const {
   OpenElement * pOelm = s.inqLastOpenElement();
   if (pOelm) {
      return pOelm->inqTag();
   }else {
      return Tag::Nil;
   }
}
inline Element const * ElmManager::inqElementLastOpened() const {
   OpenElement const * pOelm = s.inqLastOpenElement();
   if (pOelm) return pOelm->inqTag().inqElementPtr(); else return 0;
}
inline Element const * ElmManager::inqElementLastClosed() const {
   return s.inqElementLastClosed();
}
inline Element const * ElmManager::inqElementBase() const {
   return elmlst[ixBase];
}
inline bool ElmManager::isDataAllowed() const {
   return s.inqLastOpenElement()->isDataAllowed;
}
inline bool ElmManager::isNet() const {
   return s.inqLastOpenElement()->inqTag().isNetStart();
}
inline bool ElmManager::isNetEnabled() const {
   return s.isNetEnabled();
}
inline int ElmManager::inqDepth() const {
   return s.inqCount();
}
inline ElementList const & ElmManager::inqListElement() const {
   return elmlst;
}
inline ElmManager::operator void *() const {
   if (bOk) return (void *)this; else return 0;
}
inline bool ElmManager::operator!() const {     // is not ok?
   if (bOk) return false; else return true;
}

#endif /* ELMMGR_HEADER =====================================================*/
