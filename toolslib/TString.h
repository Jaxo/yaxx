/*
* $Id: TString.h,v 1.5 2002-12-19 06:46:43 pgr Exp $
*
* (C) Copyright Jaxo Systems, Inc. 2002
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 12/15/2002
*
*/

#ifndef COM_JAXO_TOOLS_TSTRING_H_INCLUDED
#define COM_JAXO_TOOLS_TSTRING_H_INCLUDED

/*---------+
| Includes |
+---------*/
class streambuf;
class ostream;

#include "toolsgendef.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

template <class T> class TOOLS_API TSubstring;
template <class T> class TOOLS_API TStringFlash;

/*------------------------------------------------------------ class TString -+
| Class Declaration                                                           |
|                                                                             |
| The TString class is a safe class, handling itself "bad" states.            |
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
class TOOLS_API TString {                     // bs
   friend class TSubstring<T>;
   friend class TStringFlash<T>;
public:
   TString();                                 // Null constructor

   /*
   | Constructors from T and T's, lengthed or sentinel'ed
   */
   TString(T b);
   TString(T const * pSource);
   TString(T const * pSource, int iLen);

   /*
   | Constructors from streams.
   | The constructor from a MemOutStream "empties" the stream
   | and save its contents into the constructed TString.
   */
   //PGR: FIXME! (create a TString constructor from a stream)
// TString(MemOutStream & umost);      // get contents and empty

   /*
   | Miscellaneous constructors
   */
   TString(int i, int iRadix = 10);           // from an int
   TString(unsigned int i, int iRadix = 10);  // from an unsigned int
   TString(long i, int iRadix = 10);          // from a long
   TString(unsigned long i, int iRadix = 10); // from an unsigned long

   /*
   |  copy constructor, deep copy, and assignment operator
   */
   TString(TString<T> const & bsSource);           // copy constructor
   TString copy() const;                             // deep copy
   TString & operator=(TString<T> const & bsSource);  // asgnt

   ~TString();

   /*
   | Conversion to TSubstrings, T * (zero ended)
   */
   TSubstring<T> operator()(int iPos, int iLen) const;  // bs(3, 5) fast!
   operator T const *() const;

   /*
   | Catenations.
   */
   TString operator+(TString<T> const & bsCatenated) const;
   TString operator+(T const * pCatenated) const;

   /*
   | Safe copy to a buffer which has at most 'max' characters.
   | Note:
   |    bs.ncopy(dest, count+1) is like strncpy(dest, this, count)
   |    except that it doesn't pad the destination string with null
   |    characters.
   */
   T * ncopy(T * buf, int max) const;

   /*
   | write to a stream buffer
   */
   bool write(streambuf * bp, int iLenMax = -1) const;

   /*
   | Miscellaneous constant inquiries of specifics
   */
   bool isVoid() const;                          // is empty?
   int length() const;
   unsigned int hashValue() const;

   /*
   |  Translations.
   */
   // int decimal() const;                      // get decimal value
   void lower();                                // change self to lower
   void upper();                                // change self to upper

   TString toLower() const;                      // return a lower version
   TString toUpper() const;                      // return a upper version

   /*
   | Test for identity and equality
   | Identity means: same origin (and implies equality)
   | Equality means: same contents
   */
   bool isIdentical(TString<T> const & source) const;
   int compare(TString<T> const & bsSource) const;
   bool operator==(TString<T> const & bsSource) const;
   bool operator!=(TString<T> const & bsSource) const;
   bool operator>(TString<T> const & bsSource) const;
   bool operator<(TString<T> const & bsSource) const;
   bool operator>=(TString<T> const & bsSource) const;
   bool operator<=(TString<T> const & bsSource) const;

   int compare(T const * pSource) const;   // this - source
   int compare(T const * pSource, int iLen) const;
   static TString getLongestSubsequence(
      TString<T> const str1,
      TString<T> const str2,
      T skipMark = (T)0x80
   );

   // quick, but at your own risks: no checks!
   int quickCompare(T const * pSource) const;
   int quickCompare(T const * pSource, int iLen) const;
   bool operator==(T const * pSource) const;
   bool operator!=(T const * pSource) const;
   bool operator>(T const * pSource) const;
   bool operator<(T const * pSource) const;
   bool operator>=(T const * pSource) const;
   bool operator<=(T const * pSource) const;

   class TOOLS_API Rep {
      friend class TStringFlash<T>;
      friend class TString<T>;
   private:
      Rep & operator=(Rep const& repSource);        // no!
      Rep(Rep const & repSource);                   // no!
      Rep() {};           // enabled for TStringFlash
//    ~Rep();             // disabled for TStringFlash

      T * pTString;
      unsigned short iReferenceCount;
      unsigned short iGiven;
      int iLength;
   };

