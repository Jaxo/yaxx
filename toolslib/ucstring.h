/*
* $Id: ucstring.h,v 1.14 2011-07-29 10:26:38 pgr Exp $
*/

#ifndef COM_JAXO_TOOLS_UCSTRING_H_INCLUDED
#define COM_JAXO_TOOLS_UCSTRING_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <stdlib.h>
#include <iostream>
#include "toolsgendef.h"
#include "migstream.h"
#include "ucformat.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

typedef unsigned short UCS_2;
class UnicodeMemWriter;
class UnicodeWriter;
class Encoder;
class MultibyteString;
class UnicodeSubstring;

// This class has extensions to help for migration:
// If you have migrated to UnicodeString, try:
// #define UNICODESTRING_EXTENDED_FOR_MIGRATION 0
// before to include this file (CC only, not in your H file!)

#ifndef UNICODESTRING_EXTENDED_FOR_MIGRATION
#define UNICODESTRING_EXTENDED_FOR_MIGRATION 1
#endif

/*--------------------------------------------------------------UnicodeString-+
| Class Declaration                                                           |
|                                                                             |
| The UnicodeString class is a safe class, handling itself "bad" states.      |
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeString {                               // ucs
   friend class UnicodeSubstring;
   friend class UcStringFlash;
public:
   UnicodeString();                                 // Null constructor

   /*
   | Constructors for ASCII < 128 codes.
   | They are safe to use at initialization time, as in:
   | static const UnicodeString("Hello");
   | This is their main purpose: reference "usual, unlocalized" constants.
   */
   UnicodeString(char c);                           // from latin char
   UnicodeString(char const * pszSource);           // from latin string
   UnicodeString(char const * pchSource, int iLen); // ...same,  lengthed

   /*
   | Constructor from encoded array of bytes.  Prefer:
   | UnicodeString(MultibyteString const & mbsSource);
   | (defined as a conv op in the MultibyteString class).
   */
   UnicodeString(                                   // encoded array of bytes
      char const * pchSource,                       // lengthed
      Encoder const & ecm,
      int iLen
   );
   UnicodeString(                                   // encoded array of bytes
      char const * pszSource,                       // zero terminated
      Encoder const & ecm
   );

   /*
   | Constructors from UCS_2 characters.
   */
   UnicodeString(UCS_2 uc);                         // from UCS_2 char
   UnicodeString(UCS_2 const * pUcSource, int iLen);

   /*
   | Constructors from streams.
   | The constructor from a UnicodeMemWriter "empties" the stream
   | and save its contents into the constructed UnicodeString.
   */
   UnicodeString(UnicodeMemWriter & umost);         // get contents and empty

   /*
   | Miscellaneous constructors
   */
   UnicodeString(int i, int iRadix = 10);           // from an int
   UnicodeString(unsigned int i, int iRadix = 10);  // from an unsigned int
   UnicodeString(long i, int iRadix = 10);          // from a long
   UnicodeString(unsigned long i, int iRadix = 10); // from an unsigned long

   /*
   |  copy constructor, deep copy, and assignment operator
   */
   UnicodeString(UnicodeString const & ucsSource);  // copy constructor
   UnicodeString copy() const;                      // deep copy
   UnicodeString & operator=(UnicodeString const & ucsSource);  // asgnt

   ~UnicodeString();

   /*
   | Bridges: conversion to MultibyteString, to UnicodeSubstrings,
   |          retrieval of each character (operator[])
   */
   MultibyteString operator()(Encoder const & ecm) const;
   UnicodeSubstring operator()(int iPos, int iLen) const;  // ucs(3, 5) fast!
   #if !UNICODESTRING_EXTENDED_FOR_MIGRATION
   // ambiguous while migrating (use the conversion to a UCS_2 *)
   UCS_2 operator[](int i) const;               // get value at i
   #endif // UNICODESTRING_EXTENDED_FOR_MIGRATION
   bool write(streambuf * bp, int iLenMax = -1) const;
   bool write(streambuf * bp, int iLenMax, Encoder const & enc) const;

   /*
   | Miscellaneous constant inquiries of specifics
   */
   bool good() const;                            // is ok (non empty)?
   bool empty() const;                           // is empty?
   bool operator!() const;                       // is empty?
   int length() const;
   unsigned int inqHashValue() const;

   /*
   |  Translations.
   */
   int decimal() const;                             // get decimal value
   void lower();                                    // change self to lower
   void upper();                                    // change self to upper

   UnicodeString toLower() const;                   // return a lower version
   UnicodeString toUpper() const;                   // return a upper version

   /*
   |  this catenator should be used sparingly: it's faster to:
   | "UnicodeMemWriter temp; temp << ...; UnicodeString result(temp);"
   */
   UnicodeString operator+(UnicodeString const & ucsCatenated) const;

   /*
   | Test for identity and equality
   | Identity means: same origin (and implies equality)
   | Equality means: same contents
   */
   bool isIdentical(UnicodeString const & source) const;
   int compare(UnicodeString const & ucsSource) const;
   bool operator==(UnicodeString const & ucsSource) const;
   bool operator!=(UnicodeString const & ucsSource) const;
   bool operator>(UnicodeString const & ucsSource) const;
   bool operator<(UnicodeString const & ucsSource) const;
   bool operator>=(UnicodeString const & ucsSource) const;
   bool operator<=(UnicodeString const & ucsSource) const;


   #if UNICODESTRING_EXTENDED_FOR_MIGRATION
  /**---------------------------------------------------------------------**
   **            ALL THE FOLLOWING METHODS ARE DEPRECATED                 **
   **                                                                     **
   ** They are here to help the migration to UnicodeStrings.              **
   **---------------------------------------------------------------------**/
                                                                        //**
   /*                                                                   //**
   | The constructor from a "zero terminated array of UCS_2 *" is       //**
   | deprecated for 2 reasons:                                          //**
   | 1. 0x00 IS a Unicode code point.                                   //**
   |    (sentinels in Unicode would better be 0xFFFE)                   //**
   | 2. It's likely coming from an already constructed UnicodeString,   //**
   |    making therefore useless allocations.                           //**
   */                                                                   //**
   UnicodeString(UCS_2 const * pUcSource);                              //**
                                                                        //**
   /*                                                                   //**
   | Be carefull using the following constructors!                      //**
   | They likely are the ones that will vanish first.                   //**
   | `own' is an optional extra-argument to constructors from           //**
   | existing C strings.                                                //**
   | - when set to `UnicodeString::Given', it means that the caller     //**
   |   owns the string on the free-store, and transfers his ownership   //**
   |   to the UnicodeString class.                                      //**
   | - when set to `UnicodeString::Constant', it means that the caller  //**
   |   wants the  string to be used asis, and no modification can be    //**
   |   done to it.  The caller guarantees the validity of the string    //**
   |   during the lifetime of the UnicodeString object.                 //**
   */                                                                   //**
   enum e_Given    { Given };                                           //**
   enum e_Constant { Constant };                                        //**
   UnicodeString(UCS_2 * pUcSource, e_Given own);                       //**
   UnicodeString(UCS_2 const * pUcSource, e_Constant own);              //**
                                                                        //**
   /*                                                                   //**
   |                                                                    //**
   | Safe copy to a buffer which has at most 'max' characters.          //**
   | Note:                                                              //**
   |    ucs.ncopy(dest, count+1) is like strncpy(dest, this, count)     //**
   |    except that it doesn't pad the destination string with null     //**
   |    characters.                                                     //**
   */                                                                   //**
   UCS_2 * ncopy(UCS_2 * buf, int max) const;                           //**
                                                                        //**
   /*                                                                   //**
   | Other methods                                                      //**
   */                                                                   //**
   UnicodeString operator+(UCS_2 const * pUcCatenated) const;           //**
   int compare(UCS_2 const * pUcSource) const;   // this - source       //**
   int compare(UCS_2 const * pUcSource, int iLen) const;                //**
   // quick, but at your own risks: no checks!                          //**
   int quickCompare(UCS_2 const * pUcSource) const;                     //**
   int quickCompare(UCS_2 const * pUcSource, int iLen) const;           //**
   bool operator==(UCS_2 const * pUcSource) const;                      //**
   bool operator!=(UCS_2 const * pUcSource) const;                      //**
   bool operator>(UCS_2 const * pUcSource) const;                       //**
   bool operator<(UCS_2 const * pUcSource) const;                       //**
   bool operator>=(UCS_2 const * pUcSource) const;                      //**
   bool operator<=(UCS_2 const * pUcSource) const;                      //**
   #if defined(_MSC_VER)                                                //**
   // needed by MSVC and ambiguous for all other compilers MSVC bug)    //**
   // MSVC doesn't quite understand conversion ops and arg matching     //**
   operator void *() const {                                            //**
      if (pRep) return (void *)(pRep->pUcString); else return 0;        //**
   }                                                                    //**
   #endif                                                               //**
   // conversion to zero ended (also: isOk?)                            //**
   operator UCS_2 const *() const {                                     //**
      if (pRep) return pRep->pUcString; else return 0;                  //**
   }                                                                    //**
                                                                        //**
   #endif // UNICODESTRING_EXTENDED_FOR_MIGRATION  -----------------------**

   class Rep {
      friend class UcStringFlash;           // - MIGRATION ONLY
      friend class UnicodeString;
   private:
      static Rep * alloc(int ucSize);
      Rep & operator=(Rep const& repSource);        // no!
      Rep(Rep const & repSource);                   // no!
      Rep() {};           // enabled for UcStringFlash - MIGRATION ONLY
//    ~Rep();             // disabled for UcStringFlash - MIGRATION ONLY

      UCS_2 * pUcString;
      unsigned short iReferenceCount;
      unsigned short iGiven;
      int iLength;
   };

