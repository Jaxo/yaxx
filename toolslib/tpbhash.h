/*
* $Id: tpbhash.h,v 1.7 2002-04-14 23:26:04 jlatone Exp $
*
* Base class for hashs.
*/

#ifndef COM_JAXO_TOOLS_TPBHASH_H_INCLUDED
#define COM_JAXO_TOOLS_TPBHASH_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"
#include "tplist.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-----------------------------------------------------------------TpHashBase-+
| Hash class                                                                  |
+----------------------------------------------------------------------------*/
class TOOLS_API TpHashBase {                 // HSH
   friend class TpHashBaseIterator;
public:
   TpHashBase(unsigned int iSize = 16);       // regular constructor
   TpHashBase(                                // constructor from TpListBase
      TpListBase const & lstSource,
      unsigned int iSize = 16
   );
   TpHashBase(TpHashBase const& hshSource);   // copy constructor
   ~TpHashBase();                             // destructor
   TpHashBase& operator=(TpHashBase const& hshSource); // assignment
   bool         b_isValid() const;
   unsigned int    b_count() const;
   RefdKey *  b_insert(RefdKey const & ki);
   RefdKey *  b_insert(RefdKey const & ki, bool & isNew);
   RefdKey *  b_force(RefdKey const & ki);
   bool         b_remove(UnicodeString const & ucsKey);
   bool         b_remove(RefdKey const * pKi);
   bool         b_remove(int ixKi);
   RefdKey *  b_find(int ixKi) const;
   RefdKey *  hshfind(UnicodeString const & ucsKey) const;

private:
   TpListBase * aLst;                  // Array of buckets
   unsigned int iMask;                 // Number of buckets - 1 (2 ** n)
   unsigned int iCount;                // Total count of entries

   static int makeMask(int i);
   int bucketsCount() const;
   void cleanup();
   TpListBase & chooseBucket(UnicodeString const & ucsKey) const;
   TpListBase & chooseBucket(int & ixKi) const;
};

/* -- INLINES -- */
inline void TpHashBase::cleanup() {
   delete [] aLst;
   aLst = 0;
   iCount = 0;
}
inline TpHashBase::~TpHashBase() {
   cleanup();
}
inline bool TpHashBase::b_isValid() const {
   if (aLst) return true; else return false;
}
inline unsigned int TpHashBase::b_count() const {
   return iCount;
}

/*---------------------------------------------------------TpHashBaseIterator-+
| Iterator class                                                              |
+----------------------------------------------------------------------------*/
class TpHashBaseIterator {
public:
   TpHashBaseIterator(TpHashBase * pHshArg, int ixKiFrom=0);
   TpHashBaseIterator(TpHashBase * pHshArg, UnicodeString const & ucsFrom);
   RefdKey * operator()();           // iterate thru all entries...
   bool remove();                      // remove current item;
   int ix() const;                        // index current item
   operator RefdKey *() const;       // tells what item is current
   RefdKey * operator->() const;     // tells what item is current
private:
   TpHashBase * pHsh;
   RefdKey * pKiCur;
   TpListBase * pLstBucket;
   int ixInBucket;
   int ixKi;
};

inline int TpHashBaseIterator::ix() const {
   return ixKi;
}
inline TpHashBaseIterator::operator RefdKey *() const {
   return pKiCur;
}
inline RefdKey * TpHashBaseIterator::operator->() const {
   return pKiCur;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
