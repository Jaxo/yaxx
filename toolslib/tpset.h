/*
* $Id: tpset.h,v 1.4 2002-04-14 23:26:04 jlatone Exp $
*
* Unordered collection of keyed items.
*/

#ifndef COM_JAXO_TOOLS_TPSET_H_INCLUDED
#define COM_JAXO_TOOLS_TPSET_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include "toolsgendef.h"
#include "RefdKey.h"
#include "tpbhash.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-----------------------------------------------------------------TP_MakeSet-+
| Provides an easy way to generate TpSet's methods to collect a class of item.|
| ItemClass is the name of a class derived from RefdKeyRep                    |
|                                                                             |
| Typical Use:                                                                |
|                                                                             |
| class PhoneEntry : public RefdKeyRep {                                      |
|    (* ... methods and data of your class ... *)                             |
| };                                                                          |
|                                                                             |
| class PhoneList : public TpSet {                                            |
|    TP_MakeSet(PhoneEntry);                                                  |
| };                                                                          |
|                                                                             |
| This will define the usual functions for unordered collection,              |
| as well as a PhoneList::Iterator to sequencially walk thru the list.        |
|                                                                             |
+----------------------------------------------------------------------------*/
#define TP_MakeSet(ItemClass)\
public:\
   ItemClass * insertIfNotThere(ItemClass const *pKiRep) {\
      return (ItemClass *)findDataOrCreate(pKiRep); }\
   class Iterator : public TpSetIterator { public:\
      Iterator(TpSet & set, int ix=0):TpSetIterator(& set, ix) {}\
      Iterator(TpSet & set, UnicodeString const & ucs):\
         TpSetIterator(&set, ucs) {}\
      Iterator(TpSet & set, UCS_2 const * pUc):\
         TpSetIterator(&set, UcStringFlash(pUc)) {}\
      ItemClass * operator()() {\
         if (it()) return (ItemClass *)it->inqData(); else return 0; }\
      ItemClass * operator->() const {\
         if (it) return (ItemClass *)it->inqData(); else return 0; }}

/*----------------------------------------------------------------------TpSet-+
| Set                                                                         |
+----------------------------------------------------------------------------*/
class TOOLS_API TpSet :                      // HSH
   private TpHashBase
{
   friend class TpSetIterator;
public:
   TpSet(unsigned short const usModArg=10);
   TpSet(                                    // constructor from TpKeyList
      TpKeyList const & source,
      unsigned short usModArg = 10
   );
   TpSet(TpSet const& source);               // copy constructor

   unsigned int count() const;               // count items in the list
   operator void*() const;                   // is the list OK?
   bool operator!() const;                // is not ok?

   // Insert a unique item and return false if the item
   // was not inserted (possibly, because it was a duplicate)
   bool insert(RefdKeyRep const * pKirep);
   bool operator+=(RefdKeyRep const * pKirep);
   bool isItemShared(RefdKeyRep const * pKirep) const;

   // Insert an item, even if duplicated
   bool force(RefdKeyRep const * pKirep);

   // remove an item
   bool remove(UnicodeString const & ucsKey);
   bool remove(int ixKi);

   RefdKeyRep const * find(UnicodeString const & ucsKey) const;

protected:
   // Following functions are protected: they return void *: the derived
   // list class should cast it to a pointer to the real item (or 0).
   // (see TP_MakeSet)

   // insert an item if not already there
   // if it's there, don't duplicate and return original item
   // if brand new, then insert it and return new item,
   void * findDataOrCreate(RefdKeyRep const * pKirep);

   // find an item from its key.  Returns NULL if inexistent
   void * findData(                            // find an item
      UnicodeString const & ucsKey             // item key
   ) const;
   void applyData(                             // for all items...
       RefdItemRep::ApplyReturnCode (*f)(void *, void *),
       void * pUser
   );

private:
   bool insert(RefdItemRep const *)             {assert  (false);return false;}
   bool operator+=(RefdItemRep const *)         {assert  (false);return false;}
   bool isItemShared(RefdItemRep const *) const {assert  (false);return false;}

public:
   static TpSet const Nil;  // should be here: CSET2 bug (csd 11)
};

/* -- INLINES -- */
inline TpSet::TpSet(unsigned short const usModArg) : TpHashBase(usModArg) {
}
inline TpSet::TpSet(TpKeyList const& source, unsigned short usModArg) :
   TpHashBase(source.inqListBase(), usModArg) {
}
inline TpSet::TpSet(TpSet const& source) : TpHashBase(source) {
}
inline unsigned int TpSet::count() const {
   return b_count();
}
inline TpSet::operator void *() const {
  if (b_isValid()) return (void *)this; else return 0;
}
inline bool TpSet::operator!() const {
  if (b_isValid()) return false; else return true;
}
inline bool TpSet::remove(UnicodeString const & ucsKey) {
   return b_remove(ucsKey);
}
inline bool TpSet::remove(int ixKi) {
   return b_remove(ixKi);
}
inline bool TpSet::insert(RefdKeyRep const * pKirep) {
   bool isNew;
   RefdItem * pKi = b_insert(pKirep, isNew);
   if (pKi && isNew) return true; else return false;
}
inline bool TpSet::operator+=(RefdKeyRep const * pKirep) {
   return insert(pKirep);
}
inline bool TpSet::isItemShared(RefdKeyRep const * pKirep) const {
   if (pKirep->inqRefCount() > 1) return true;  else return false;
}
inline bool TpSet::force(RefdKeyRep const * pKirep) {
   RefdKey * pKi;
   if (pKi = b_force(pKirep), pKi) {
      return true;
   }else {
      return false;
   }
}
inline void * TpSet::findDataOrCreate(RefdKeyRep const * pKirep) {
   RefdKey * pKi;
   if (pKi = b_insert(pKirep), pKi) {
      return pKi->inqData();
   }else {
      return 0;
   }
}
inline RefdKeyRep const * TpSet::find(UnicodeString const & ucsKey) const {
   return hshfind(ucsKey)->inqKi();
}
inline void * TpSet::findData(UnicodeString const & ucsKey) const {
   RefdKey * pKi = hshfind(ucsKey);
   if (pKi) return pKi->inqData(); else return 0;
}

/*--------------------------------------------------------------TpSetIterator-+
| Iterator class                                                              |
+----------------------------------------------------------------------------*/
class TpSetIterator {
public:
   TpSetIterator(TpSet * set);
   TpSetIterator(TpSet * set, int ixKiFrom);
   TpSetIterator(TpSet * set, UnicodeString const & ucsFrom);
   int ix() const;
protected:
   TpHashBaseIterator it;
};

/* -- INLINES -- */
inline TpSetIterator::TpSetIterator(TpSet * pSet) : it(pSet) {
}
inline TpSetIterator::TpSetIterator(TpSet * pSet, int ixKiFrom
) : it(pSet, ixKiFrom) {
}
inline TpSetIterator::TpSetIterator(
   TpSet * pSet, UnicodeString const & ucsFrom
) : it(pSet, ucsFrom) {
}
inline int TpSetIterator::ix() const {
   return it.ix();
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