private:
   Rep *pRep;

   #if UNICODESTRING_EXTENDED_FOR_MIGRATION
   static int compare(UCS_2 const *, UCS_2 const *, int iLen);
   UnicodeString(UCS_2 const* pUcSource, e_Given own);             // no!
   UnicodeString(UCS_2 const* pUcSource, int iLen, e_Given own);   // no!
   #endif

   UnicodeString(Rep & rep); // for UcStringFlash - MIGRATION ONLY
   UnicodeString(UnicodeString const & ucs, int iPos, int iLen);
   UnicodeString(
      UnicodeString const & ucs1, int iPos1, int iLen1,
      UnicodeString const & ucs2, int iPos2, int iLen2
   );
   void makeFromInteger(unsigned long ulVal, int iRadix, bool isNegative);

   void cow();                                       // Copy On Write
   UCS_2 const * inqUcPtr() const;
   static int compare(UCS_2 const *, int, UCS_2 const *, int);

public:
   static UnicodeString const Nil; // should be here: CSET2 bug
};

TOOLS_API ostream & operator<<(ostream & out, UnicodeString const & ucs);
TOOLS_API ostream & operator<<(ostream& out, UCS_2 const *pUc);
TOOLS_API int strlength(UCS_2 const * pUc);

/* -- INLINES -- */
inline UnicodeString::UnicodeString() : pRep(0) {
}
inline UnicodeString::UnicodeString(UnicodeString::Rep & rep) : pRep(&rep) {
}
inline UnicodeString::UnicodeString(int iVal, int iRadix) {
   if (iVal < 0) {
      makeFromInteger((unsigned long)(-iVal), iRadix, true);
   }else {
      makeFromInteger((unsigned long)iVal, iRadix, false);
   }
}
inline UnicodeString::UnicodeString(unsigned int uiVal, int iRadix) {
   makeFromInteger((unsigned long)(uiVal), iRadix, false);
}
inline UnicodeString::UnicodeString(long lVal, int iRadix) {
   if (lVal < 0) {
      makeFromInteger((unsigned long)(-lVal), iRadix, true);
   }else {
      makeFromInteger((unsigned long)lVal, iRadix, false);
   }
}
inline UnicodeString::UnicodeString(unsigned long ulVal, int iRadix) {
   makeFromInteger(ulVal, iRadix, false);
}
inline bool UnicodeString::good() const {
   if (pRep) return true; else return false;
}
inline bool UnicodeString::empty() const {
   if (pRep) return false; else return true;
}
inline bool UnicodeString::operator!() const {
   if (pRep) return false; else return true;
}
#if !UNICODESTRING_EXTENDED_FOR_MIGRATION
inline UCS_2 UnicodeString::operator[](int i) const {
   if (!pRep || (i >= pRep->iLength)) {
      return 0;  // or 0xFFFF sentinel ?
   }else {
      return (pRep->pUcString)[i];
   }
}
#endif
inline void UnicodeString::cow() {
   if (pRep->iReferenceCount > 1) *this = copy();
}
inline int UnicodeString::length() const {
   if (pRep) return pRep->iLength; else return 0;
}
inline UCS_2 const * UnicodeString::inqUcPtr() const {
   if (pRep) return pRep->pUcString; else return 0;
}
inline bool UnicodeString::isIdentical(
   UnicodeString const & source
) const {
   if (pRep == source.pRep) return true; else return false;
}
inline bool UnicodeString::operator!=(
   UnicodeString const & ucsSource
) const {
   if (operator==(ucsSource)) return false; else return true;
}
inline bool UnicodeString::operator>(
   UnicodeString const & ucsSource
) const {
   if (compare(ucsSource) > 0) return true; else return false;
}
inline bool UnicodeString::operator<(
   UnicodeString const & ucsSource
) const  {
   if (compare(ucsSource) < 0) return true; else return false;
}
inline bool UnicodeString::operator<=(
   UnicodeString const & ucsSource
) const {
   if (compare(ucsSource) <= 0) return true; else return false;
}
inline bool UnicodeString::operator>=(
   UnicodeString const & ucsSource
) const {
   if (compare(ucsSource) >= 0) return true; else return false;
}
inline unsigned int UnicodeString::inqHashValue() const {
   unsigned int i = length();
   if (i) {
      UCS_2 const * aUc  = pRep->pUcString;
      return i + aUc[0] + (aUc[i >> 1]>>4) + aUc[i -1];
   }else {
      return 0;
   }
}

