/*
* $Id: tplistwh.h,v 1.6 2002-04-14 23:26:04 jlatone Exp $
*
* List class with a (HAS-A) hash method to speed up the search.
* Items must be unique: it's not allowed to have the same key,
* but a different content.
* Important => uniqueness does not mean "not duplicata".  The
* list can refer to the same object several times, but it is
* guaranteed that this object is unique.
*/

#ifndef COM_JAXO_TOOLS_TPLISTWH_H_INCLUDED
#define COM_JAXO_TOOLS_TPLISTWH_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include "toolsgendef.h"
#include "RefdKey.h"
#include "tpblist.h"
#include "tpbhash.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------------------TpListWithHash-+
| ListWithHash class  - A list class that HAS-A hash                           |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpListWithHash :             // LSTWH
   private TpListBase
{
public:
   TpListWithHash(                           // Regular constructor
      unsigned int iAverageCount=0,
      unsigned short usModArg=10
   );
   TpListWithHash(                           // to build empty lists (dangerous)
      unsigned int iCount,
      unsigned short usModArg,
      TpListBase::e_FillAtInit
   );
   TpListWithHash(
      TpKeyList const & lstSource,
      unsigned short usModArg=10
   );
   TpListWithHash(TpListWithHash const & hlstSource);

   //--- KeyCollection:
   bool insert(RefdKeyRep const * pKirep);      // same as insertLast()
   bool operator+=(RefdKeyRep const * pKirep);  // same as insertLast()
   bool isItemShared(RefdKeyRep const * pKirep) const;
   bool remove(UnicodeString const & ucsKey);

   // count of items
   unsigned int count() const;

   operator void*() const;                       // is the collection OK?
   bool operator!() const;                    // is not ok?


   //--- KeySequence:
   // insert new item at a given position, or replace an existing one
   bool insertAt(RefdKeyRep const * pKirep, int ixKi);

   // same, at beginning of the sequence
   bool insertFirst(RefdKeyRep const * pKirep);

   // same, at end of the sequence
   bool insertLast(RefdKeyRep const * pKirep);

   // if new key: replace at a given position; if not, replace existing one
   bool replaceAt(RefdKeyRep const * pKirep, int ixKi);

   // get item index (position)
   int ix(UnicodeString const & ucsKey) const;
   int ix(RefdKeyRep const * pKirep) const;

   UnicodeString const & findKey(int ixKi) const;

   // remove an item at some position
   bool removeAt(int ixKi);                 // remove from given position
   bool removeFirst();                      // remove first
   bool removeLast();                       // remove last
   bool operator--();                       // as RemoveLast

   //--- specific to this class
   // same functions as above, but source is another list
   bool insert(TpListWithHash const& hlstSource, int ixKi);  //>>> TO DO
   bool replace(TpListWithHash const& hlstSource, int ixKi); //>>> TO DO
   bool insertFirst(TpListWithHash const& hlstSource);
   bool insertLast(TpListWithHash const& hlstSource);
   bool operator+=(TpListWithHash const& hlstSource);

   // find item from index / key
   RefdKeyRep const * find(int ix) const;
   RefdKeyRep const * find(UnicodeString const & ucsKey) const;

   // Do not use.  This is to ensure backward compatibility.
   unsigned int maxIx() const;     // use count()
   bool isOk() const;           // use operator void *()

protected:
   // Following functions are protected: they return void *.
   // They should be kept protected until one of the derived class
   // casts the return value to a pointer to the real item data (or 0).
   // (see TP_MakeList)

   //--- KeyCollection:
   // Find an item from its key.  Returns NULL if inexistent
   void * findData(                            // NULL if not found
      UnicodeString const & ucsKey             // Item key
   ) const;

   // For all items, do f()
   void applyData(
       RefdItemRep::ApplyReturnCode(*f)(void * pItem, void * pUser),
       void * pUser
   );

   //--- Sequence
   // Find an item from its position.  Returns NULL if inexistent
   void * findDataAt(int ixKi) const;          // NULL if out of bounds
   void * firstData() const;                   // first item
   void * lastData() const;                    // last item

   // Returns item data from a key and a starting index (position.)
   // Set its actual index (if inexistent, index is -1 and 0 returned)
   // Note: to simply get an index, it's much faster to use ix(ucsKey).
   void * findDataFrom(                        // NULL if not found
      UnicodeString const & ucsKey,            // Item key
      int & ixKi                               // Input/Output
   ) const;

   // For all items starting at ixFrom, do f()
   void applyDataFrom(
       RefdItemRep::ApplyReturnCode(*f)(void * pData, void * pUser),
       void * pUser,
       int ixFrom
   );

private:
   TpHashBase hsh;   // initialized after TpList!

   RefdKey * m_insert(RefdKey const& kiSource, int ixKi);
   RefdKey * m_replace(RefdKey const& kiSource, int ixKi);
   bool insert(RefdItemRep const *)           { assert  (false); return false; }
   bool operator+=(RefdItemRep const *)       { assert  (false); return false; }
   bool isItemShared(RefdItemRep const *) const {assert  (false);return false; }
   bool insertAt(RefdItemRep const *, int)    { assert  (false); return false; }
   bool insertFirst(RefdItemRep const *)      { assert  (false); return false; }
   bool insertLast(RefdItemRep const *)       { assert  (false); return false; }
   bool replaceAt(RefdItemRep const *, int)   { assert  (false); return false; }
   int ix(RefdItemRep const *) const             { assert  (false); return -1; }
};

