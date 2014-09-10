/*
* $Id: tplist.h,v 1.6 2002-04-14 23:26:04 jlatone Exp $
*
* Ordered collection of keyed items.
*/

#ifndef COM_JAXO_TOOLS_TPLIST_H_INCLUDED
#define COM_JAXO_TOOLS_TPLIST_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include "toolsgendef.h"
#include "RefdKey.h"
#include "tpblist.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------------------------------TpList-+
|                                                                              |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpList:            // LST
   protected TpListBase
{
public:
   TpList();                        // Nil constructor
   TpList(int iAverageCount);       // average number of entries
   TpList(                          // to build empty lists (dangerous)
      int iCount,
      e_FillAtInit
   );
   TpListBase const & inqListBase() const;

   //--- Collection:
   // insert item at the end (same as insertLast)
   bool insert(RefdItemRep const * pKirep);
   bool operator+=(RefdItemRep const * pKirep);
   bool isItemShared(RefdItemRep const * pKirep) const;

   // count of items
   unsigned int count() const;

   operator void*() const;                     // is the collection OK?
   bool operator!() const;                  // is not ok?

   //--- Sequence:
   // insert item at given position
   bool insertAt(RefdItemRep const * pKirep, int ixKi);

   // insert item at beginning of the sequence
   bool insertFirst(RefdItemRep const * pKirep);

   // insert item at end of the sequence
   bool insertLast(RefdItemRep const * pKirep);

   // replace item at given position
   bool replaceAt(RefdItemRep const * pKirep, int ixKi);

   // get index
   int ix(RefdItemRep const * pKirep) const;

   // remove an item at some position
   bool removeAt(int ixKi);                 // remove from given position
   bool removeFirst();                      // remove first
   bool removeLast();                       // remove last
   bool operator--();                       // as RemoveLast

   //--- Specific to this class
   // same functions as above, but source is another list
   bool insertAt(TpList const& lstSource, int ixKi);
   bool insertFirst(TpList const& lstSource);
   bool insertLast(TpList const& lstSource);
   bool operator+=(TpList const& lstSource);
   bool replaceAt(TpList const& lstSource, int ixKi);

protected:
   // Following functions are protected: they return void *.
   // They should be kept protected until one of the derived class
   // casts the return value to a pointer to the real item data (or 0).

   //--- Collection:
   // For all items, do f()
   void applyData(
       RefdItemRep::ApplyReturnCode(*f)(void * pData, void * pUser),
       void * pUser
   );

   //--- Sequence
   // Find an item from its position.  Returns NULL if inexistent
   void * findDataAt(int ixKi) const;          // NULL if out of bounds
   void * firstData() const;                   // first item
   void * lastData() const;                    // last item

   // For all items starting at ixFrom, do f()
   void applyDataFrom(
       RefdItemRep::ApplyReturnCode(*f)(void * pData, void * pUser),
       void * pUser,
       int ixFrom
   );
};