/*-----------------------------------------------------------UnicodeSubstring-+
| Substrings  (a very convenient li'll class)                                 |
|                                                                             |
| Substrings are different from String because they keep                      |
| their parent String alive.  They should be use:                             |
| - for scanning                                                              |
| - to redefine sub-fields of a String                                        |
|                                                                             |
| Note:                                                                       |
|    The conversion from a UnicodeString to a UnicodeSubstring is easy        |
|    and fast, but the reverse is not true.                                   |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeSubstring {     // UCSS: to access a lengthed subfield
public:
   enum e_Direction { AT_FIRST, FROM_LAST };
   enum e_Choice    { ONE_OF, NONE_OF };
   enum { LMAX  = ~(int)0 };

   UnicodeSubstring();
   UnicodeSubstring(UnicodeString const & ucs);
   UnicodeSubstring(
      UnicodeString const & ucs,       // What string is it based on?
      int iPos,                        // Relative offset of the subfield
      int iLen=LMAX                    // Subfield length
   );
   UnicodeSubstring(
      UnicodeSubstring const & ucss,   // What substring is it based on?
      int iPos,                        // Relative offset of the subfield
      int iLen=LMAX                    // Subfield length
   );

   // Construct a UnicodeSubstring from haystack, starting at needle.
   // aka: find / rfind / iterators.
   UnicodeSubstring(                     // "strstr"
      UnicodeSubstring const & ucssHayStack,
      UnicodeString const & ucsNeedle,
      e_Direction direction = AT_FIRST
   );
   UnicodeSubstring(
      UnicodeSubstring const & ucssHayStack,
      UnicodeSubstring const & ucssNeedle,
      e_Direction direction = AT_FIRST
   );
   UnicodeSubstring(                     // "strchr"
      UnicodeSubstring const & ucssHayStack,
      UCS_2 ucNeedle,
      e_Direction direction = AT_FIRST   // use: FROM_LAST for strrchr
   );
   UnicodeSubstring(                     // "strcspn"
      UnicodeSubstring const & ucssHayStack,
      e_Choice choice,
      UnicodeString const & ucsNeedles,
      e_Direction direction = AT_FIRST
   );
   UnicodeSubstring(
      UnicodeSubstring const & ucssHayStack,
      e_Choice choice,
      UnicodeSubstring const & ucssNeedles,
      e_Direction direction = AT_FIRST
   );
   UnicodeSubstring token(               // "strysp"
      UnicodeString const & ucsNeedles,  // noise surrounding the token
      e_Direction direction = AT_FIRST   // use: FROM_LAST for strrysp
   );
   UnicodeSubstring token(
      UnicodeSubstring const & ucssNeedles,
      e_Direction direction = AT_FIRST
   );

   // uses default copy constructor and assignment operator

   operator UnicodeString() const;     // also used to check validity
   UnicodeSubstring operator()(int iPos, int iLen) const;

   bool good() const;               // is ok (non empty)?
   bool empty() const;              // is empty?
   bool operator!() const;          // is not ok?
   UCS_2 operator[](int i) const;

   int length() const;
   int absolutePosition() const;       // against mother UnicodeString


   UnicodeString toLower() const;
   UnicodeString toUpper() const;

   int compare(UnicodeSubstring const & ucssSource) const; // this - source

   bool operator==(UnicodeSubstring const & ucssSrc) const;
   bool operator!=(UnicodeSubstring const & ucssSrc) const;
   bool operator>(UnicodeSubstring const & ucssSrc) const;
   bool operator<(UnicodeSubstring const & ucssSrc) const;
   bool operator>=(UnicodeSubstring const & ucssSrc) const;
   bool operator<=(UnicodeSubstring const & ucssSrc) const;

   // this catenator should be used sparingly: it's faster to:
   // "UnicodeMemWriter temp; temp << ...; UnicodeString result(temp);"
   UnicodeString operator+(UnicodeSubstring const &) const;

   bool write(streambuf * bp, int iLenMax = -1) const;
   bool write(streambuf * bp, int iLenMax, Encoder const & enc) const;

private:
   static UnicodeString const & checkSanity(
      UnicodeString const & ucs,
      int & iPos,
      int & iLen
   );
   static UnicodeString const & checkSanity(
      UnicodeSubstring const & ucss,
      int & iPos,
      int & iLen
   );
   void initAt(
      UnicodeSubstring const & ucssHayStack,
      int iPosRelative
   );
   void initAt(
      UnicodeSubstring const & ucssHayStack,
      UCS_2 const * pUcNeedle, int iLenNeedle, e_Direction direction
   );
   void initAt(
      UnicodeSubstring const & ucssHayStack, e_Choice,
      UCS_2 const * pUcNeedles, int iLenNeedles, e_Direction direction
   );
   UnicodeSubstring initAt(
       UCS_2 const * pUcNeedles, int iLenNeedles,
       e_Direction direction
   );
   UCS_2 const * inqUcPtr() const;

   UnicodeString ucs;
   int iPos;
   int iLen;

public:
   static UnicodeSubstring const Nil; // should be here: CSET2 bug
};

ostream & operator<<(ostream & out, UnicodeSubstring const & ucss);

/* -- INLINES -- */
inline int UnicodeSubstring::length() const {
   return iLen;
}
inline UCS_2 const * UnicodeSubstring::inqUcPtr() const {
   return ucs.inqUcPtr() + iPos;   // can be oblivion
}
inline UnicodeSubstring::UnicodeSubstring() : iPos(-1), iLen(0) {
}
inline UnicodeSubstring::UnicodeSubstring(
   UnicodeString const & ucsArg
) :
   ucs(ucsArg), iPos(ucsArg.good()? 0 : -1), iLen(ucsArg.length())
{}
inline UnicodeSubstring::UnicodeSubstring(
   UnicodeSubstring const & ucssHayStack,
   UnicodeString const & ucsNeedle,
   e_Direction direction
) {
   initAt(
      ucssHayStack, ucsNeedle.inqUcPtr(), ucsNeedle.length(), direction
   );
}
inline UnicodeSubstring::UnicodeSubstring(
   UnicodeSubstring const & ucssHayStack,
   UnicodeSubstring const & ucssNeedle,
   e_Direction direction
) {
   initAt(
      ucssHayStack, ucssNeedle.inqUcPtr(), ucssNeedle.length(), direction
   );
}
inline UnicodeSubstring::UnicodeSubstring(
   UnicodeSubstring const & ucssHayStack,
   e_Choice choice,
   UnicodeString const & ucsNeedles,
   e_Direction direction
) {
   initAt(
      ucssHayStack, choice,
      ucsNeedles.inqUcPtr(), ucsNeedles.length(), direction
   );
}
inline UnicodeSubstring::UnicodeSubstring(
   UnicodeSubstring const & ucssHayStack,
   e_Choice choice,
   UnicodeSubstring const & ucssNeedles,
   e_Direction direction
) {
   initAt(
      ucssHayStack, choice,
      ucssNeedles.inqUcPtr(), ucssNeedles.length(), direction
   );
}
inline UnicodeSubstring UnicodeSubstring::token(
   UnicodeString const & ucsNeedles,
   e_Direction direction
) {
   return initAt(ucsNeedles.inqUcPtr(), ucsNeedles.length(), direction);
}
inline UnicodeSubstring UnicodeSubstring::token(
   UnicodeSubstring const & ucssNeedles,
   e_Direction direction
) {
   return initAt(ucssNeedles.inqUcPtr(), ucssNeedles.length(), direction);
}
inline UnicodeSubstring::operator UnicodeString() const {
   if (!iPos && (iLen == ucs.length())) {
      return ucs;
   }else {
      return UnicodeString(ucs, iPos, iLen);
   }
}
inline UnicodeSubstring UnicodeSubstring::operator()(
   int iPosArg, int iLenArg
) const {
   return UnicodeSubstring(*this, iPosArg, iLenArg);
}
inline UnicodeSubstring UnicodeString::operator()(
  int iPos, int iLen
) const {
   return UnicodeSubstring(*this, iPos, iLen);
}
inline bool UnicodeSubstring::good() const {
   if (iLen) return true; else return false;
}
inline bool UnicodeSubstring::empty() const {
   if (iLen) return false; else return true;
}
inline bool UnicodeSubstring::operator!() const {
   if (iLen) return false; else return true;
}
inline UCS_2 UnicodeSubstring::operator[](int i) const {
   if (i >= iLen) return 0;  else return ucs[iPos+i];
}
inline int UnicodeSubstring::absolutePosition() const {
   return iPos;
}
inline bool UnicodeSubstring::operator!=(
   UnicodeSubstring const & ucssSrc
) const {
   if (operator==(ucssSrc)) return false; else return true;
}
inline bool UnicodeSubstring::operator>(
   UnicodeSubstring const & ucssSrc
) const {
   if (compare(ucssSrc) > 0) return true; else return false;
}
inline bool UnicodeSubstring::operator<(
   UnicodeSubstring const & ucssSrc
) const  {
   if (compare(ucssSrc) < 0) return true; else return false;
}
inline bool UnicodeSubstring::operator<=(
   UnicodeSubstring const & ucssSrc
) const {
   if (compare(ucssSrc) <= 0) return true; else return false;
}
inline bool UnicodeSubstring::operator>=(
   UnicodeSubstring const & ucssSrc
) const {
   if (compare(ucssSrc) >= 0) return true; else return false;
}