/* -- INLINES -- */
inline TpListWithHash::TpListWithHash(
   unsigned int iAverageCount,
   unsigned short usModArg
) :
   TpListBase(iAverageCount), hsh(usModArg) {
}
inline TpListWithHash::TpListWithHash(
   unsigned int iCount,
   unsigned short usModArg,
   TpListBase::e_FillAtInit ef
) :
   TpListBase(iCount, ef), hsh(usModArg) {
}
inline TpListWithHash::TpListWithHash(
   TpKeyList const & lstSource, unsigned short usModArg
) :
   TpListBase(lstSource.inqListBase()),
   hsh(lstSource.inqListBase(), usModArg)
{
}
inline TpListWithHash::TpListWithHash(
   TpListWithHash const & hlstSource
) :
   TpListBase(hlstSource), hsh(hlstSource.hsh)
{}
inline bool TpListWithHash::insert(RefdKeyRep const * pKirep) {
   if (m_insert(pKirep, b_count())) {
      return true;
   }else {
      return false;
   }
}
inline bool TpListWithHash::operator+=(RefdKeyRep const * pKirep) {
   if (m_insert(pKirep, b_count())) {
      return true;
   }else {
      return false;
   }
}
inline bool TpListWithHash::isItemShared(RefdKeyRep const * pKirep) const {
   return b_isItemShared(pKirep);
}
inline unsigned int TpListWithHash::count() const {
   return b_count();
}
inline TpListWithHash::operator void *() const {
   if (b_isAlive() && hsh.b_isValid()) return (void *)this; else return 0;
}
inline bool TpListWithHash::operator!() const {
   if (b_isAlive() && hsh.b_isValid()) return false; else return true;
}
inline bool TpListWithHash::insertAt(RefdKeyRep const * pKirep, int ixKi) {
   if (m_insert(pKirep, ixKi)) {
      return true;
   }else {
      return false;
   }
}
inline bool TpListWithHash::insertFirst(RefdKeyRep const * pKirep) {
   if (m_insert(pKirep, 0)) {
      return true;
   }else {
      return false;
   }
}
inline bool TpListWithHash::insertLast(RefdKeyRep const * pKirep) {
   if (m_insert(pKirep, b_count())) {
      return true;
   }else {
      return false;
   }
}
inline bool TpListWithHash::replaceAt(RefdKeyRep const * pKirep, int ixKi) {
   if (m_replace(pKirep, ixKi)) {
      return true;
   }else {
      return false;
   }
}
inline bool TpListWithHash::removeFirst() {
   return removeAt(0);
}
inline bool TpListWithHash::removeLast() {
   return removeAt(b_count()-1);
}
inline bool TpListWithHash::operator--() {
   return removeAt(b_count()-1);
}
inline int TpListWithHash::ix(UnicodeString const & ucsKey) const {
   return b_ix(hsh.hshfind(ucsKey));
}
inline int TpListWithHash::ix(RefdKeyRep const * pKirep) const {
   return b_ix(pKirep);
}
inline RefdKeyRep const *TpListWithHash::find(
   UnicodeString const & ucsKey
) const {
   return hsh.hshfind(ucsKey)->inqKi();
}
inline RefdKeyRep const * TpListWithHash::find(int ix) const {
   return ((RefdKey *)b_find(ix))->inqKi();
}
inline void * TpListWithHash::findData(UnicodeString const & ucsKey) const {
   RefdKey * pKi = hsh.hshfind(ucsKey);
   if (pKi) return pKi->inqData(); else return 0;
}
inline void TpListWithHash::applyData(
   RefdItemRep::ApplyReturnCode(*f)(void * pItem, void * pUser),
   void * pUser
) {
   applyDataFrom(f, pUser, 0);
}
inline UnicodeString const & TpListWithHash::findKey(int ix) const {
   RefdKey * pKi = (RefdKey *)b_find(ix);
   if (pKi) return pKi->key(); else return UnicodeString::Nil;
}
inline void * TpListWithHash::findDataAt(int ix) const {
   RefdKey * pKi = (RefdKey *)b_find(ix);
   if (pKi) return pKi->inqData(); else return 0;
}
inline void * TpListWithHash::firstData() const {
   if (b_isAlive()) return b_access(0)->inqData(); else return 0;
}
inline void * TpListWithHash::lastData() const {
   if (b_isAlive()) {
      return b_access(b_getCount() - 1)->inqData();
   }else {
      return 0;
   }
}
inline unsigned int TpListWithHash::maxIx() const {
   return b_count();
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
