/*
* $Id: element.h,v 1.9 2011-07-29 10:26:39 pgr Exp $
*
* Describes the element structure and its components
*/

#if !defined ELEMENT_HEADER && defined __cplusplus
#define ELEMENT_HEADER

/*---------+
| Includes |
+---------*/
#include <new>
#include "attrib.h"
#include "../../toolslib/tplistix.h"
#include "../parser/yspenum.h"
#include "../parser/yspevent.h"
#include "../model/mdltree.h"
#include "../entity/entity.h"

class Element;

/*-----------------------------------------------------------------ElmContent-+
| Represents the content model.                                               |
|                                                                             |
| Note:                                                                       |
|   More than one element can point to the same instance of this struct.      |
|   Destruction is controlled by reference counting.                          |
+----------------------------------------------------------------------------*/
class YASP3_API ElmContent {
public:
   ElmContent();                                     // Nil constructor
   ElmContent(ModelTree const & model);              // regular, w/o excpt.
   ElmContent(                                       // regular with excpt.
      ModelTree const & model,
      TpIxList const & inclusions,
      TpIxList const & exclusions
   );
   ElmContent(ElmContent const & source);            // copy constructor
   ~ElmContent();

   ElmContent& operator=(ElmContent const & source); // assignment
   operator void *() const;                          // isOk?
   bool operator!() const;                        // is not ok?

   bool isIncluded(int ixElm) const;
   bool isExcluded(int ixElm) const;
   ModelTree const & inqModel() const;
   TpIxList const & inqInclusions() const;
   TpIxList const & inqExclusions() const;

   ElmContent copy(TpNameIxXtable const & ixtbl) const;  // deep copy
   bool operator==(ElmContent const & source) const;
   bool operator!=(ElmContent const & source) const;

private:
   ElmContent(
      ModelTree const & model,
      TpIxList const & inclusions,
      TpIxList const & exclusions,
      TpNameIxXtable const & ixtbl
   );
   class Rep {
   public:
      Rep(
         ModelTree const & model,
         TpIxList const & inclusionsArg,
         TpIxList const & exclusionsArg,
         bool & isOk
      );
      Rep& operator=(Rep const & source); // no!  no way, Jose
      Rep(Rep const & source);            // no!

      int iReferenceCount;                // Contents can be shared
      ModelTree mdl;
      TpIxList inclusions;
      TpIxList exclusions;
   };
   Rep * pRep;
public:
   static ElmContent const Nil;
   static ElmContent const Undefined;
};

/* -- INLINES -- */
inline ElmContent::ElmContent() : pRep(0) {
}
inline ElmContent::ElmContent(ElmContent const & source) {
   pRep = source.pRep;
   if (pRep) ++pRep->iReferenceCount;
}
inline ElmContent::~ElmContent() {
   if ((pRep) && (!--pRep->iReferenceCount)) {
      delete pRep;
   }
   pRep = 0;
}
inline ElmContent& ElmContent::operator=(ElmContent const & source) {
   if (pRep != source.pRep) {
      this->~ElmContent();
      new(this) ElmContent(source);
   }
   return *this;
}
inline ElmContent::operator void *() const {
   if (pRep) return (void *)this; else return 0;
}
inline bool ElmContent::operator!() const {
   if (pRep) return false; else return true;
}
inline bool ElmContent::isIncluded(int ixElm) const {
   if (pRep) return pRep->inclusions.contain(ixElm); else return false;
}
inline bool ElmContent::isExcluded(int ixElm) const {
   if (pRep) return pRep->exclusions.contain(ixElm); else return false;
}
inline ModelTree const & ElmContent::inqModel() const {
   if (pRep) return pRep->mdl; else return ModelTree::Nil;
}
inline TpIxList const & ElmContent::inqInclusions() const {
   if (pRep) return pRep->inclusions; else return TpIxList::Nil;
}
inline TpIxList const & ElmContent::inqExclusions() const {
   if (pRep) return pRep->exclusions; else return TpIxList::Nil;
}
inline bool ElmContent::operator!=(ElmContent const & source) const {
   if (*this == source) return false; else return true;
}