/*-----------------------------------------------------UnicodeStringExtractor-+
| Allows to extract values from UnicodeStrings as if they were streams.       |
|                                                                             |
| Ex:                                                                         |
|    UnicodeStringExtractor extract(UnicodeString("123  foo"));               |
|    int i;                                                                   |
|    UCS_2 name[8];                                                           |
|    extract >> i >> setw(8) >> name; // i is 123, name is "foo"              |
+----------------------------------------------------------------------------*/
class UnicodeStringExtractor : public UnicodeExtractor {
public:
   UnicodeStringExtractor(UnicodeString const & ucsArg) :
      ucs(ucsArg), skipped(0)
   {}
   int inqSkipped() const { return skipped; }
private:
   int peek(int offset) {
      int peeked = ucs[offset + skipped];
      if (!peeked) peeked = -1;  // EOF_SIGNAL;
      return peeked;
   }
   int skip(int iToSkip) {
      skipped += iToSkip;
      return iToSkip;
   }
   UnicodeString const ucs;
   int skipped;
};

inline int UnicodeString::decimal() const {
   // Do not collapse the 2 next lines!
   // decosf does not understand operator inheritance and
   // needs an explicit reference to the base class.
   UnicodeStringExtractor ucsx(*this);
   UnicodeExtractor & ucx = ucsx;
   int i;
   ucx >> i;
   return i;
}


