/*
* $Id: ByteString.cpp,v 1.6 2011-07-29 10:26:35 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <string.h>
#include <assert.h>
#include <new>
#include "ByteString.h"

#ifdef __MWERKS_NEEDS_MEMCMP_FIX__
#define memcmp memcmpFix4PalmOS30
#include "../../palmiolib/memcmp.h"
#endif

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

ByteString const ByteString::NilByte((void ***)0);  // *not* refcounted

/*-------------------------------------------------------ByteString::Rep::Rep-+
|                                                                             |
+----------------------------------------------------------------------------*/
#if defined _WIN32
#pragma warning(disable:4355)
#endif
ByteString::Rep::Rep(NewParam & p) :
   m_pch(((Byte *)this) + p.m_fixedSize), m_len(p.m_bufSize)
{
   m_pch[m_len] = '\0';
   iReferenceCount = 1;
}
#if defined _WIN32
#pragma warning(default:4355)
#endif

/*-------------------------------------------------------ByteString::Rep::Rep-+
| Nil constructor                                                             |
+----------------------------------------------------------------------------*/
ByteString::Rep::Rep() : m_pch(0), m_len(0) {
   iReferenceCount = 0-1;   // to recognize it while debugging
}

/*----------------------------------------------ByteString::Rep::operator new-+
|                                                                             |
+----------------------------------------------------------------------------*/
void * ByteString::Rep::operator new(size_t sz, NewParam * p) {
   if (!p->m_fixedSize) {
      return p->m_staticMem;
   }else {
      assert (sz == p->m_fixedSize);
      return ::new char[sz + ((1 + p->m_bufSize) * sizeof (Byte))];
   }
}

/*-----------------------------------------------------ByteString::ByteString-+
| Private "Nil constructor"                                                   |
+----------------------------------------------------------------------------*/
#if defined _WIN32
#pragma warning(disable:4291)     // no matching delete
#endif
ByteString::ByteString(void ***) {
   static char nilByteRep[sizeof (Rep)];
   Rep::NewParam p(nilByteRep);
   m_pRep = new(&p) Rep();
}
#if defined _WIN32
#pragma warning(default:4291)
#endif

/*-----------------------------------------------------ByteString::ByteString-+
|                                                                             |
+----------------------------------------------------------------------------*/
ByteString::ByteString() {
   m_pRep = ByteString::NilByte.m_pRep;
}

/*----------------------------------------------------ByteString::~ByteString-+
|                                                                             |
+----------------------------------------------------------------------------*/
ByteString::~ByteString()
{
   if ((m_pRep->m_pch) && (!--m_pRep->iReferenceCount)) {
      m_pRep->~Rep();
      delete [] (char *) m_pRep;
   }
}

/*-----------------------------------------------------------ByteString::hash-+
|                                                                             |
+----------------------------------------------------------------------------*/
unsigned int ByteString::hash() const
{
   unsigned int i = m_pRep->m_len;
   if (i) {
      char const * a  = m_pRep->m_pch;
      return i + a[0] + (a[i >> 1]>>4) + a[i -1];
   }else {
      return 0;
   }
}

/*------------------------------------------------ByteString::Rep::skipSpaces-+
|                                                                             |
+----------------------------------------------------------------------------*/
int ByteString::Rep::skipSpaces(int i) const {
   while ((i < m_len) && isSpace(m_pch[i])) ++i;
   return i;
}

/*--------------------------------------------------ByteString::Rep::skipWord-+
|                                                                             |
+----------------------------------------------------------------------------*/
int ByteString::Rep::skipWord(int i) const {
   while ((i < m_len) && !isSpace(m_pch[i])) ++i;
   return i;
}

/*-------------------------------------------------ByteString::Rep::wordIndex-+
|                                                                             |
+----------------------------------------------------------------------------*/
int ByteString::Rep::wordIndex(int wordNo) const
{
   assert (wordNo > 0);
   char const *pEnd = m_pch + m_len;
   char const *pCur = m_pch-1;
   for (;;) {
       if (++pCur >= pEnd) return -1;
       if (!isSpace(*pCur)) {
          if (--wordNo == 0) break;
          for (;;) {
             if (++pCur >= pEnd) return -1;
             if (isSpace(*pCur)) break;
          }
       }
   }
   return pCur-m_pch;
}

