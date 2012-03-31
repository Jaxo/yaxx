/*
* $Id: arglist.h,v 1.7 2011-07-29 10:26:37 pgr Exp $
*/

#ifndef COM_JAXO_TOOLS_ARGLIST_H_INCLUDED
#define COM_JAXO_TOOLS_ARGLIST_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <new>
#include "toolsgendef.h"
#include "ucstring.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------------------ArgListSimple-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API ArgListSimple {
   friend class ArgListWithName;

public:
   ArgListSimple();
   ArgListSimple(unsigned int iEstimatedCount);
   ArgListSimple(ArgListSimple const & source);
   ~ArgListSimple();
   ArgListSimple & operator=(ArgListSimple const & source);

   bool operator+=(UnicodeString const & source);
   ArgListSimple copy() const;

   bool isEmpty() const;
   unsigned int count() const;
   UnicodeString const & operator[](int i) const;
   UnicodeString const & operator[](UnicodeString const & ucs) const;
   bool operator==(ArgListSimple const & source) const;
   bool operator!=(ArgListSimple const & source) const;

   // Migration Only:
   UnicodeString const & operator[](UCS_2 const * pUcSource) const;

private:
   class ArgListRep {
   public:
      int iReferenceCount;
      unsigned int iArgsCount;
      unsigned int iRealCount;
      UnicodeString * aUcs;

      ArgListRep();
      ~ArgListRep();
   private:
      ArgListRep& operator=(ArgListRep const & source); // no!
      ArgListRep(ArgListRep const & source);            // no!
   };
   ArgListRep *pRep;

   int inqIx(UnicodeString const &) const;
   bool setSize(unsigned int iNewSize);

public:
   static ArgListSimple const Nil;
};

/* -- INLINES -- */
inline ArgListSimple::ArgListRep::ArgListRep() :
   iReferenceCount(1),
   iArgsCount(0),
   iRealCount(0),
   aUcs(0)
{
}
inline ArgListSimple::ArgListRep::~ArgListRep() {
   delete [] aUcs;
}
inline ArgListSimple::ArgListSimple() : pRep(0) {
}
inline ArgListSimple::ArgListSimple(unsigned int iEstimatedCount) : pRep(0) {
   if (iEstimatedCount) setSize(iEstimatedCount);
}
inline ArgListSimple::ArgListSimple(ArgListSimple const & source) {
   pRep = source.pRep;
   if (pRep) ++pRep->iReferenceCount;
}
inline ArgListSimple::~ArgListSimple() {
   if ((pRep) && (!--pRep->iReferenceCount)) {
      delete pRep;
   }
   pRep = 0;                            // void this instance
}
inline ArgListSimple& ArgListSimple::operator=(ArgListSimple const & source) {
   this->~ArgListSimple();
   return * new(this) ArgListSimple(source);
}
inline bool ArgListSimple::isEmpty() const {
   if (pRep) return false; else return true;
}
inline bool ArgListSimple::operator!=(ArgListSimple const & source) const {
   if (*this == source) return false; else return true;
}
inline unsigned int ArgListSimple::count() const {
   if (pRep) return pRep->iArgsCount; else return 0;
}
inline UnicodeString const & ArgListSimple::operator[](int i) const {
   if ((i < 0) || ((unsigned int)i >= count())) {
      return UnicodeString::Nil;
   }else {
      return pRep->aUcs[i];
   }
}

/*-------------------------------------------------------------------NamedArg-+
| A named argument is made of a symbolic name followed by its value           |
+----------------------------------------------------------------------------*/
class NamedArg {
public:
   NamedArg() {};
   NamedArg(
      UnicodeString const & ucsNameArg,
      UnicodeString const & ucsValArg
   );
   UnicodeString const & inqName() const;
   UnicodeString const & inqValue() const;
private:
   UnicodeString ucsName;
   UnicodeString ucsVal;
public:
   static NamedArg const Nil;
};

/* -- INLINES -- */
inline NamedArg::NamedArg(
   UnicodeString const & ucsNameArg,
   UnicodeString const & ucsValArg
) :
   ucsName(ucsNameArg), ucsVal(ucsValArg)
{}
inline UnicodeString const & NamedArg::inqName() const {
   return ucsName;
}
inline UnicodeString const & NamedArg::inqValue() const {
   return ucsVal;
}
inline ostream& operator<<(ostream & out, NamedArg const & source) {
   out << source.inqName() << "=" << source.inqValue();
   return out;
}

/*------------------------------------------------------------ArgListWithName-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API ArgListWithName : private ArgListSimple {  // IS-LIKE-A
public:
   ArgListWithName();
   ArgListWithName(unsigned int iEstimatedCount);
   // copy, assgnt op are the default

   bool operator+=(NamedArg const & source);
   ArgListSimple::copy;

   ArgListSimple::isEmpty;
   unsigned int count() const;
   NamedArg operator[](int i) const;
   UnicodeString const & operator[](UnicodeString const &) const;

   bool operator==(ArgListWithName const & source) const;
   bool operator!=(ArgListWithName const & source) const;

private:
   int inqIx(UnicodeString const &) const;

public:
   static ArgListWithName const Nil;
};

/* -- INLINES -- */
inline ArgListWithName::ArgListWithName() {
}
inline ArgListWithName::ArgListWithName(unsigned int iEstimatedCount) {
   if (iEstimatedCount) setSize(iEstimatedCount << 1);
}
inline NamedArg ArgListWithName::operator[](int i) const {
   if ((i < 0) || (i<<=1, (unsigned int)i >= ArgListSimple::count())) {
      return NamedArg::Nil;
   }else {
      return NamedArg(pRep->aUcs[i], pRep->aUcs[i+1]);
   }
}
inline bool ArgListWithName::operator!=(
   ArgListWithName const & source
) const {
   if (*this == source) return false; else return true;
}
inline unsigned int ArgListWithName::count() const {
   return ArgListSimple::count() / 2;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