#if UNICODESTRING_EXTENDED_FOR_MIGRATION

/**-----------------------------------------------------------------------**
 **                   ALL THE FOLLOWING IS DEPRECATED                     **
 **                                                                       **
 **  Do not pay attention to the rest of this file,                       **
 **  This would just be a waste of time.                                  **
 **-----------------------------------------------------------------------**/


inline bool UnicodeString::operator!=(UCS_2 const * pUcSource) const {
   if (operator==(pUcSource)) return false; else return true;
}
inline bool UnicodeString::operator>=(UCS_2 const * pUcSource) const {
   if (operator<(pUcSource)) return false; else return true;
}
inline bool UnicodeString::operator<=(UCS_2 const * pUcSource) const {
   if (operator>(pUcSource)) return false; else return true;
}
inline int UnicodeString::quickCompare(UCS_2 const * pUcSource) const {
   UCS_2 const * s1 = inqUcPtr();
   UCS_2 const * s2 = pUcSource;
   while (*s1 && (*s1 == *s2)) { ++s1, ++s2; }
   return(*s1 - *s2);
}
inline int UnicodeString::compare(
   UCS_2 const * s1, UCS_2 const * s2, int i2 /* assumed > 0 */
) {
   int c;
   do c = *s1++ - *s2; while (!c && *s2++ && --i2);
   return c;
}
inline int UnicodeString::quickCompare(
   UCS_2 const * pUcSource, int iLen
) const {
   if (iLen) return compare(inqUcPtr(), pUcSource, iLen); else return 0;
}