/* -- INLINES -- */
inline TpListBase const & TpList::inqListBase() const {
   TpListBase const & me = *this;  // required by crazy compilers
   return me;
}
inline TpList::TpList() {
}
inline TpList::TpList(int iAverageCount) : TpListBase(iAverageCount) {
}
inline TpList::TpList(
   int iCount, TpListBase::e_FillAtInit ef
) :
   TpListBase(iCount, ef) {
}
inline bool TpList::insert(RefdItemRep const * pKirep) {
   if (b_insert(pKirep, b_count())) return true; else return false;
}
inline bool TpList::operator+=(RefdItemRep const * pKirep) {
   if (b_insert(pKirep, b_count())) return true; else return false;
}
inline bool TpList::isItemShared(RefdItemRep const * pKirep) const {
   return b_isItemShared(pKirep);
}
inline unsigned int TpList::count() const {
   return b_count();
}
inline TpList::operator void *() const {
  if (b_isAlive()) return (void *)this; else return 0;
}
inline bool TpList::operator!() const {
  if (b_isAlive()) return false; else return true;
}
inline bool TpList::insertAt(
   RefdItemRep const * pKirep, int ixKi
) {
   if (b_insert(pKirep, ixKi)) return true; else return false;
}
inline bool TpList::insertFirst(RefdItemRep const * pKirep) {
   if (b_insert(pKirep, 0)) return true; else return false;
}
inline bool TpList::insertLast(RefdItemRep const * pKirep) {
   if (b_insert(pKirep, b_count())) return true; else return false;
}
inline bool TpList::replaceAt(RefdItemRep const * pKirep, int ixKi) {
   if (b_replace(pKirep, ixKi)) return true; else return false;
}
inline bool TpList::removeAt(int ixKi) {
   return b_remove(ixKi);
}
inline bool TpList::removeFirst() {
   return b_remove(0);
}
inline bool TpList::removeLast() {
   return b_remove(b_count()-1);
}
inline bool TpList::operator--() {
   return b_remove(b_count()-1);
}
inline bool TpList::insertAt(TpList const& lstSource, int ixKi) {
   return b_insert(lstSource, ixKi);
}
inline bool TpList::insertFirst(TpList const& lstSource) {
   return b_insert(lstSource, 0);
}
inline bool TpList::insertLast(TpList const& lstSource) {
   return b_insert(lstSource, b_count());
}
inline bool TpList::operator+=(TpList const& lstSource) {
   return b_insert(lstSource, b_count());
}
inline bool TpList::replaceAt(TpList const& lstSource, int ixKi) {
   return b_replace(lstSource, ixKi);
}
inline int TpList::ix(RefdItemRep const * pKirep) const {
   return b_ix(pKirep);
}
inline void TpList::applyData(
   RefdItemRep::ApplyReturnCode(*f)(void * pItem, void * pUser),
   void * pUser
) {
   applyDataFrom(f, pUser, 0);
}
inline void * TpList::findDataAt(int ix) const {
   RefdItem * pKi = b_find(ix);
   if (pKi) return pKi->inqData(); else return 0;
}
inline void * TpList::firstData() const {
   if (b_isAlive()) return b_access(0)->inqData(); else return 0;
}
inline void * TpList::lastData() const {
   if (b_isAlive()) {
      return b_access(b_getCount() - 1)->inqData();
   }else {
      return 0;
   }
}

/*-------------------------------------------------------------------TpKeyList-+
|                                                                              |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpKeyList :            // LST
   private TpList
{
public:
   TpKeyList();                        // Nil constructor
   TpKeyList(int iAverageCount);       // average number of entries
   TpKeyList(                          // to build empty lists (dangerous)
      int iCount,
      TpListBase::e_FillAtInit
   );
   using TpList::inqListBase;

   //--- KeyCollection
   // insert item at the end (same as insertLast)
   bool insert(RefdKeyRep const * pKirep);
   bool operator+=(RefdKeyRep const * pKirep);
   bool isItemShared(RefdKeyRep const * pKirep) const;

   // remove an item, given its key
   bool remove(UnicodeString const & ucsKey);

   // count of items
   using TpList::count;

#if ! (defined(OS__EWS) || defined(OS__HPUX))
   using TpList::operator void*;               // is the collection OK?
#endif
   using TpList::operator!;                    // is not ok?

   //--- KeySequence
   // insert item at given position
   bool insertAt(RefdKeyRep const * pKirep, int ixKi);

   // insert item at beginning of the sequence
   bool insertFirst(RefdKeyRep const * pKirep);

   // insert item at end of the sequence
   bool insertLast(RefdKeyRep const * pKirep);

   // replace item at given position
   bool replaceAt(RefdKeyRep const * pKirep, int ixKi);

   // get index
   int ix(UnicodeString const & ucsKey) const;
   int ix(RefdKeyRep const * pKirep) const;

   // what is the key at index ix?
   UnicodeString const & findKey(int ixKi) const;

   // remove an item at some position
   using TpList::removeAt;                     // remove from given position
   using TpList::removeFirst;                  // remove first
   using TpList::removeLast;                   // remove last
   using TpList::operator--;                   // as RemoveLast

   //--- Specific to this class
   // same functions as above, but source is another list
   bool insertAt(TpList const& lstSource, int ixKi);
   bool insertFirst(TpList const& lstSource);
   bool insertLast(TpList const& lstSource);
   bool operator+=(TpList const& lstSource);
   bool replaceAt(TpList const& lstSource, int ixKi);

   // find item from index / key
   RefdKeyRep const * find(int ix) const;
   RefdKeyRep const * find(UnicodeString const & ucsKey) const;

   // get item index from key and starting position
   int ixFrom(UnicodeString const & ucsKey, int ix)   const;

protected:
   // Following functions are protected: they return void *.
   // They should be kept protected until one of the derived class
   // casts the return value to a pointer to the real item data (or 0).

   //--- KeyCollection:
   // Find an item from its key.  Returns NULL if inexistent
   void * findData(                            // NULL if not found
      UnicodeString const & ucsKey             // Item key
   ) const;

   //--- KeySequence
   using TpList::applyData;
   using TpList::findDataAt;
   using TpList::firstData;
   using TpList::lastData;
   using TpList::applyDataFrom;

   // Returns item data from a key and a starting index (position.)
   // Set its actual index (if inexistent, index is -1 and 0 returned)
   void * findDataFrom(                        // NULL if not found
      UnicodeString const & ucsKey,            // Item key
      int & ixKi                               // Input/Output
   ) const;

private:
   RefdKey *  keyfind(UnicodeString const & ucsKey) const;
   RefdKey *  keyfind(UnicodeString const & ucsKey, int & ixKi) const;
   bool insert(RefdItemRep const *)           { assert  (false); return false; }
   bool operator+=(RefdItemRep const *)       { assert  (false); return false; }
   bool isItemShared(RefdItemRep const *) const {assert  (false);return false; }
   bool insertAt(RefdItemRep const *, int)    { assert  (false); return false; }
   bool insertFirst(RefdItemRep const *)      { assert  (false); return false; }
   bool insertLast(RefdItemRep const *)       { assert  (false); return false; }
   bool replaceAt(RefdItemRep const *, int)   { assert  (false); return false; }
   int ix(RefdItemRep const *) const             { assert  (false); return -1; }

public:
   static TpKeyList const Nil; // should be here: CSET2 bug (csd 11)
};

/* -- INLINES -- */
inline TpKeyList::TpKeyList() {
}
inline TpKeyList::TpKeyList(int iAverageCount) : TpList(iAverageCount) {
}
inline TpKeyList::TpKeyList(
   int iCount, TpListBase::e_FillAtInit ef
) :
   TpList(iCount, ef) {
}