private:
   Rep *pRep;

   TString(Rep & rep); // for TStringFlash
   TString(TString<T> const & bs, int iPos, int iLen);
   TString(
      TString<T> const & bs1, int iPos1, int iLen1,
      TString<T> const & bs2, int iPos2, int iLen2
   );
   static Rep * alloc(int bSize);
   void makeFromInteger(unsigned long ulVal, int iRadix, bool isNegative);

   void cow();                                       // Copy On Write
   T const * inqPtr() const;
   static int compare(T const *, T const *, int iLen);
   static int compare(T const *, int, T const *, int);
   static int strlength(T const * p);

public:
   TOOLS_API static TString<T> const Nil;
};

template <class T> TString<T>::Rep * repAlloc(int bSize);
template <class T> ostream & operator<<(ostream & out, TString<T> const & bs);

/* -- INLINES -- */
template <class T> inline TString<T>::TString() : pRep(0) {
}
template <class T> inline TString<T>::TString(TString<T>::Rep & rep) : pRep(&rep) {
}
template <class T> inline TString<T>::TString(int iVal, int iRadix) {
   if (iVal < 0) {
      makeFromInteger((unsigned long)(-iVal), iRadix, true);
   }else {
      makeFromInteger((unsigned long)iVal, iRadix, false);
   }
}
template <class T> inline TString<T>::TString(unsigned int uiVal, int iRadix) {
   makeFromInteger((unsigned long)(uiVal), iRadix, false);
}
template <class T> inline TString<T>::TString(long lVal, int iRadix) {
   if (lVal < 0) {
      makeFromInteger((unsigned long)(-lVal), iRadix, true);
   }else {
      makeFromInteger((unsigned long)lVal, iRadix, false);
   }
}
template <class T> inline TString<T>::TString(unsigned long ulVal, int iRadix) {
   makeFromInteger(ulVal, iRadix, false);
}
template <class T> inline bool TString<T>::isVoid() const {
   if (pRep) return false; else return true;
}
template <class T> inline void TString<T>::cow() {
   if (pRep->iReferenceCount > 1) *this = copy();
}
template <class T> inline int TString<T>::length() const {
   if (pRep) return pRep->iLength; else return 0;
}
template <class T> inline T const * TString<T>::inqPtr() const {
   if (pRep) return pRep->pTString; else return 0;
}
template <class T> inline bool TString<T>::isIdentical(
   TString<T> const & source
) const {
   if (pRep == source.pRep) return true; else return false;
}
template <class T> inline bool TString<T>::operator!=(
   TString<T> const & bsSource
) const {
   if (operator==(bsSource)) return false; else return true;
}
template <class T> inline bool TString<T>::operator>(
   TString<T> const & bsSource
) const {
   if (compare(bsSource) > 0) return true; else return false;
}
template <class T> inline bool TString<T>::operator<(
   TString<T> const & bsSource
) const  {
   if (compare(bsSource) < 0) return true; else return false;
}
template <class T> inline bool TString<T>::operator<=(
   TString<T> const & bsSource
) const {
   if (compare(bsSource) <= 0) return true; else return false;
}
template <class T> inline bool TString<T>::operator>=(
   TString<T> const & bsSource
) const {
   if (compare(bsSource) >= 0) return true; else return false;
}
template <class T> inline unsigned int TString<T>::hashValue() const {
   unsigned int i = length();
   if (i) {
      T const * a  = pRep->pTString;
      return i + a[0] + (a[i >> 1]>>4) + a[i -1];
   }else {
      return 0;
   }
}
template <class T> inline TString<T>::operator T const *() const {
   if (pRep) return pRep->pTString; else return 0;
}
template <class T> inline bool TString<T>::operator!=(T const * pSource) const {
   if (operator==(pSource)) return false; else return true;
}
template <class T> inline bool TString<T>::operator>=(T const * pSource) const {
   if (operator<(pSource)) return false; else return true;
}
template <class T> inline bool TString<T>::operator<=(T const * pSource) const {
   if (operator>(pSource)) return false; else return true;
}
template <class T> inline int TString<T>::quickCompare(T const * pSource) const {
   T const * s1 = inqPtr();
   T const * s2 = pSource;
   while (*s1 && (*s1 == *s2)) { ++s1, ++s2; }
   return(*s1 - *s2);
}
template <class T> inline int TString<T>::compare(
   T const * s1, T const * s2, int i2 /* assumed > 0 */
) {
   int c;
   do c = *s1++ - *s2; while (!c && *s2++ && --i2);
   return c;
}
template <class T> inline int TString<T>::quickCompare(
   T const * pSource, int iLen
) const {
   if (iLen) return compare(inqPtr(), pSource, iLen); else return 0;
}