/*--------------------------------------------------------------UcStringFlash-+
| ** MIGRATION ONLY **                                                        |
| Associated to UnicodeString::UnicodeString(Rep &), the following            |
| constructors allow the fast construction of UnicodeString from              |
| an array of UCS_2 *.                                                        |
|                                                                             |
| UnicodeStringFlash are safer than UnicodeString::e_Constant, because        |
| of their Copy On Read mechanism -- deep copy when copied / assigned.        |
|                                                                             |
| A typical usage is to pass a const UnicodeString to a passive method        |
| as the ones that just examine the string contents.                          |
| If a UnicodeStringFlash get copied, it self transforms to a true            |
| UnicodeString -- this behaviour is a kind of Copy On Read.                  |
|                                                                             |
| Another typical usage is for working on large and unique buffers            |
| created when the Application starts and staying alive until the end.        |
|                                                                             |
| Note that a UnicodeStringFlash has its own Rep included within the class.   |
| The original UnicodeStringFlash has the Rep pointer pointing at this Rep.   |
| All subsequent copies will reset the pRep, allocated on the free store.     |
|                                                                             |
| The construction of a UnicodeStringFlash from a UnicodeString is permitted. |
| However, except in special cases, this just is a waste of about 12 bytes,   |
| because the construction will create a regular pRep.                        |
|                                                                             |
| The internal buffer of UnicodeStringFlash is available, and can be changed. |
| This eases a lot the migration from UCS_2 * array, but makes it             |
| as unprotected as a UCS_2 * C array.                                        |
| The assignment from a UnicodeString is not permitted, because it would      |
| cut the link with the UnicodeStringFlash buffer, the target now using the   |
| buffer of the source UnicodeString.  While there is no real danger doing    |
| this, it could lead to an apparent misfunction of the Flash.                |
+----------------------------------------------------------------------------*/
class TOOLS_API UcStringFlash : public UnicodeString {
public:
   UcStringFlash(UCS_2 const *);
   UcStringFlash(UCS_2 const *, int iLen);
   UcStringFlash(UnicodeString const &);
   // copy and assignt are inherited
   void resetLength(int iLen) { pRep->iLength = iLen; }
   void resetBuffer(UCS_2 * pUc);
private:
   UcStringFlash & operator=(UnicodeString const &);  // no!
   char buf[sizeof(UnicodeString::Rep)];
};

