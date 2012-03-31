/*
* $Id: mdllctr.h,v 1.2 2011-07-29 10:26:39 pgr Exp $
*
* Locator inside a Model Tree to find out if an element matches
* the content model.
*/

#if !defined MDLLCTR_HEADER
#define MDLLCTR_HEADER

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include "hitbits.h"
#include "mdltree.h"

/*-------------------------------------------------------------e_MdllctrStatus-+
| This enum is used to return the status of the locator after                  |
| `isValidStart()' or `isValidEnd()'                                           |
+-----------------------------------------------------------------------------*/
typedef enum {
   /*
   | The proposed element fits in the model.
   | If the element is also on the exclusion list, it
   | was not excluded because this exclusion is invalid.
   |
   | The ModelLocator reflects the new position reached.
   */
   MDLLCTR_HIT,

   /*
   | The proposed element can not fit in the model,
   | because a well determinate element is required
   |
   | The ModelLocator does not need to be restored.
   | The peekMissing() method can be called to get
   | the required element.
   */
   MDLLCTR_MISSING,

   /*
   | The proposed element fits because it is an inclusion.
   | The ModelLocator reflects the new position reached.
   |
   | To pursue with this model, caller needs to restore
   | the ModelLocator.
   */
   MDLLCTR_INCLUDED,

   /*
   | The proposed element would fit in the model,
   | but is a valid exclusion.
   |
   | To pursue with this model, caller needs to restore
   | the ModelLocator.
   */
   MDLLCTR_EXCLUDED,

   /*
   | The proposed element can not fit in the model,
   | but the current started element can be safely closed
   | (to see if the proposed element could belong to the
   | parent of the current started element).
   |
   | To undo the close, caller needs to restore
   | the ModelLocator.
   */
   MDLLCTR_CLOSED,

   /*
   | The proposed element can not fit in the model,
   | and the current started element can not be closed
   | because undeterminate elements are missing.
   |
   | To pursue with this model, caller needs to restore
   | the ModelLocator.
   */
   MDLLCTR_UNCLOSABLE,

   /*
   | An internal error occurred.
   */
   MDLLCTR_INTERNAL_ERROR
}e_MdllctrStatus;

#if defined __cplusplus
/*----------------------------------------------------------------ModelLocator-+
|                                                                              |
+-----------------------------------------------------------------------------*/
class ModelToken;
class ModelTree;

class ModelLocator {
public:
   /*---------------+
   | Public Methods |
   +---------------*/
   ModelLocator();                                    // NULL constructor
   ModelLocator(ModelTree const& mdlArg);             // Regular constructor
   ModelLocator(ModelLocator const& from);            // copy
   ModelLocator& operator=(ModelLocator const& from); // assignment

   ~ModelLocator() {}                 // dummy - just to remove warning

   // after copy or assignement, check this
   operator void *() const;           // isOk?
   bool operator!() const;         // is not ok?

   e_MdllctrStatus isValidStart(
      int ixElmGiven,                 // Proposed element (-1 means #PCDATA)
      bool const isExclusion,      // true if element listed in exclusion
      bool const isInclusion       // true if element listed in inclusion
   );
   int peekMissing() const;           // what element is missing?
   bool hitMissing();              // hit the missing element

   e_MdllctrStatus isValidEnd();
   bool isBestCandidate(int ixElm) const; // repeatable token, currently hit

private:
   /*-------------+
   | Private Data |
   +-------------*/
   ModelTree const& mdl;
   HitBits hbits;
   ModelToken const* pTknCurr;

   /*----------------+
   | Private Methods |
   +----------------*/
   bool find_first_optional_sibling_unhit();
   bool find_first_required_sibling_unhit();
   e_MdllctrStatus find_missing_element();
   e_MdllctrStatus find_another_candidate(bool const isCurrentHit);
   bool find_another_and_candidate(
      ModelToken const* pTknParent,
      bool const isCurrentHit
   );
   bool find_another_seq_candidate();
   bool find_another_or_candidate();
   void hit(ModelToken const* pTkn);
   void reset_hits(ModelToken const* pTkn);
   bool isHit(ModelToken const* pTkn) const;
   bool isSome_hits(ModelToken const* pTkn) const;

public:
   static ModelLocator const Nil;
};

/*--- INLINES ---*/
inline ModelLocator::ModelLocator() :
   mdl(ModelTree::Nil), hbits(0), pTknCurr(0)
{ }

inline ModelLocator::ModelLocator(ModelTree const& mdlArg) :
   mdl(mdlArg),
   hbits(mdlArg.iNbrOfTokens),
   pTknCurr(mdlArg.pTknBase)
{
   if (!hbits || !mdl) new(this) ModelLocator(); // overwrite with a null one
}
inline ModelLocator::ModelLocator(
   ModelLocator const & source
) : mdl(source.mdl), hbits(source.hbits), pTknCurr(source.pTknCurr) {
   if (!hbits || !mdl) new(this) ModelLocator(); // overwrite with a null one
}
inline ModelLocator::operator void *() const {
   if (mdl) return (void *)this; else return 0;
}
inline bool ModelLocator::operator!() const {
   if (!mdl) return true; else return false;
}
inline ModelLocator & ModelLocator::operator=(ModelLocator const & source) {
   if (this != &source) {
      this->~ModelLocator();
      new(this) ModelLocator(source);
   }
   return * this;
}
inline int ModelLocator::peekMissing() const {
   if ((pTknCurr) && (pTknCurr->isElement())) {
      return pTknCurr->ixElm;
   }else {
      return -1;
   }
}
inline bool ModelLocator::hitMissing() {
   if ((pTknCurr) && (pTknCurr->isElement())) {
      find_another_candidate(true);
      return true;
   }else {
      return false;
   }
}
inline bool ModelLocator::isBestCandidate(int ixElm) const {
   if (pTknCurr && (pTknCurr->ixElm == ixElm) && (pTknCurr->isRepeatable())) {
      return true;
   }else {
      return false;
   }
}
#endif /* __cplusplus */
#endif /* MDLLCTR_HEADER =====================================================*/