/*--------------------------------------------------------- class TSubstring -+
| TSubstrings  (a very convenient li'll class)                                |
|                                                                             |
| TSubstrings are different from TString because they keep                    |
| their parent TString alive.  They should be use:                            |
| - for scanning                                                              |
| - to redefine sub-fields of a TString                                       |
|                                                                             |
| Note:                                                                       |
|    The conversion from a TString to a TSubstring is easy                    |
|    and fast, but the reverse is not true.                                   |
+----------------------------------------------------------------------------*/
template <class T>
class TOOLS_API TSubstring {           //  bss
public:
   enum e_Direction { AT_FIRST, FROM_LAST };
   enum e_Choice    { ONE_OF, NONE_OF };
   enum { LMAX  = ~(int)0 };

   TSubstring();
   TSubstring(TString<T> const & bs);
   TSubstring(
      TString<T> const & bs,           // What string is it based on?
      int iPos,                        // Relative offset of the subfield
      int iLen=LMAX                    // Subfield length
   );
   TSubstring(
      TSubstring<T> const & bss,       // What substring is it based on?
      int iPos,                        // Relative offset of the subfield
      int iLen=LMAX                    // Subfield length
   );

   // Construct a TSubstring from haystack, starting at needle.
   // aka: find / rfind / iterators.
   TSubstring(                         // "strstr"
      TSubstring<T> const & bssHayStack,
      TString<T> const & bsNeedle,
      e_Direction direction = AT_FIRST
   );
   TSubstring(
      TSubstring<T> const & bssHayStack,
      TSubstring<T> const & bssNeedle,
      e_Direction direction = AT_FIRST
   );
   TSubstring(                         // "strchr"
      TSubstring<T> const & bssHayStack,
      T bNeedle,
      e_Direction direction = AT_FIRST // use: FROM_LAST for strrchr
   );
   TSubstring(                         // "strcspn"
      TSubstring<T> const & bssHayStack,
      e_Choice choice,
      TString<T> const & bsNeedles,
      e_Direction direction = AT_FIRST
   );
   TSubstring(
      TSubstring<T> const & bssHayStack,
      e_Choice choice,
      TSubstring<T> const & bssNeedles,
      e_Direction direction = AT_FIRST
   );
   TSubstring<T> token(                // "strtok"
      TString<T> const & bsNeedles,    // noise surrounding the token
      e_Direction direction = AT_FIRST // use: FROM_LAST for strrtok
   );
   TSubstring<T> token(
      TSubstring<T> const & bssNeedles,
      e_Direction direction = AT_FIRST
   );

   // uses default copy constructor and assignment operator

   operator TString() const;           // also used to check validity
   TSubstring<T> operator()(int iPos, int iLen) const;

   bool isVoid() const;                // is empty?
   T operator[](int i) const;

   int length() const;
   int absolutePosition() const;       // against mother TString


   TString<T> toLower() const;
   TString<T> toUpper() const;

