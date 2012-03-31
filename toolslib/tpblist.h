/*
* $Id: tpblist.h,v 1.6 2002-04-14 23:26:04 jlatone Exp $
*
* Base class for lists.
*/

#ifndef COM_JAXO_TOOLS_TPBLIST_H_INCLUDED
#define COM_JAXO_TOOLS_TPBLIST_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"
#include "RefdKey.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-------------------------------------------------------------------TpListRep-+
| List Rep - internal class                                                    |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpListRep {
   friend class TpListBase;
private:
   #if (defined(__IBMCPP__) || defined (__IBMC__)) && defined(__DEBUG_ALLOC__)
      enum { INCR=4 };                      // stress algorithms a wee bit more
   #else
      enum { INCR=16 };                     // iRealCount is n * this
   #endif
   RefdItem * aKi;                         // Array of `iCount' items
   TpListRep(                               // Regular
     bool & isOk,
     TpListRep const* repSource,
     RefdItem const* aKiSource,
     int iCountKi,
     int ixKi,
     int iCountKiRplc = 0
   );
   TpListRep(int iAverageCount);            // Special
   ~TpListRep();

   int iReferenceCount;
   int iCount;                              // Count of items in the list
   int iRealCount;                          // Threshold

   bool quickInsert(
      RefdItem const* aKiSource,
      int iCountKi,
      int ixKi
   );
   bool quickReplace(
      RefdItem const* aKiSource,
      int iCountKi,
      int ixKi
   );
   bool quickRemove(int ixKi);
   static int roundToIncr(int iCount);
   TpListRep& operator=(const TpListRep& repSource);    // no!
   TpListRep(const TpListRep& repSource);               // no!
};

/*------------------------------------------------------------------TpListBase-+
| List Base - internal class / do not use                                      |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpListBase {
public:
   enum e_FillAtInit { FillWithEmptyItems };   // dangerous!!!

   TpListBase();
   TpListBase(int iAverageCount);              // regular constructor
   TpListBase(int iCount, e_FillAtInit);       // dangerous constructor
   TpListBase(TpListBase const& lstSource);    // copy constructor
   ~TpListBase();                              // destructor
   TpListBase& operator=(                      // assignment
      TpListBase const& lstSource
   );
   bool      b_isAlive() const;
   unsigned int b_count() const;
   unsigned int b_getCount() const;            // no checks!
   RefdItem *  b_access(int ixKi) const;      // no checks!
   RefdItem *  b_insert(RefdItem const & kiSource, int ixKi);
   RefdItem *  b_replace(RefdItem const & kiSource, int ixKi);
   bool      b_remove(int ixKi);            // remove from given position
   RefdItem *  b_find(int ixKi) const;
   bool      b_insert(TpListBase const& lstSource, int ixKi);
   bool      b_replace(TpListBase const& lstSource, int ixKi);
   int          b_ix(RefdItem const * pKi) const;  // % this list
   int          b_ix(RefdItemRep const * pKirep) const;
   bool      b_isItemShared(RefdItemRep const * pKirep) const;
   RefdItem * inqArrayPtr() const;

private:
   TpListRep * pRep;
   void cleanup();
public:
   static TpListBase const Nil; // should be here: CSET2 bug (csd 11)
};

/* -- INLINES -- */
inline TpListBase::TpListBase() {
   pRep = 0;
}
inline void TpListBase::cleanup() {
   if ((pRep) && (--pRep->iReferenceCount <= 0)) {
      delete pRep;
   }
   pRep = 0;                            // void this instance
}
inline TpListBase::~TpListBase() {
   cleanup();
}
inline bool TpListBase::b_isAlive() const {
   if (pRep) return true; else return false;
}
inline unsigned int TpListBase::b_count() const {
   if (pRep) return pRep->iCount; else return 0;
}
inline bool TpListBase::b_isItemShared(RefdItemRep const * pKirep) const {
   if ((pKirep->inqRefCount() > 1) || (pRep && pRep->iReferenceCount > 1)) {
      return true;
   }else {
      return false;
   }
}
inline unsigned int TpListBase::b_getCount() const {
   return pRep->iCount;
}
inline RefdItem * TpListBase::b_access(int ixKi) const {
   return pRep->aKi + ixKi;
}
inline RefdItem * TpListBase::b_find(int ixKi) const {
   if ((pRep) && (ixKi >= 0) && (ixKi < pRep->iCount)) {
      return pRep->aKi + ixKi;
   }else {
      return 0;
   }
}
inline RefdItem * TpListBase::inqArrayPtr() const {
   return pRep->aKi;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
