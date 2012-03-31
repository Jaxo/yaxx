/*
* $Id: sort.h,v 1.4 2002-04-14 23:26:04 jlatone Exp $
*
* Sort an array of items.
*
* How To Use?
* ----------
*
* 1) enter:
*    MAKE_SortFunction(SortMe, Foo);  // Foo is the class of items to sort
*
* 2) In the definition of the Foo class, add the 2 following functions:
*
*    int compare(Foo const& against) const;
*    void exchange(Foo & against);
*
*    The compare function must return:
*       - 0 if the key in `this' is equal to the one in `against',
*       - Less than 0 if the key if the key in `this' is less
*         than the one in `against',
*       - Greather than 0 if the key if the key in `this' is is greather
*         than the one in `against'.
*    The exchange function must exchange the items "in_place" - as in:
*       Foo temp(*this);
*       *this = against, against = temp;
*
* That is all.  To sort the items, just do:
*     SortMe(theArrayOfFoo, theCount);
*
* If the items to sort are not in a contiguous vanilia array, then
* you might want to use the MAKE_SortFunctionWithAccess macro (see below).
*
* Advanced explainations:
* ----------------------
*
* The SortFunctor class is a pure functor to provide sort methods:
* the one currently implemented is Hoare's Quicksort algorithm.
*
* SortFunctor must be used as a base class of the SortFunction class,
* which define a transaction object.  Example:
*
* class SortFunction : private SortFunctor { // this IS_A Sort functor
* public:
*    SortItemFunction(Item * aItemArg, int iCount) : aItem(aItemArg) {
*       quick(iCount);
*    }
* private:
*    int compare(int ix1, int ix2) const;
*    void exchange(int ix1, int ix2);
*    // where: "ix1" and "ix2" are he indeces of two items
*    // to compare or exchange
*    Item *aItem;  // the array of items to sort;
* };
*
* Because each reference goes thru exchange and compare, the caller keeps
* the entire control on the way items are arranged in the array.
*
* MAKE_SortFunctionWithAccess(SortFunctionClass, TableClass)
* assumes that the access to the item number n is done calling:
* ItemClass & TableClass::operator[](int n);
*/

#ifndef COM_JAXO_TOOLS_SORT_H_INCLUDED
#define COM_JAXO_TOOLS_SORT_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------------------------SortFunctor-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API SortFunctor {
public:
   void quick(int const iCount);     // number of items to sort
protected:
   virtual int compare(int, int) const = 0;
   virtual void exchange(int, int) = 0;
private:
   void hoares_sort(int beg, int end);
};

/* -- Macros (for conveniency) -- */

// generate a specific sort functor "SortFunctionClass" to sort "ItemClass"
// It's assumed that items belong to a contiguous array
// You must define:
// int         ItemClass::compare(ItemClass const& itemAgainst) const;
// void        ItemClass::exchange(ItemClass & itemAgainst);

#define MAKE_SortFunction(SortFunctionClass, ItemClass) \
  class SortFunctionClass : private SortFunctor {\
  public:\
     SortFunctionClass(ItemClass * aIArg, int iCount) : aI(aIArg) {\
        quick(iCount);}\
  private:\
     int compare(int ix1, int ix2) const { return aI[ix1].compare(aI[ix2]);}\
     void exchange(int ix1, int ix2) { aI[ix1].exchange(aI[ix2]);}\
     ItemClass *aI;}

// generate a specific sort functor "SortFunctionClass" to sort items
// when not a simple contiguous array, but ramdom access through the
// operator [] of the TableClass container.
// You must define:
// ItemClass & TableClass::operator[](int n);
// int         ItemClass::compare(ItemClass const& itemAgainst) const;
// void        ItemClass::exchange(ItemClass & itemAgainst);

#define MAKE_SortFunctionWithAccess(SortFunctionClass, TableClass)\
  class SortFunctionClass : private SortFunctor {\
  public:\
     SortFunctionClass(TableClass & aIArg, int iCount) : aI(aIArg) {\
        quick(iCount);}\
  private:\
     int compare(int ix1, int ix2) const { return aI[ix1].compare(aI[ix2]);}\
     void exchange(int ix1, int ix2) { aI[ix1].exchange(aI[ix2]);}\
     TableClass & aI;}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