inline bool TpKeyList::insert(RefdKeyRep const * pKirep) {
   return TpList::insert(pKirep);
}
inline bool TpKeyList::operator+=(RefdKeyRep const * pKirep) {
   return TpList::operator+=(pKirep);
}
inline bool TpKeyList::isItemShared(RefdKeyRep const * pKirep) const {
   return TpList::isItemShared(pKirep);
}
inline bool TpKeyList::remove(UnicodeString const & ucsKey) {
   int ixKi = 0;
   RefdKey * pKi = keyfind(ucsKey, ixKi);
   if (pKi) {
      removeAt(ixKi);
      return true;
   }else {
      return false;
   }
}
inline bool TpKeyList::insertAt(RefdKeyRep const * pKirep, int ixKi) {
   return TpList::insertAt(pKirep, ixKi);
}
inline bool TpKeyList::insertFirst(RefdKeyRep const * pKirep) {
   return TpList::insertFirst(pKirep);
}
inline bool TpKeyList::insertLast(RefdKeyRep const * pKirep) {
   return TpList::insertLast(pKirep);
}
inline bool TpKeyList::replaceAt(RefdKeyRep const * pKirep, int ixKi) {
   return TpList::replaceAt(pKirep, ixKi);
}
inline int TpKeyList::ix(RefdKeyRep const * pKirep) const {
   return TpList::ix(pKirep);
}
inline int TpKeyList::ix(UnicodeString const & ucsKey) const {
   int ixRet = 0;
   keyfind(ucsKey, ixRet);
   return ixRet;
}
inline int TpKeyList::ixFrom(UnicodeString const & ucsKey, int ix) const {
   int ixRet = ix;
   keyfind(ucsKey, ixRet);
   return ixRet;
}
inline RefdKeyRep const * TpKeyList::find(UnicodeString const & ucsKey) const {
   return keyfind(ucsKey)->inqKi();
}
inline RefdKeyRep const * TpKeyList::find(int ix) const {
   return ((RefdKey *)b_find(ix))->inqKi();
}
inline void * TpKeyList::findData(UnicodeString const & ucsKey) const {
   RefdKey * pKi = keyfind(ucsKey);
   if (pKi) return pKi->inqData(); else return 0;
}
inline UnicodeString const & TpKeyList::findKey(int ix) const {
   RefdKey * pKi = (RefdKey *)b_find(ix);
   if (pKi) return pKi->key(); else return UnicodeString::Nil;
}
inline void * TpKeyList::findDataFrom(
   UnicodeString const & ucsKey, int & ixKi
) const {
   RefdKey * pKi = keyfind(ucsKey, ixKi);
   if (pKi) return pKi->inqData(); else return 0;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
