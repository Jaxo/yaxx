/* $Id: RefdItem.h,v 1.7 2011-07-29 10:26:36 pgr Exp $ */

#ifndef COM_JAXO_TOOLS_REFDITEM_H_INCLUDED
#define COM_JAXO_TOOLS_REFDITEM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <new>
#include "toolsgendef.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------------------------RefdItemRep-+
| Base class for item.   Reference counted.                                   |
+----------------------------------------------------------------------------*/
class TOOLS_API RefdItemRep {           // itrep
   friend class RefdItem;
public:
   enum ApplyReturnCode {
      ContinueApply, BreakApply, RemoveItemAndContinueApply
   };
   RefdItemRep();                        // null constructor
   int inqRefCount() const;
protected:
   virtual ~RefdItemRep() {}             // virtual destructor

private:
   int iReferenceCount;
   RefdItemRep& operator=(RefdItemRep const& repSource); // no!
   RefdItemRep(RefdItemRep const& repSource);            // no!
};

/* -- INLINES -- */
inline RefdItemRep::RefdItemRep() : iReferenceCount(0) {
}
inline int RefdItemRep::inqRefCount() const {
  return iReferenceCount;
}

/*-------------------------------------------------------------------RefdItem-+
| Referenced Item                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API RefdItem {                 // it
   friend class RefdKey;
public:
   RefdItem() { pRep = 0; };               // Null constructor
   RefdItem(RefdItemRep const * pSource);  // Regular
   RefdItem(RefdItem const & source);      // Copy
   ~RefdItem();
   RefdItem & operator=(RefdItem const & source);
   RefdItem & operator=(RefdItemRep const * pSource);  // short cut


   bool operator==(RefdItem const & ki) const;
   bool operator!=(RefdItem const & ki) const;
   bool operator==(RefdItemRep const * pKirep) const;
   bool operator!=(RefdItemRep const * pKirep) const;

   bool isPresent() const;
   bool isShared() const;

   void * inqData() const;                 // should be virtual? protected?

private:
   RefdItemRep * pRep;

public:
   static RefdItem const Nil;
};

/* -- INLINES -- */
inline RefdItem::RefdItem(RefdItem const & source) {
   pRep = source.pRep;
   if (pRep) ++pRep->iReferenceCount;
}
inline RefdItem::RefdItem(RefdItemRep const * pSource) {
   pRep = (RefdItemRep *)pSource;  // violates constness (but guaranteed to be)
   if (pRep) ++pRep->iReferenceCount;
}
inline RefdItem::~RefdItem() {
   if ((pRep) && (!--pRep->iReferenceCount)) {
      delete pRep;
   }
   pRep = 0;                            // void this instance
}
inline RefdItem& RefdItem::operator=(RefdItem const & source) {
   if (pRep != source.pRep) {
      this->~RefdItem();
      new(this) RefdItem(source);
   }
   return *this;
}
inline RefdItem& RefdItem::operator=(RefdItemRep const * pSource) {
   if (pRep != pSource) {
      if (pRep) this->~RefdItem();
      new(this) RefdItem(pSource);
   }
   return *this;
}
inline bool RefdItem::operator==(RefdItem const & ki) const {
   if (pRep == ki.pRep) return true; else return false;
}
inline bool RefdItem::operator!=(RefdItem const & ki) const {
   if (pRep != ki.pRep) return true; else return false;
}
inline bool RefdItem::operator==(RefdItemRep const * pKirep) const {
   if (pRep == pKirep) return true; else return false;
}
inline bool RefdItem::operator!=(RefdItemRep const * pKirep) const {
   if (pRep != pKirep) return true; else return false;
}
inline bool RefdItem::isPresent() const {
   if (pRep) return true; else return false;
}
inline bool RefdItem::isShared() const {
   return (pRep->iReferenceCount > 1);
}
inline void * RefdItem::inqData() const {
   return (void *)pRep;
}

/*-----------------------------------------------------------------TP_MakeXXX-+
| Provides an easy way to generate collections methods.                       |
| ItemClass is the name of a class derived from RefdKeyRep.                   |
|                                                                             |
| Typical Use:                                                                |
|                                                                             |
| class FooEntry : public RefdKeyRep {                                        |
|    (* ... methods and data of your class ... *)                             |
| };                                                                          |
| ------                                                                      |
| class FooCollection : public virtual TpKeyCollection {                      |
|    TP_MakeKeyCollection(FooEntry);                                          |
| };                                                                          |
| ------                                                                      |
| class FooSet : public virtual FooCollection, public virtual TpSet {         |
|    TP_MakeSet(FooEntry);                                                    |
| };                                                                          |
| class FooSequence :                                                         |
|    public virtual FooCollection, public virtual TpKeySequence               |
| {                                                                           |
|    TP_MakeSequence(FooEntry);                                               |
| };                                                                          |
| class FooList : public FooSequence, public TpList {                         |
| };                                                                          |
| class FooListWithHash : public FooSequence, public TpListWithHash {         |
| };                                                                          |
|                                                                             |
+----------------------------------------------------------------------------*/
#define TP_MakeKeyCollection(ItemClass)\
public:\
   ItemClass * operator[](UnicodeString const & ucs) const {\
      return (ItemClass *)findData(ucs); }\
   ItemClass * operator[](UCS_2 const * pUc) const {\
      return (ItemClass *)findData(UcStringFlash(pUc)); }\
   void apply(RefdItemRep::ApplyReturnCode (*f)(ItemClass*, void*),void* pUser) {\
      applyData((RefdItemRep::ApplyReturnCode(*)(void*,void*))f,pUser);}

#define TP_MakeSequence(ItemClass)\
public:\
   ItemClass * operator[](int ix) const {return (ItemClass *)findDataAt(ix);}\
   ItemClass * first() const {return (ItemClass *)firstData();}\
   ItemClass * last() const {return (ItemClass *)lastData();}\
   void applyFrom(\
      RefdItemRep::ApplyReturnCode (*f)(ItemClass*,void*),void* pUser,int ixFrom) {\
      applyDataFrom((RefdItemRep::ApplyReturnCode(*)(void*,void*))f,pUser,ixFrom);}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
