/*
* $Id: mbstring.h,v 1.8 2011-07-29 10:26:38 pgr Exp $
*
* Multibyte Strings.
*/

#ifndef COM_JAXO_TOOLS_MBSTRING_H_INCLUDED
#define COM_JAXO_TOOLS_MBSTRING_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <iostream>
#include "toolsgendef.h"
#include "Encoder.h"
#include "ucstring.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------------MultibyteString-+
| Multibyte string                                                            |
+----------------------------------------------------------------------------*/
class TOOLS_API MultibyteString {                   // mbs
public:
   MultibyteString();
   MultibyteString(
      UnicodeString const & ucsSource,
      Encoder const & ecm
   );
   MultibyteString(
      UCS_2 const * pUcSource,
      Encoder const & ecm,
      int iLenSource
   );

   MultibyteString(MultibyteString const & mbsSource); // copy
   ~MultibyteString();

   MultibyteString& operator=(MultibyteString const& mbsSource);

   operator UnicodeString() const;
   bool good() const;                             // is ok (non empty)?
   bool empty() const;                            // is empty?
   bool operator!() const;                        // is empty?

   // will be soon or later deprcated:
   MultibyteString(char const * pszSource, Encoder const & ecm);
   char const * inqString() const;                   // temporary???

   class Rep {                                       // mbsrep
      friend class MultibyteString;
   private:
      static Rep * alloc(int chSize, Encoder const & ecm);
      char * data();
      Rep();                                         // no!
      ~Rep();                                        // no!
      Rep& operator=(Rep const& repSource);          // no!
      Rep(Rep const& repSource);                     // no!

      Encoder ecm;                                   // encoding
      int iReferenceCount;
      int iLength;
   };

private:
   int length() const;
   Rep *pRep;

public:
   static MultibyteString const Nil;  // should be here: CSET2 bug (csd 11)
};

/* -- INLINES -- */
inline char * MultibyteString::Rep::data() {
   return (char *)(this+1);
}
inline MultibyteString::MultibyteString() {
   pRep = 0;
}
inline MultibyteString::operator UnicodeString() const {
   if (pRep) {
      return UnicodeString(pRep->data(), pRep->ecm, pRep->iLength);
   }else {
      return UnicodeString::Nil;
   }
}
inline bool MultibyteString::good() const {
   if (pRep) return true; else return false;
}
inline bool MultibyteString::empty() const {
   if (pRep) return false; else return true;
}
inline bool MultibyteString::operator!() const {
   if (pRep) return false; else return true;
}
inline char const * MultibyteString::inqString() const {
   if (pRep) return pRep->data(); else return 0;
}
inline int MultibyteString::length() const {
   if (pRep) return pRep->iLength; else return 0;
}
inline MultibyteString UnicodeString::operator()(Encoder const & ecm) const {
   if (pRep) {
      return MultibyteString(pRep->pUcString, ecm, pRep->iLength);
   }else {
      return MultibyteString::Nil;
   }
}

/*------------------------------------------------------------CompositeString-+
| The Composite class allows                                                  |
| - to pass a value in any one of Multibyte, Unicode, UCS_2 *, or char *.     |
| - to get returned values go Unicode or MultiByte according to the left hand.|
|                                                                             |
| It is recommened to be used only at the "border line" between the           |
| external world and this code: all code inside should deal with              |
| pure UnicodeString's.                                                       |
|                                                                             |
| Example:                                                                    |
|                                                                             |
|    Bozo::setFoo(CompositeString &);                                         |
|    CompositeString const & Bozo::inqFoo() const;                            |
|                                                                             |
| Can be called:                                                              |
|                                                                             |
|    theBozo.setFoo("Hello");                                                 |
|    theBozo.setFoo(ucsHello);    (ucsHello is a UnicodeString)               |
|    UnicodeString const ucsBar = theBozo.inqFoo();                           |
|    char * pszBar = theBozo.inqFoo().inqString();                            |
|    ...                                                                      |
+----------------------------------------------------------------------------*/
class CompositeString: public MultibyteString, public UnicodeString {
public:
   CompositeString() {}
   CompositeString(MultibyteString const &);
   CompositeString(UnicodeString const &);
   CompositeString(UnicodeString const &, Encoder const &);
   CompositeString(UCS_2 const *);
   CompositeString(char const *);
   CompositeString(char const * p, Encoder const & enc);
   bool good() const;                            // is ok (non empty)?
   bool empty() const;                           // is empty?
   bool operator!() const;                       // is empty?
   int length() const;

   static CompositeString const Nil;
};

/* -- INLINES -- */

inline CompositeString::CompositeString(MultibyteString const & mbs) :
   MultibyteString(mbs), UnicodeString(mbs)
{}
inline CompositeString::CompositeString(UnicodeString const & ucs) :
   MultibyteString(ucs, Encoder()), UnicodeString(ucs)
{}
inline CompositeString::CompositeString(
   UnicodeString const & ucs, Encoder const & enc
) :
   MultibyteString(ucs, enc), UnicodeString(ucs)
{}
inline CompositeString::CompositeString(UCS_2 const * pUc) :
   UnicodeString(pUc)
{
   MultibyteString & mbs = *this;
   mbs = MultibyteString(*this, Encoder());
}
inline CompositeString::CompositeString(char const * p, Encoder const & enc) :
   MultibyteString(p, enc),
   UnicodeString(p, enc)
{}

inline CompositeString::CompositeString(char const * p) :
   UnicodeString(p, Encoder())
{
   MultibyteString & mbs = *this;
   mbs = MultibyteString(*this, Encoder());
}
inline bool CompositeString::good() const {
   return UnicodeString::good();
}
inline bool CompositeString::empty() const {
   return UnicodeString::empty();
}
inline bool CompositeString::operator!() const {
   return UnicodeString::operator!();
}
inline int CompositeString::length() const {
   return UnicodeString::length();
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
