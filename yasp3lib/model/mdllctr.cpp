/*
* $Id: mdllctr.cpp,v 1.2 2002-04-16 17:13:49 jlatone Exp $
*
* Validation of a proposal to start/end an element
*
* Store the context of the current position within the content model.
* The ModelTree structure serves as a road map.
*
* Notes:
*    A ModelLocator should always work, even if the ModelTree is bad.
*
*    When:                                 status is:           pTknBase is:
* 1  the element was not declared,         GRPO_EXPECTED        NULL
* 2  the element has a declared content,   OK                   NULL
* 3  the model content is OK               OK                   non null
* 4  the model content is not yet ended    OK < and < ERROR_ALL non null
* 5  the model content is incorrect        >= ERROR_ALL         NULL
*
*    When pTknBase is NULL, everything works fine: all propositions are
*    agreed on.
*    When pTknBase is non-Null, we might have troubles (case 4).
*    Also, if a memory error occured (HitBit).
*    In this case, we overwrite our ModelLocator with a NULL one.
*/

/*--------------+
| Include files |
+--------------*/
#include "mdltree.h"
#include "mdllctr.h"

ModelLocator const ModelLocator::Nil;

/*-------------------------------------------------------------------------hit-+
| Set a hit bit on                                                             |
+-----------------------------------------------------------------------------*/
inline void ModelLocator::hit(ModelToken const* pTkn) {
   hbits.set_on(pTkn->seqNo());
}

/*-----------------------------------------------------------------------isHit-+
| Test if a hit bit is on                                                      |
+-----------------------------------------------------------------------------*/
inline bool ModelLocator::isHit(ModelToken const* pTkn) const {
   return hbits.is_on(pTkn->seqNo());
}

/*------------------------------------------------------------------reset_hits-+
| Reset all hit bits of the current model group                                |
+-----------------------------------------------------------------------------*/
inline void ModelLocator::reset_hits(ModelToken const* pTkn)
{
   pTkn = pTkn->child;
   for (;;) {
      hbits.set_off(pTkn->seqNo());
      if (pTkn->isYounger()) break;
      pTkn = pTkn->sibling;
   }
}

/*-----------------------------------------------------------------isSome_hits-+
| Return true if tokens where hit in the current model group                   |
+-----------------------------------------------------------------------------*/
inline bool ModelLocator::isSome_hits(ModelToken const* pTkn) const
{
   for (pTkn = pTkn->child; !isHit(pTkn); pTkn = pTkn->sibling) {
      if (pTkn->isYounger()) return false;
   }
   return true;
}

/*-------------------------------------------find_first_optional_sibling_unhit-+
| ... and return true if found                                                 |
+-----------------------------------------------------------------------------*/
inline bool ModelLocator::find_first_optional_sibling_unhit()
{
   for (;;) {
      if (!isHit(pTknCurr)) {
         return true;
      }
      if (pTknCurr->isYounger()) {
         return false;
      }
      pTknCurr = pTknCurr->sibling;
   }
}

/*-------------------------------------------find_first_required_sibling_unhit-+
| ... and return true if found                                                 |
+-----------------------------------------------------------------------------*/
inline bool ModelLocator::find_first_required_sibling_unhit()
{
   for (;;) {
      if ((!isHit(pTknCurr)) && (!pTknCurr->isOmissible())) {
         return true;
      }
      if (pTknCurr->isYounger()) {
         return false;
      }
      pTknCurr = pTknCurr->sibling;
   }
}

