/* $Id: CharConverter.cpp,v 1.11 2002-10-25 06:33:46 pgr Exp $ */

#include <string.h>
#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif
#include "CharConverter.h"
#include "RexxString.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

CharConverter const CharConverter::Upper;

/*----------------------------------------------------CharConverter::operator-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CharConverter::operator()(
   char * pTgt,
   char const * pSrc,
   int len
) const {
   char const * pEnd = pSrc + len;
   while (pSrc < pEnd) {
      char c = *pSrc++;
      if ((c >= 'a') && (c <= 'z')) c -= ('a' - 'A');
      *(pTgt++) = c;
//    *(pTgt++) = toupper(*(unsigned char *)pSrc++);
   }
}

/*---------------------------------DynamicCharConverter::DynamicCharConverter-+
| Build a cross table such that translated = m_x[translatee];                 |
+----------------------------------------------------------------------------*/
DynamicCharConverter::DynamicCharConverter(
   char pad, RexxString const & tableo
) {
   int len = tableo.length();
   if (len > sizeof m_table) len = sizeof m_table;
   memcpy(m_table, tableo, len);
   memset(m_table+len, pad, sizeof m_table - len);
}

/*---------------------------------DynamicCharConverter::DynamicCharConverter-+
| Build a cross table such that translated = table[translatee];               |
+----------------------------------------------------------------------------*/
DynamicCharConverter::DynamicCharConverter(
   RexxString const & tablei, char pad
) {
   for (int i=0; i < sizeof m_table; ++i) {
      m_table[i] = i;    // aka identity, which is the default
   }
   unsigned char const * e_p = (unsigned char const *)(char const *)tablei;
   unsigned char const * c_p = e_p + tablei.length();
   while (c_p > e_p) {
      m_table[*(--c_p)] = pad;
   }
}

/*---------------------------------DynamicCharConverter::DynamicCharConverter-+
| Build a cross table such that translated = table[translatee];               |
+----------------------------------------------------------------------------*/
DynamicCharConverter::DynamicCharConverter(
   RexxString const & tablei,
   char pad,
   RexxString const & tableo
) {
   for (int i=0; i < sizeof m_table; ++i) {
      m_table[i] = i;    // aka identity, which is the default
   }
   // because of the duplicates rule, do it reverse
   unsigned char const * pIn = (unsigned char const *)((char const *)tablei);
   unsigned char const * pOut = (unsigned char const *)((char const *)tableo);
   unsigned char const * pOutCur = pOut + tableo.length();
   unsigned char const * pInCur = pIn + tablei.length();
   unsigned char const * pInPeg = pIn + tableo.length();
   while (pInCur > pInPeg) {
      m_table[*(--pInCur)] = pad;
   }
   while (pInCur > pIn) {
      m_table[*(--pInCur)] = *(--pOutCur);
   }
}

/*-------------------------------------------DynamicCharConverter::operator()-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DynamicCharConverter::operator()(
   char * pTgt,
   char const * pSrc,
   int len
) const {
   char const * pEnd = pSrc + len;
   while (pSrc < pEnd) {
      *pTgt++ = m_table[(unsigned char)(*pSrc++)];
   }
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