/*--------------------------------------------------------------------Element-+
| An Element is the structure which groups all information related to         |
| an element: coming from the ELEMENT decl, as well as the ATTLIST decl.      |
|                                                                             |
| Element's are pointed at from the ElmList.                                  |
| They are created via ElmManager::defineElement, and destroyed when the      |   |
| ElmManeger is destroyed. (thus they survive across openDocument calls).     |
+----------------------------------------------------------------------------*/
class YASP3_API Element : public RefdKeyRep {
   friend ostream & operator<<(ostream& out, Element const & elm); // debug
public:
   Element();
   Element(
      UnicodeString const & ucsGi,
      int ixElmArg,
      ElmContent const & contentArg,
      bool omitStartArg,
      bool omitEndArg,
      Attlist const & attlstArg
   );
   Element(
      UnicodeString const & ucsGi,
      int ixElmArg,
      ElmContent const & contentArg,
      bool omitStartArg,
      bool omitEndArg
   );
   Element(Element const & source); // deep copy (but members go shallow)
   Element& operator=(Element const & source);
   Element copy(           // deep copy (full)
      TpNameIxXtable const & ixtbl
   ) const;

   operator void *() const;        // isOk?
   bool operator!() const;      // is not ok?
   bool operator==(Element const & source) const;
   bool operator!=(Element const & source) const;

   // -- regular inquiries --

   UnicodeString const & inqGi() const;
   UnicodeString const & inqName() const;  // as previous, just to be nice
   ModelTree const & inqModel() const;
   TpIxList const & inqInclusions() const;
   TpIxList const & inqExclusions() const;
   Attlist const & inqAttlist() const;
   int inqIx() const;

   bool isDefined() const;
   bool isStartRequired() const;
   bool isEndRequired() const;

   bool isIncluded(Element const * pElm) const;
   bool isExcluded(Element const * pElm) const;

   // -- non const functions (reserved) --

   bool attachAttlist(Attlist const & attlstArg);
   void attachContent(
      ElmContent const & contentArg,
      bool omitStart,
      bool omitEnd
   );

private:
   int ixElm;                      // handle known by model to refer this elm
   ElmContent content;             // content, including the exceptions
   Attlist attlst;                 // associated attribute list
   struct {                        // elem decl flag:
      unsigned int omitStart : 1;  //   start tag can be omitted
      unsigned int omitEnd   : 1;  //   end tag can be omitted
   }b;
public:
   static Element const Nil;
};

/* -- INLINES -- */
inline Element::Element() {
  ixElm = 0xDeadBeef;
  b.omitStart = 0;
  b.omitEnd = 0;
}
inline Element::Element(Element const & source) :
   RefdKeyRep(source.key()),
   ixElm(source.ixElm),
   content(source.content),
   attlst(source.attlst),
   b(source.b)
{}
inline Element& Element::operator=(Element const & source) {
   if (this != &source) {
      this->~Element();
      new(this) Element(source);
   }
   return *this;
}
inline bool Element::operator!=(Element const & source) const {
   if (*this == source) return false; else return true;
}
inline Element::operator void *() const {
   if (content) return (void *)this; else return 0;
}
inline bool Element::operator!() const {
   if (content) return false; else return true;
}
inline UnicodeString const & Element::inqGi() const {
   return key();
}
inline UnicodeString const & Element::inqName() const {
   return key();
}
inline ModelTree const & Element::inqModel() const {
   return content.inqModel();
}
inline TpIxList const & Element::inqInclusions() const {
   return content.inqInclusions();
}
inline TpIxList const & Element::inqExclusions() const {
   return content.inqExclusions();
}
inline Attlist const & Element::inqAttlist() const {
   return attlst;
}
inline int Element::inqIx() const {
   return ixElm;
}
inline bool Element::isDefined() const {  // short cut to model
  if (content.inqModel().isDefined()) return true; else return false;
}
inline bool Element::isStartRequired() const {
  if (b.omitStart) return false; else return true;
}
inline bool Element::isEndRequired() const {
  if (b.omitEnd) return false; else return true;
}
inline bool Element::isIncluded(Element const * pElm) const {
   return content.isIncluded(pElm->ixElm);
}
inline bool Element::isExcluded(Element const * pElm) const {
   return content.isExcluded(pElm->ixElm);
}