/*--------------------------------------------------------------UnicodeBuffer-+
| This is a very simple and efficient class to feed an "endless" buffer       |
| with Unicode characters.                                                    |
| This class should stay very simple: if more complex operations are          |
| required, the UnicodeMemWriter class should be used.                        |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeBuffer {
public:
   enum { SIZE_INCR = 256 };
   UnicodeBuffer(int iSize = SIZE_INCR);
   ~UnicodeBuffer();

   void operator<<(UCS_2 uc);

   UCS_2 const * getCurrentToken();
   void endCurrentToken(UCS_2 ucSep);
   int lengthCurrentToken() const;

   bool isFirstChar() const;
   bool isFirstToken() const;
   int length() const;
   int count() const;
   UnicodeString getResult() const;
   operator void *() const;          // isOk?
   bool operator!() const;        // is not ok?

private:
   bool makeRoom();
   UnicodeBuffer& operator=(UnicodeBuffer const& source); // no!
   UnicodeBuffer(UnicodeBuffer const& source);            // no!

   UCS_2 * pUcBuf;               // the buffer
   UCS_2 * pUcCurr;              // current pointer in buffer
   UCS_2 * pUcToken;             // pointer to current token in buffer
   int iLenBuf;                      // current size of the buffer
   int iLen;                         // will get the exact length
   int iCount;                       // How many tokens have we found?
};

/* -- INLINES -- */
inline UnicodeBuffer::~UnicodeBuffer() {
   free(pUcBuf);
}
inline void UnicodeBuffer::operator<<(UCS_2 uc) {
   if ((++iLen < iLenBuf) || makeRoom()) {
      *pUcCurr++ = uc;
   }
}
inline UCS_2 const * UnicodeBuffer::getCurrentToken() {
   if (pUcBuf) {
      *pUcCurr = 0;
      return pUcToken;
   }else {
      return 0;
   }
}
inline void UnicodeBuffer::endCurrentToken(UCS_2 ucSep)
{
   if ((++iLen < iLenBuf) || makeRoom()) {
      ++iCount;
      *pUcCurr++ = ucSep;
      pUcToken = pUcCurr;
   }
}
inline int UnicodeBuffer::lengthCurrentToken() const {
   if (pUcBuf) {
      return pUcCurr - pUcToken;
   }else {
      return 0;
   }
}
inline bool UnicodeBuffer::isFirstChar() const {
   if (pUcBuf && (pUcCurr == pUcToken)) {
      return true;
   }else {
      return false;
   }
}
inline bool UnicodeBuffer::isFirstToken() const {
   if (pUcBuf && (pUcCurr == pUcToken)) {
      return true;
   }else {
      return false;
   }
}
inline int UnicodeBuffer::length() const {
   return iLen;
}
inline int UnicodeBuffer::count() const {
   if (pUcCurr > pUcToken) {
      return iCount+1;
   }else {
      return iCount;
   }
}
inline UnicodeString UnicodeBuffer::getResult() const {
   if (!iLen) {
      return UnicodeString("");         // empty is not null!
   }else {
      // Note: following cast to disambiguate constructor on silly SGI compiler.
      return UnicodeString((const UCS_2 *)pUcBuf, iLen);
   }
}
inline UnicodeBuffer::operator void *() const {
   return (void *)pUcBuf;
}
inline bool UnicodeBuffer::operator!() const {
   if (pUcBuf) return false; else return true;
}

#endif // UNICODESTRING_EXTENDED_FOR_MIGRATION

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