/*----------------------------------------------------------------isValidStart-+
| Determine how a proposed element fits within the current model.              |
|                                                                              |
| On Entry:                                                                    |
|   `pTknCurr'     points a primitive content token                            |
|   `ixElmGiven'   is a handle designating an element (-1 if #PCDATA)          |
|   `isInclusion'  is true if the element belongs to the inclusion list        |
|   `isExclusion'  is true if the element belongs to the exclusion list        |
|                  (both can be true!)                                         |
|                                                                              |
| When returning:                                                              |
|                                                                              |
|   Return code is: (see mdllctr.h for explanations)                           |
|     MDLLCTR_HIT                                                              |
|     MDLLCTR_MISSING                                                          |
|     MDLLCTR_INCLUDED                                                         |
|     MDLLCTR_EXCLUDED                                                         |
|     MDLLCTR_CLOSED                                                           |
|     MDLLCTR_UNCLOSABLE                                                       |
|     MDLLCTR_INTERNAL_ERROR                                                   |
|                                                                              |
| NOTE:                                                                        |
|    The current model we check against can never be EMPTY, #PCDATA:           |
|    - for EMPTY, caller opens and closes the element: not *really* stacked    |
|    - #PCDATA is never stacked.                                               |
|    In case of CDATA or RCDATA, this procedure is called just for <#PCDATA>,  |
|    i.e. ixElmGiven is -1 (PCDATA).  It cannot be a true element,             |
|    because STAGO is not recognized.                                          |
+-----------------------------------------------------------------------------*/
e_MdllctrStatus ModelLocator::isValidStart(
   int ixElmGiven,                            // Proposed element (-1: PCDATA)
   bool const isExclusion,
   bool const isInclusion
)
{
   if (!mdl.isModel()) {                      // declared content
      if (mdl.isAny()) {
         if (isExclusion) {
            return MDLLCTR_EXCLUDED;
         }else {
            return MDLLCTR_HIT;
         }
      }else {                                 // this should be DCDATA
         assert  (ixElmGiven == -1);
         return MDLLCTR_HIT;
      }
   }
   if (!pTknCurr) {                           // model has ended or it's Nil
      if (isInclusion) {
         return isExclusion? MDLLCTR_EXCLUDED : MDLLCTR_INCLUDED;
      }else {
         return MDLLCTR_CLOSED;
      }
   }

   e_MdllctrStatus st = MDLLCTR_HIT;
   do {                                       // get a prim content token
      while (pTknCurr->isGroup()) {
         reset_hits(pTknCurr);                // reset all hit bits
         pTknCurr = pTknCurr->child;          // grab the child
      }
      if (ixElmGiven == pTknCurr->ixElm) {    // Hit!
         if (
            (isExclusion) && (
               (pTknCurr->isOmissible()) || (pTknCurr->parent()->isOrGroup())
            )
         ) {
            st = MDLLCTR_EXCLUDED;            // Valid exclusion
         }
         find_another_candidate(true);
         return st;
      }
   }while (st = find_another_candidate(false), st == MDLLCTR_HIT);

   if (isInclusion) {
      return isExclusion? MDLLCTR_EXCLUDED : MDLLCTR_INCLUDED;
   }
   return st;
}

/*--------------------------------------------------find_another_and_candidate-+
| Find out if another token could be a candidate in an AND group, or, if not,  |
| if the group can be safely closed.                                           |
|                                                                              |
| On Entry:                                                                    |
|   `pTknCurr'     points the current token (as returned by previous calls     |
|                  to this routine, or initialized to the first AND token)     |
|   `pTokenParent' is `pTknCurr's parent                                       |
|   `isCurrentHit' indicates if `pTknCurr' was just hit                        |
|                                                                              |
| When returning:                                                              |
|                                                                              |
|   Return code is:                                                            |
|     true:  another candidate was found                                       |
|     false: no other candidate                                                |
|                                                                              |
|   `pTknCurr' is:                                                             |
|   - NULL pointer:  no other candidate, but the  group can be safely closed   |
|   - A valid ModelToken pointer:                                              |
|     - if true is returned, this is the new candidate                         |
|     - if false is returned, this is the first required token that prevents   |
|                             the group to be closed                           |
|                                                                              |
| IMPLEMENTATION NOTES:                                                        |
|                                                                              |
| In an AND group, no more token can be a candidate if all tokens, including   |
| optionals, were hit.                                                         |
|                                                                              |
| AND groups are tricky to handle, especially to avoid infinite loops and      |
| not to miss any.                                                             |
|                                                                              |
| Here is the way this is done:                                                |
|                                                                              |
| 1) At the time the token is hit: (isCurrentHit is true)                      |
|   a- if it is repeatable, find_candidate() returns early and this code       |
|      is not reached for this time.                                           |
|   b- if it is not repeatable, this code is called: it will restart the       |
|      scanning in the AND group to the first unhit token (from the            |
|      beginning).  This token will no more be a candidate.                    |
|   Return.                                                                    |
|                                                                              |
| 2) If isCurrentHit is false,                                                 |
|   a - if the AND token hit bit is on, this token was a repeatable token      |
|       left as current by 1)a-.  Since isCurrentHit is false, it is safe      |
|       to declare this token no more a candidate: restart the scanning        |
|       of the AND group from the beginning to the next unhit token.           |
|       Return.                                                                |
|   b - Else, if a next sibling is unhit, make it a new candidate.             |
|       Return.                                                                |
|   c - If no more sibling, it's definitively INVALID.                         |
|       Return.                                                                |
|                                                                              |
| Again, this algorithm ensures the AND list wont be examined twice,           |
| and that all candidate tokens are returned one after each other.             |
| Because, 1) the list is restarted one time only (when the token              |
| was hit, or later if it's a repeatable token), and 2) it restarts            |
| from the beginning so we wont miss any.                                      |
+-----------------------------------------------------------------------------*/
bool ModelLocator::find_another_and_candidate(
   ModelToken const* pTknParent, bool const isCurrentHit
)
{
   if ((isCurrentHit) || (isHit(pTknCurr))) {
      pTknCurr = pTknParent->child;            // restart from the elder
      if (find_first_optional_sibling_unhit()) {
         return true;                          // new candidate
      }
      pTknCurr = 0;                            // good: no more candidates!
      return false;                            // but group can be closed
   }else {
      if (!pTknCurr->isYounger()) {            // more siblings
         pTknCurr = pTknCurr->sibling;         // start at next unhit sibling
         if (find_first_optional_sibling_unhit()) {
            return true;                       // new candidate
         }
      }
      /*
      | All AND tokens have been examined.
      | See if we could end the group by
      | now skipping over optional tokens...
      */
      pTknCurr = pTknParent->child;            // re-scan from the beginning
      if (find_first_required_sibling_unhit()) {
         return false;                         // required
      }
      pTknCurr = 0;                            // group can be closed!
      return false;
   }
}