   int compare(TSubstring<T> const & bssSource) const; // this - source

   bool operator==(TSubstring<T> const & bssSrc) const;
   bool operator!=(TSubstring<T> const & bssSrc) const;
   bool operator>(TSubstring<T> const & bssSrc) const;
   bool operator<(TSubstring<T> const & bssSrc) const;
   bool operator>=(TSubstring<T> const & bssSrc) const;
   bool operator<=(TSubstring<T> const & bssSrc) const;

   TString<T> operator+(TSubstring<T> const &) const;

   bool write(streambuf * bp, int iLenMax = -1) const;

private:
   static TString<T> const & checkSanity(
      TString<T> const & bs,
      int & iPos,
      int & iLen
   );
   static TString<T> const & checkSanity(
      TSubstring<T> const & bss,
      int & iPos,
      int & iLen
   );
   void initAt(
      TSubstring<T> const & bssHayStack,
      int iPosRelative
   );
   void initAt(
      TSubstring<T> const & bssHayStack,
      T const * pNeedle, int iLenNeedle, e_Direction direction
   );
   void initAt(
      TSubstring<T> const & bssHayStack, e_Choice,
      T const * pNeedles, int iLenNeedles, e_Direction direction
   );
   TSubstring<T> initAt(
       T const * pNeedles, int iLenNeedles,
       e_Direction direction
   );
   T const * inqPtr() const;

   TString<T> bs;
   int iPos;
   int iLen;

public:
   TOOLS_API static TSubstring<T> const Nil;
};

template <class T> ostream & operator<<(ostream & out, TSubstring<T> const & bss);

/* -- INLINES -- */
template <class T> inline int TSubstring<T>::length() const {
   return iLen;
}
template <class T> inline T const * TSubstring<T>::inqPtr() const {
   return bs.inqPtr() + iPos;   // can be oblivion
}
template <class T> inline TSubstring<T>::TSubstring() : iPos(-1), iLen(0) {
}
template <class T> inline TSubstring<T>::TSubstring(
   TString<T> const & bsArg
) :
   bs(bsArg), iPos(bsArg.isVoid()? -1 : 0), iLen(bsArg.length())
{}
template <class T> inline TSubstring<T>::TSubstring(
   TSubstring<T> const & bssHayStack,
   TString<T> const & bsNeedle,
   e_Direction direction
) {
   initAt(
      bssHayStack, bsNeedle.inqPtr(), bsNeedle.length(), direction
   );
}
template <class T> inline TSubstring<T>::TSubstring(
   TSubstring<T> const & bssHayStack,
   TSubstring<T> const & bssNeedle,
   e_Direction direction
) {
   initAt(
      bssHayStack, bssNeedle.inqPtr(), bssNeedle.length(), direction
   );
}
template <class T> inline TSubstring<T>::TSubstring(
   TSubstring<T> const & bssHayStack,
   e_Choice choice,
   TString<T> const & bsNeedles,
   e_Direction direction
) {
   initAt(
      bssHayStack, choice,
      bsNeedles.inqPtr(), bsNeedles.length(), direction
   );
}
template <class T> inline TSubstring<T>::TSubstring(
   TSubstring<T> const & bssHayStack,
   e_Choice choice,
   TSubstring<T> const & bssNeedles,
   e_Direction direction
) {
   initAt(
      bssHayStack, choice,
      bssNeedles.inqPtr(), bssNeedles.length(), direction
   );
}
template <class T> inline TSubstring<T> TSubstring<T>::token(
   TString<T> const & bsNeedles,
   e_Direction direction
) {
   return initAt(bsNeedles.inqPtr(), bsNeedles.length(), direction);
}
template <class T> inline TSubstring<T> TSubstring<T>::token(
   TSubstring<T> const & bssNeedles,
   e_Direction direction
) {
   return initAt(bssNeedles.inqPtr(), bssNeedles.length(), direction);
}
template <class T> inline TSubstring<T>::operator TString<T>() const {
   if (!iPos && (iLen == bs.length())) {
      return bs;
   }else {
      return TString(bs, iPos, iLen);
   }
}
template <class T> inline TSubstring<T> TSubstring<T>::operator()(
   int iPosArg, int iLenArg
) const {
   return TSubstring(*this, iPosArg, iLenArg);
}
template <class T> inline TSubstring<T> TString<T>::operator()(
  int iPos, int iLen
) const {
   return TSubstring<T>(*this, iPos, iLen);
}
template <class T> inline bool TSubstring<T>::isVoid() const {
   if (iLen) return false; else return true;
}
template <class T> inline T TSubstring<T>::operator[](int i) const {
   if (i >= iLen) return 0;  else return bs[iPos+i];
}
template <class T> inline int TSubstring<T>::absolutePosition() const {
   return iPos;
}
template <class T> inline bool TSubstring<T>::operator!=(
   TSubstring<T> const & bssSrc
) const {
   if (operator==(bssSrc)) return false; else return true;
}
template <class T> inline bool TSubstring<T>::operator>(
   TSubstring<T> const & bssSrc
) const {
   if (compare(bssSrc) > 0) return true; else return false;
}
template <class T> inline bool TSubstring<T>::operator<(
   TSubstring<T> const & bssSrc
) const  {
   if (compare(bssSrc) < 0) return true; else return false;
}
template <class T> inline bool TSubstring<T>::operator<=(
   TSubstring<T> const & bssSrc
) const {
   if (compare(bssSrc) <= 0) return true; else return false;
}
template <class T> inline bool TSubstring<T>::operator>=(
   TSubstring<T> const & bssSrc
) const {
   if (compare(bssSrc) >= 0) return true; else return false;
}