/*---------------------------------------------------ByteString::Rep::wordPos-+
|                                                                             |
+----------------------------------------------------------------------------*/
int ByteString::Rep::wordPos(Rep const * phrase, int wordNo) const
{
   if (wordNo == -1) wordNo = 1;
   char const * pBegSource = m_pch + wordIndex(wordNo);
   char const * pEndSource = m_pch + m_len;
   char const * pBegPhrase = phrase->m_pch;
   char const * pEndPhrase = pBegPhrase + phrase->m_len;
   if ((pBegSource < m_pch) || (pEndPhrase == pBegPhrase)) return 0;

   // strip leading and trailing spaces, so a space found means a word follows
   while (isSpace(*--pEndSource)) {}
   while (isSpace(*--pEndPhrase)) if (pEndPhrase == pBegPhrase) return 0;
   while (isSpace(*pBegPhrase)) ++pBegPhrase;

   // now, do the quest.
   for (;;) {
      if (*pBegSource == *pBegPhrase) {  // quick check
         char const * pPhrase = pBegPhrase;
         char const * pSource = pBegSource;
         for (;;) {                      // get a match on all words
            if (++pPhrase > pEndPhrase) {
               if ((++pSource > pEndSource) || isSpace(*pSource)) {
                  return wordNo;         // success!
               }else {
                  break;
               }
            }else if ((++pSource > pEndSource) || (*pPhrase != *pSource)) {
               break;
            }else {
               if (isSpace(*pPhrase)) {  // we *know* a word follows
                  while (isSpace(*++pPhrase)) {}
                  while (isSpace(*++pSource)) {}
                  if (*pPhrase != *pSource) break;
               }
            }
         }
      }
      for (;;) {                         // skip this word
          if (++pBegSource > pEndSource) return 0;
          if (isSpace(*pBegSource)) {    // we *know* a word follows
             ++wordNo;
             while (isSpace(*++pBegSource)) {}
             break;
          }
      }
   }
}

/*------------------------------------------------ByteString::Rep::wordsCount-+
|                                                                             |
+----------------------------------------------------------------------------*/
int ByteString::Rep::wordsCount() const
{
   int count = 0;
   char const * const d_p = m_pch + m_len;
   char const * p = m_pch;
   while (p < d_p) {
      if (isSpace(*p)) {
         while ((++p < d_p) && isSpace(*p)) {}
      }else {
         ++count;
         while ((++p < d_p) && !isSpace(*p)) {}
      }
   }
   return count;
}

/*-------------------------------------------------------ByteString::Rep::pos-+
|                                                                             |
+----------------------------------------------------------------------------*/
int ByteString::Rep::pos(Rep const * needle, int pos) const
{
   int lenNeedle = needle->m_len;
   if (lenNeedle && (pos <= m_len)) {
      char const * const pNeedle = needle->m_pch;
      char const * pCur;
      char const * const pEnd = m_pch + m_len - lenNeedle;
      if (pos == -1) {
         pCur = m_pch;
      }else {
         pCur = m_pch + pos - 1;   // Byte -> C index
      }
      while (pCur <= pEnd) {
         if (0 == memcmp(pCur++, pNeedle, lenNeedle)) {
            return pCur - m_pch;
         }
      }
   }
   return 0;
}

/*---------------------------------------------------ByteString::Rep::lastpos-+
|                                                                             |
+----------------------------------------------------------------------------*/
int ByteString::Rep::lastpos(Rep const * needle, int pos) const
{
   int lenNeedle = needle->m_len;
   if (lenNeedle) {
      char const * const pNeedle = needle->m_pch;
      char const * pCur;
      if ((pos == -1) || (pos > m_len)) {
         pCur = m_pch + m_len - lenNeedle;
      }else {
         pCur = m_pch + pos - lenNeedle;
      }
      for (; pCur >= m_pch; --pCur) {
         if (0 == memcmp(pCur, pNeedle, lenNeedle)) {
            return pCur - m_pch + 1;   // C -> Byte index
         }
      }
   }
   return 0;
}

/*---------------------------------------------------ByteString::Rep::compare-+
| Compares two strings and returns the position of the first non matching     |
| character [1,largest length]                                                |
| Returns 0 if strings are equal                                              |
+----------------------------------------------------------------------------*/
int ByteString::Rep::compare(ByteString::Rep const * against, char pad) const
{
   int i;
   Rep const * a;
   Rep const * b;
   if (m_len < against->m_len) {
      a = this;
      b = against;
   }else {
      b = this;
      a = against;
   }
   for (i=0; i < a->m_len; ++i) {
      if (a->m_pch[i] != b->m_pch[i]) return i+1;
   }
   while (i < b->m_len) {
      if (pad != b->m_pch[i]) return i+1;
      ++i;
   }
   return 0;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