/*--------------------------------------------------find_another_seq_candidate-+
| Find out if another token could be a candidate in an SEQ group, or, if not,  |
| if the group can be safely closed.                                           |
|                                                                              |
| On Entry:                                                                    |
|   `pTknCurr'  points the current token (as returned by previous calls        |
|               to this routine, or initialized to the first SEQ token)        |
| When returning:                                                              |
|                                                                              |
|   Return code is:                                                            |
|     true:  another candidate was found                                       |
|     false: no other candidate                                                |
|                                                                              |
|   `pTknCurr' is:                                                             |
|   - NULL pointer:  no other candidate, but the  group can be safely closed   |
|   - A valid ModelToken pointer:                                              |
|     - if true is returned, this is the new candidate                         |
|     - if false is returned, this is the first required token that prevents   |
|                             the group to be closed                           |
|                                                                              |
| In an SEQ group, no more token can be a candidate if current token           |
| is the last and was either hit, or is omittable.                             |
| The next valid token is the next sequential one.                             |
| It is not an no-sibling token, or SEQ group would have ended                 |
+-----------------------------------------------------------------------------*/
bool ModelLocator::find_another_seq_candidate()
{
   if ((pTknCurr->isOmissible()) || (isHit(pTknCurr))) {
      if (pTknCurr->isYounger()) {        // last token?
         pTknCurr = 0;                    // group can be safely closed
         return false;                    // there are no more candidates
      }else {
         pTknCurr = pTknCurr->sibling;    // here is the candidate
         return true;
      }
   }else {
      return false;                       // sorry: pTknCurr is required
   }
}

/*---------------------------------------------------find_another_or_candidate-+
| Find out if another token could be a candidate in an OR group, or, if not,   |
| if the group can be safely closed.                                           |
|                                                                              |
| On Entry:                                                                    |
|   `pTknCurr'  points the current token (as returned by previous calls        |
|               to this routine, or initialized to the first OR token)         |
| When returning:                                                              |
|                                                                              |
|   Return code is:                                                            |
|     true:  another candidate was found                                       |
|     false: no other candidate                                                |
|                                                                              |
|   `pTknCurr' is:                                                             |
|   - NULL pointer:  no other candidate, but the  group can be safely closed   |
|   - A valid ModelToken pointer: this is the new candidate                    |
|                                                                              |
| In an OR group, no more token can be a candidate if current token was hit.   |
| This occurs at the time the token is hit (isCurrentHit is true) when it is   |
| non repeatable, or when looking for a match and the token was a repeatable   |
| token hit previously.                                                        |
|                                                                              |
| The next valid token is the next sequential one.                             |
| It is not a no-sibling token, or OR group would have ended                   |
+-----------------------------------------------------------------------------*/
inline bool ModelLocator::find_another_or_candidate()
{
   if (isHit(pTknCurr)) {
      pTknCurr = 0;                       // group can be safely closed
      return false;                       // there are no more candidates
   }else {
      pTknCurr = pTknCurr->sibling;
      return true;                        // one more candidate
   }
}

