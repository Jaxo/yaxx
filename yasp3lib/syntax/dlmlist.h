/*
* $Id: dlmlist.h,v 1.6 2011-07-29 10:26:40 pgr Exp $
*
* A delimiter list contains all delimiters used by the current concrete
* syntax.  The constructor always builds a reference concrete syntax.
* Variant syntaxes are obtained by making changes on top of the reference.
* The delimiter list is also used to build the delimiter finder.
*/

#if !defined DLMLIST_HEADER && defined __cplusplus
#define DLMLIST_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "../../toolslib/ucstring.h"
#include "delim.h"

class Charset;
// class ostream;

/*------------------------------------------------------------------DelimList-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API DelimList {               // dlmlst
   friend ostream & operator<<(ostream & out, DelimList const & dlmlst);
public:
   DelimList(Charset const& chsArg);
   ~DelimList();

   static const UnicodeString & rcsValue(Delimiter::e_Ix ixdl);
   UnicodeString const & operator[](Delimiter::e_Ix ixdl) const;
   unsigned int count() const;
   unsigned int countShortref() const;

   bool replace(Delimiter::e_Ix ixdl, UnicodeString const & ucsValue);
   bool suppress_shortrefs();
   bool add_shortref(UnicodeString const & ucsValue);

   Charset const & charset() const;

   operator void *() const;               // is ok?
   bool operator!() const;             // is not ok?

private:
   Charset const & chs;                   // Remember charset used for building
   unsigned int iCountShortref;
   unsigned int iCountDelim;
   UnicodeString * aUcsValue;

   void clear();
   bool replace_double_delim(int const iDlmcode);
   bool reallocate(int iCount);
   DelimList(DelimList const& from);               // No copy constructor
   DelimList& operator=(DelimList const& from);    // No assignment operator
};

/*-- INLINES --*/
inline UnicodeString const & DelimList::operator[](Delimiter::e_Ix ixdl) const {
   if ((unsigned int)ixdl < iCountDelim) {
      return aUcsValue[ixdl];
   }else {
      return UnicodeString::Nil;
   }
}
inline unsigned int DelimList::count() const {
   return iCountDelim;
}
inline unsigned int DelimList::countShortref() const {
   return iCountShortref;
}
inline Charset const & DelimList::charset() const {
   return chs;
}
inline DelimList::operator void *() const {           // cast
   if (iCountDelim) return (void *)this; else return 0;
}
inline bool DelimList::operator!() const {
   if (iCountDelim) return false; else return true;
}
#endif /* DLMLIST_HEADER =====================================================*/