/*------------------------------------------------------------------------Tag-+
| Tag objects are created when a tag is parsed                                |
+----------------------------------------------------------------------------*/
class YASP3_API Tag : public RefdItemRep {
public:

   // -- standard inquiries --

   Element const & inqElement() const; // what element do we start/end?
   Attlist const & inqAttlist() const; // what is the attribute list?
   Tag const & inqParent() const;      // who is the parent?
   Entity const & inqEntity() const;   // first ENTITY attribute (deprecated)

   bool isOmittedStart() const;     // start was inferred by the context
   bool isRegularStart() const;     // started with <foo> (no minimization)
   bool isNetStart() const;         // started with <foo/
   bool isEmptyStart() const;       // started with <>

   bool isOmittedEnd() const;       // end was inferred by the context
   bool isRegularEnd() const;       // ended with </foo> (no minimization)
   bool isNetEnd() const;           // ended with /
   bool isEmptyEnd() const;         // ended with </>

   bool isInclusion() const;        // is OK because inclusion
   bool isInvalidStart() const;     // started because the user said so!
   bool isInvalidEnd() const;       // prematurely ended
   bool isFakedEnd() const;         // DcData dcl ended by bad end tag

   // -- short cuts --
   UnicodeString const & inqName() const;
   int inqAttributeCount() const;
   UnicodeString const & inqAttributeValue(int ix) const;
   UnicodeString const & inqAttributeValue(UCS_2 const * pName) const;

   Attribute const & inqAttribute(int ix) const;
   Attribute const & inqAttribute(UCS_2 const * pName) const;
   Element const * inqElementPtr() const;   // no checks
   UnicodeString inqLongName() const;

   bool inqLongName(Writer & umost) const;

   int checkExceptions(Element const * pElm) const;
   int checkEndElement(Element const * pElm) const;
   int checkEndNet() const;

   // -- other const methods

   void stringize(
      Writer & uost,
      bool isEndTag,
      bool isFullAttlist = true
   ) const;

   operator void *() const;              // isOk?
   bool operator!() const;               // is not ok?

   // -- For Yasp3 use --

   enum Minimize {
      Omitted = 0,
      Regular = 1,
      Net = 2,
      Empty = 3
   };
   enum MiscCond {
      Inclusion    = 1 << 0,
      InvalidStart = 1 << 1,
      InvalidEnd   = 1 << 2,   // end content model not satisfied
      FakedEnd     = 1 << 3    // end implied by invalid end tag (DCDATA)
   };
   class YASP3_API Flag {
      friend class Tag;
      friend ostream& operator<<(ostream& out, Flag const & flag);
   public:
      Flag(Minimize miniStartArg = Omitted) :
         miniStart(miniStartArg), miniEnd(Omitted), cond(0) {
      }
      void operator=(Minimize miniStartArg) {
         miniStart = miniStartArg;
      }
      Flag & operator<<(MiscCond condArg) {
         cond |= condArg;
         return *this;
      }
   private:
      Minimize miniStart;
      Minimize miniEnd;
      char cond;
   };

   Tag() {}
   Tag(
      Element const * pElmArg,
      Attlist const & attlstArg
   );
   Tag(
      Element const * pElmArg,
      Attlist const & attlstArg,
      Flag flag
   );

   bool setEntity(Entity const * pEnt);  // should vanish some day
   bool setParent(Tag const & parent);
   Tag & operator<<(Tag::Minimize miniEndArg);
   Tag & operator<<(Tag::MiscCond condArg);

private:
   Tag& operator=(Tag const & source); // no!
   Tag(Tag const & source);            // no!