/*------------------------------------------------------find_another_candidate-+
| Find the next candidate token in the current model                           |
|                                                                              |
| On Entry:                                                                    |
|   `pTknCurr'     points a primitive content token                            |
|   `isCurrentHit' indicates if `pTknCurr' was just hit                        |
|                                                                              |
| When returning:                                                              |
|                                                                              |
|   Return code is:                                                            |
|     MDLLCTR_HIT        another candidate was found                           |
|     MDLLCTR_CLOSED     no other candidate                                    |
|     MDLLCTR_MISSING    no other candidate: reqd element to be entered first  |
|     MDLLCTR_UNCLOSABLE no other candidate: something missing...              |
|                                                                              |
|   `pTknCurr' is:                                | when the return code is:   |
|   ----------------------------------------------+-------------------------   |
|   the new candidate -- this can be a group      | MDLLCTR_HIT                |
|                                                 |                            |
|   NULL pointer                                  | MDLLCTR_CLOSED             |
|                                                 |                            |
|   the contextually required element preventing  | MDLLCTR_MISSING            |
|   the group to be closed -- this IS an element  |                            |
|                                                 |                            |
|   a contextually required token preventing      | MDLLCTR_UNCLOSABLE         |
|   the group to be closed -- this can be a group |                            |
|                                                                              |
+-----------------------------------------------------------------------------*/
e_MdllctrStatus ModelLocator::find_another_candidate(bool const isCurrentHit)
{
   if (isCurrentHit) {
      /*
      | Current token matches: turn on the hit bit.
      | If the current token is repeatable, returns: this token
      | is again a candidate (note at 11.2.4).
      */

      hit(pTknCurr);
      if (pTknCurr->isRepeatable()) {
         return MDLLCTR_HIT;
      }
   }else {
      /*
      | If nothing was hit in the current group, find out if
      | further scanning in the group can be avoided:
      | - in an AND or OR group, when the last token is reached;
      | - in a SEQ group, when the first non-optional token is
      |   reached
      | End as many groups as possible using this scheme.
      */
      for (
         ModelToken const* pTknParent = pTknCurr;
         pTknParent = pTknParent->parent(),
         pTknParent && !isSome_hits(pTknParent) && (  // none were hit
            (pTknCurr->isYounger()) || (              // last token
               (pTknParent->isSeqGroup()) &&
               (!(pTknCurr->isOmissible()))
            )
         );
         pTknCurr = pTknParent
      )
         ;
   }
   for (
      ModelToken const* pTknParent = pTknCurr;
      pTknParent = pTknParent->parent(), pTknParent;
      pTknCurr = pTknParent
   ) {
      if (pTknParent->isAndGroup()) {
         if (find_another_and_candidate(pTknParent, isCurrentHit)) {
            return MDLLCTR_HIT;
         }
         if (pTknCurr) {                       // required token...
            pTknCurr = pTknParent;             // also, check the parent
            return find_missing_element();     // can be MDLLCTR_MISSING
         }
      }else if (pTknParent->isSeqGroup()) {
         if (find_another_seq_candidate()) {
            return MDLLCTR_HIT;
         }
         if (pTknCurr) {                       // required token...
            return find_missing_element();     // can be MDLLCTR_MISSING
         }
      }else {  /* or_group */
         if (find_another_or_candidate()) {
            return MDLLCTR_HIT;
         }
         if (pTknCurr) {                       // required token...
            return MDLLCTR_UNCLOSABLE;
         }
      }
      /*
      | If no more token in the group can be a candidate,
      | end the group. If the group is repeateable, return:
      | same group is again a candidate.
      */
      hit(pTknParent);                         // hit the group
      if (pTknParent->isRepeatable()) {        // if repeatable,
         pTknCurr = pTknParent;
         return MDLLCTR_HIT;                   // return
      }
   }

   /*
   | The model is now ended
   */
   if (isHit(pTknCurr) || (pTknCurr->isOmissible())) {
      pTknCurr = 0;                            // hit, or not but allowed to
      return MDLLCTR_CLOSED;
   }
   if (pTknCurr->isOrGroup()) {
      return MDLLCTR_UNCLOSABLE;
   }
   return find_missing_element();
}

