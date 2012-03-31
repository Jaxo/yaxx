/*
* $Id: arglist.cpp,v 1.4 2002-04-09 07:34:30 pgr Exp $
*
*  Argument Lists
*/

/*--------------+
| Include Files |
+--------------*/
#include <assert.h>
#include <string.h>
#include "arglist.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

ArgListSimple const ArgListSimple::Nil;
ArgListWithName const ArgListWithName::Nil;
NamedArg const NamedArg::Nil;

/*--------------------------------------------------------ArgListSimple::copy-+
| Deep copy                                                                   |
+----------------------------------------------------------------------------*/
ArgListSimple ArgListSimple::copy() const
{
   unsigned int const iCount = count();

   ArgListSimple copy(iCount);
   for (unsigned int i=0; i < iCount; ++i) {
      copy += pRep->aUcs[i].copy();
   }
   return copy;
}

/*-------------------------------------------------------ArgListSimple::inqIx-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline int ArgListSimple::inqIx(UnicodeString const & ucs) const
{
   unsigned int const iCount = count();
   if (iCount) {
      for (unsigned int i=0; i < iCount; ++i) {
         if (pRep->aUcs[i] == ucs) {
            return i;
         }
      }
   }
   return -1;
}

/*--------------------------------------------------ArgListSimple::operator[]-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString const & ArgListSimple::operator[](
   UnicodeString const & ucsName
) const {
   int const i = inqIx(ucsName);
   if (i == -1) {
      return UnicodeString::Nil;
   }
   return pRep->aUcs[i];
}
UnicodeString const & ArgListSimple::operator[](
   UCS_2 const * pUcSource
) const {
   int const i = inqIx(UcStringFlash(pUcSource));
   if (i == -1) {
      return UnicodeString::Nil;
   }
   return pRep->aUcs[i];
}

/*--------------------------------------------------ArgListSimple::operator==-+
| Equality.  The order is irrelevant.                                         |
+----------------------------------------------------------------------------*/
bool ArgListSimple::operator==(ArgListSimple const & source) const
{
   if (pRep == source.pRep) return true;

   unsigned int iCount = count();
   if (iCount == source.count()) {
      if (iCount) {
         char * check = new char[iCount];
         memset(check, 0, iCount);
         do {
            int i = source.inqIx(pRep->aUcs[--iCount]);
            if ((i == -1) || (check[i] == 1)) {
               ++iCount;  // not found, or already found
               break;
            }
            check[i] = 1;   // checked.
         }while (iCount);
         delete [] check;
      }
      if (!iCount) return true;
   }
   return false;
}

/*-----------------------------------------------------ArgListSimple::setSize-+
| iNewCount must be greater than iOldCount!!!                                 |
+----------------------------------------------------------------------------*/
bool ArgListSimple::setSize(unsigned int iNewCount)
{
   assert (iNewCount > count());
   UnicodeString * aUcsNew = new UnicodeString[iNewCount];  // full of Nils
   if (!aUcsNew) {
      return false;
   }
   unsigned int const iOldCount = count();
   if (!iOldCount) {
      pRep = new ArgListRep;
      if (!pRep) {
         delete aUcsNew;
         return false;
      }
   }else {
      memcpy(aUcsNew, pRep->aUcs, iOldCount * sizeof(UnicodeString));
      memset(pRep->aUcs, 0, iOldCount * sizeof(UnicodeString));
      delete [] pRep->aUcs;
   }
   pRep->aUcs = aUcsNew;
   pRep->iRealCount = iNewCount;
   return true;
}

/*--------------------------------------------------ArgListSimple::operator+=-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool ArgListSimple::operator+=(UnicodeString const & ucsSource)
{
   unsigned int iOldCount = count();
   if (
      (!iOldCount || (iOldCount >= pRep->iRealCount)) &&
      (!setSize(((iOldCount+4)>>2)<<2)) // round (count()+1) to a Xple of 4.
   ) {
      return false;
   }else {
      pRep->aUcs[iOldCount] = ucsSource;
      ++pRep->iArgsCount;
      return true;
   }
}

/*-----------------------------------------------------ArgListWithName::inqIx-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline int ArgListWithName::inqIx(UnicodeString const & ucsName) const
{
   unsigned int const iCount = ArgListSimple::count();
   if (iCount) {
      for (unsigned int i=0; i < iCount; i+=2) {
         if (pRep->aUcs[i] == ucsName) {
            return i/2;
         }
      }
   }
   return -1;
}

/*------------------------------------------------ArgListWithName::operator[]-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString const & ArgListWithName::operator[](
   UnicodeString const & ucsName
) const {
   int const i = inqIx(ucsName);
   if (i == -1) {
      return UnicodeString::Nil;
   }
   return pRep->aUcs[(2*i)+1];
}

/*------------------------------------------------ArgListWithName::operator+=-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool ArgListWithName::operator+=(NamedArg const & source)
{
   if (
      ArgListSimple::operator+=(source.inqName()) &&
      ArgListSimple::operator+=(source.inqValue())
   ) {
      return true;
   }else {
      return false;
   }
}

/*------------------------------------------------ArgListWithName::operator==-+
| Equality.  The order is irrelevant.                                         |
+----------------------------------------------------------------------------*/
bool ArgListWithName::operator==(ArgListWithName const & source) const
{
   if (pRep == source.pRep) return true;

   unsigned int iCount = ArgListSimple::count();
   if (iCount == source.ArgListSimple::count()) {
      if (iCount) {
         iCount /= 2;
         char * check = new char[iCount];
         memset(check, 0, iCount);
         do {
            int i = source.inqIx(pRep->aUcs[2 * --iCount]);   // name
            if (
               (i == -1) || (check[i] == 1) ||  // -1, or checked
               (source.pRep->aUcs[(2*i)+1] != pRep->aUcs[(2*iCount)+1])
            ) {
               ++iCount;      // -1, or odd, or checked, or value mismatch
               break;
            }
            check[i] = 1;     // checked.
         }while (iCount);
         delete [] check;
      }
      if (!iCount) return true;
   }
   return false;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