/*---------------------------------------------------------------TStringFlash-+
| Associated to TString<T>::TString(Rep &), the following                     |
| constructors allow the fast construction of TString from                    |
| an array of T *.  This array of Tr must stay alive for                      |
| the duration of the TStringFlash class is has been constructed from.        |
|                                                                             |
| When a TStringFlash get copied, it self transforms to a true                |
| TString -- this behaviour is a kind of Copy On Fetch (the symmetric         |
| of Copy On Write)                                                           |
|                                                                             |
| Note that a TStringFlash has its own Rep included within the class.         |
| The original TStringFlash has the Rep pointer pointing at this Rep.         |
| All subsequent copies will reset the pRep, allocated on the free store.     |
|                                                                             |
| Typical usage:                                                              |
|                                                                             |
| - initialization:                                                           |
|    TStringFlash bsHello("Hello");  // IS-A TString                          |
|                                                                             |
|  - quickly create a TString from a (temporary) T *                          |
|    pass it to a method that requires a TString but doesn't                  |
|    change the string contents                                               |
|    int TString<T>::compare(T const * pSource) const {                       |
|      return compare(TStringFlash(pSource));                                 |
|    }                                                                        |
|                                                                             |
| Another typical usage is for working on large and unique buffers            |
| created when the Application starts and staying alive until the end.        |
|                                                                             |
| The construction of a TStringFlash from a TString is permitted.             |
| However, except in special cases, this just is a waste of about 12 bytes,   |
| because the construction will create a regular pRep.                        |
|                                                                             |
| The assignment from a TString is not permitted, because it would            |
| cut the link with the TStringFlash buffer, the target now using the         |
| buffer of the source TString.  While there is no real danger doing          |
| this, it could lead to an apparent misfunction of the TStringFlash.         |
+----------------------------------------------------------------------------*/
template <class T>
class TOOLS_API TStringFlash : public TString<T> {
public:
   TStringFlash(T const *);
   TStringFlash(T const *, int iLen);
   TStringFlash(TString<T> const &);
   // copy and assignt are inherited

   // these methods are deprecated
   void resetLength(int iLen) { rep.iLength = iLen; }
   void resetBuffer(T * p);

private:
   TStringFlash & operator=(TString<T> const &);  // no!
   TString<T>::Rep rep;
};

#ifdef TOOLS_NAMESPACE
}
#endif
#endif // COM_JAXO_TOOLS_TSTRING_H_INCLUDED

#ifdef JAXO_INSTANCIATE_TEMPLATE
#include "TString.cpp"
#endif

/*===========================================================================*/