/*--------------------------------------------------------find_missing_element-+
| Find out if a required element can be found unambiguously.                   |
|                                                                              |
| On Entry:                                                                    |
|   `pTknCurr'     points a required content token                             |
| When returning:                                                              |
|                                                                              |
|   `pTknCurr'     points to the innest content token that can be              |
|                  unambiguously determinated.                                 |
|                                                                              |
|   Return code is:                                                            |
|     MDLLCTR_MISSING    if pTknCurr is at a primitive content token           |
|     MDLLCTR_UNCLOSABLE if pTknCurr is at a model group                       |
|                                                                              |
+-----------------------------------------------------------------------------*/
e_MdllctrStatus ModelLocator::find_missing_element()
{
   while (pTknCurr->isGroup()) {
      if (pTknCurr->isAndGroup()) {
         pTknCurr = pTknCurr->child;           // restart from beginning
         if (!find_first_required_sibling_unhit()) {
            return MDLLCTR_INTERNAL_ERROR;     // it *must* exist
         }
         if (!pTknCurr->isYounger()) {         // more siblings
            ModelToken const* pTknSave = pTknCurr;
            pTknCurr = pTknCurr->sibling;      // start at next unhit sibling
            if (find_first_required_sibling_unhit()) {
               return MDLLCTR_UNCLOSABLE;      // at least 2 choices...
            }
            pTknCurr = pTknSave;
         }
      }else if (pTknCurr->isSeqGroup()) {
         pTknCurr = pTknCurr->child;           // restart from beginning
         if (!find_first_required_sibling_unhit()) {
            return MDLLCTR_INTERNAL_ERROR;     // it *must* exist
         }
      }else {  /* isOrGroup() */
         return MDLLCTR_UNCLOSABLE;            // undeterminated element missing
      }
   }
   return MDLLCTR_MISSING;                     // missing element found
}

/*------------------------------------------------------------------isValidEnd-+
| Determine if the current content model can be ended.                         |
|                                                                              |
| Try to find a way out from the model tree starting at the current token.     |
| Climb the tree up while setting the hit bits.  Then check the highest node.  |
|                                                                              |
| On Entry:                                                                    |
|   (no parameters)                                                            |
|                                                                              |
| When returning:                                                              |
|                                                                              |
|   Return code is: (see mdllctr.h for explanations)                           |
|                                                                              |
|     MDLLCTR_MISSING                                                          |
|     MDLLCTR_CLOSED                                                           |
|     MDLLCTR_UNCLOSABLE                                                       |
|     MDLLCTR_INTERNAL_ERROR  An internal error occurred.                      |
|                                                                              |
+-----------------------------------------------------------------------------*/
e_MdllctrStatus ModelLocator::isValidEnd()
{
   if (!pTknCurr || !mdl.isModel()) {         // model has ended, or it's Nil,
      return MDLLCTR_CLOSED;                  // or it's declared.  That's it
   }
   for (
      ModelToken const* pTknParent = pTknCurr;
      pTknParent = pTknParent->parent(), pTknParent;
      pTknCurr = pTknParent
   ) {
      if (pTknParent->isOrGroup()) {          // OR group is satisfied
         if (isHit(pTknCurr)) {               // if a token was hit
            hit(pTknParent);                  // turn on group hit bit
         }
      }else {                                 // and_group or seq_group
         if (isSome_hits(pTknParent)) {       // some were hit
            if (pTknParent->isAndGroup()) {
               pTknCurr = pTknParent->child;  // restart from beginning
            }
            if (find_first_required_sibling_unhit()) {  // required token...
               return find_missing_element();
            }
            hit(pTknParent);                  // turn on group hit bit
         }
      }
   }
   if ((isHit(pTknCurr)) || (pTknCurr->isOmissible())) {
      pTknCurr = 0;                           // group can be safely closed
      return MDLLCTR_CLOSED;                  // hit, or not but allowed to
   }
   if (pTknCurr->isOrGroup()) {
      return MDLLCTR_UNCLOSABLE;
   }
   return find_missing_element();             // MISSING or UNCLOSABLE
}

/*===========================================================================*/