   RefdItem parent;                   // it *is* a Tag!
   RefdKey elmptr;
   Attlist attlst;
   Flag flag;
   RefdKey entptr;     //>>> PGR: should get rid of this.

public:
   static Tag const Nil;
};

/* -- INLINES -- */
inline Tag::Tag(
   Element const * pElmArg,
   Attlist const & attlstArg
) :
   elmptr(pElmArg), attlst(attlstArg), flag(Tag::Omitted) {
}
inline Tag::Tag(
   Element const * pElmArg,
   Attlist const & attlstArg,
   Flag flagArg
) :
   elmptr(pElmArg), attlst(attlstArg), flag(flagArg) {
}
inline Tag & Tag::operator<<(Tag::Minimize miniEndArg) {
   flag.miniEnd = miniEndArg;
   return *this;
}
inline Tag & Tag::operator<<(Tag::MiscCond condArg) {
   flag.cond |= condArg;
   return *this;
}
inline Tag::operator void *() const {
   if (elmptr.isPresent()) return (void *)this; else return 0;
}
inline bool Tag::operator!() const {
   if (elmptr.isPresent()) return false; else return true;
}
inline Element const & Tag::inqElement() const {
   Element const * pElm = (Element const *)elmptr.inqData();
   if (pElm) return *pElm; else return Element::Nil;
}
inline Attlist const & Tag::inqAttlist() const {
   return attlst;
}
inline Tag const & Tag::inqParent() const {
   if (parent.isPresent()) {
      return  *(Tag const *)(parent.inqData());
   }else {
      return Tag::Nil;
   }
}
inline Entity const & Tag::inqEntity() const {
   Entity const * pEnt = (Entity const *)entptr.inqData();
   if (pEnt) return *pEnt; else return Entity::Nil;
}
inline bool Tag::isOmittedStart() const {
   if (flag.miniStart == Omitted) return true; else return false;
}
inline bool Tag::isRegularStart() const {
   if (flag.miniStart == Regular) return true; else return false;
}
inline bool Tag::isNetStart() const {
   if (flag.miniStart == Net) return true; else return false;
}
inline bool Tag::isEmptyStart() const {
   if (flag.miniStart == Empty) return true; else return false;
}
inline bool Tag::isOmittedEnd() const {
   if (flag.miniEnd == Omitted) return true; else return false;
}
inline bool Tag::isRegularEnd() const {
   if (flag.miniEnd == Regular) return true; else return false;
}
inline bool Tag::isNetEnd() const {
   if (flag.miniEnd == Net) return true; else return false;
}
inline bool Tag::isEmptyEnd() const {
   if (flag.miniEnd == Empty) return true; else return false;
}
inline bool Tag::isInclusion() const {
   if (flag.cond & Inclusion) return true; else return false;
}
inline bool Tag::isInvalidStart() const {
   if (flag.cond & InvalidStart) return true; else return false;
}
inline bool Tag::isInvalidEnd() const {
   if (flag.cond & InvalidEnd) return true; else return false;
}
inline bool Tag::isFakedEnd() const {
   if (flag.cond & FakedEnd) return true; else return false;
}
inline UnicodeString const & Tag::inqName() const {
   return inqElement().inqName();
}
inline int Tag::inqAttributeCount() const {
   return attlst.count();
}
inline UnicodeString const & Tag::inqAttributeValue(int ix) const {
   return attlst[ix].inqValue();
}
inline UnicodeString const & Tag::inqAttributeValue(
   UCS_2 const * pName
) const {
   return attlst[pName].inqValue();
}
inline Attribute const & Tag::inqAttribute(int ix) const {
   return attlst[ix];
}
inline Attribute const & Tag::inqAttribute(UCS_2 const * pName) const {
   return attlst[pName];
}
inline Element const * Tag::inqElementPtr() const {
   return (Element const *)elmptr.inqData();
}
inline bool Tag::setEntity(Entity const * pEnt) {
   entptr = pEnt;
   return true;
}
inline bool Tag::setParent(Tag const & parentArg) {
   parent = &parentArg;
   return true;
}
#endif /* ELEMENT_HEADER ====================================================*/
