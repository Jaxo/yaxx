/*
* $Id: entstrea.cpp,v 1.6 2011-07-29 10:26:39 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <iostream>
#include "entity.h"
#include "entstrea.h"

#if defined ISO_14882 || defined __MWERKS__
#define base() _M_buf
#endif

/*-------------------------------------BracketedStreamBuf::BracketedStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
BracketedStreamBuf::BracketedStreamBuf(
   UCS_2 const * pUcPrefixArg,
   UCS_2 const * pUcBodyArg,
   UCS_2 const * pUcSuffixArg
) :
   MemStreamBuf(
      (char const *)pUcPrefixArg,
      strlength(pUcPrefixArg) * sizeof(UCS_2),
      MemStreamBuf::Constant
   ),
   pUcPrefix(pUcPrefixArg),
   iLenPrefix(strlength(pUcPrefixArg) * sizeof(UCS_2)),
   pUcBody(pUcBodyArg),
   iLenBody(strlength(pUcBodyArg) * sizeof(UCS_2)),
   pUcSuffix(pUcSuffixArg),
   iLenSuffix(strlength(pUcSuffixArg) * sizeof(UCS_2))
{}

/*----------------------------------------------BracketedStreamBuf::underflow-+
|                                                                             |
+----------------------------------------------------------------------------*/
int BracketedStreamBuf::underflow()
{
   /*---
   | Visual C++ has bad manners, like:
   | 1) calling undeflow for no reason (gptr < egptr); or,
   | 2) calling underflow for good, but with gptr() > egptr().
   | The first case is caught by next if.  To avoid the second
   | case, no assumption is made on how much gptr() is > egptr().
   +---*/
   if (gptr() < egptr()) {
      return *gptr();
   }

   if (base() == (char const *)pUcPrefix) {
      init((char const *)pUcBody, iLenBody, MemStreamBuf::Constant);
      return sgetc();
   }else if (base() == (char const *)pUcBody) {
      init((char const *)pUcSuffix, iLenSuffix, MemStreamBuf::Constant);
      return sgetc();
   }
   return EOF;
}

/*-----------------------------------------------BracketedStreamBuf::in_avail-+
|                                                                             |
+----------------------------------------------------------------------------*/
int BracketedStreamBuf::in_avail()
{
   int i = MemStreamBuf::in_avail();
   if (base() != (char const *)pUcSuffix) {
      i += iLenSuffix;
      if (base() != (char const *)pUcBody) {
         i += iLenBody;
      }
   }
   return i;
}

/*------------------------------------------------BracketedStreamBuf::seekoff-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos BracketedStreamBuf::seekoff(
   streamoff so, ios::seekdir dir, ios__openmode om
) {
   switch (om) {
   case ios::in:
      switch (dir) {
      case ios::cur:
         so -= in_avail();
         /* fall thru */
      case ios::end:
         so += pcount();
      default: // case ios::beg:
         break;
      }
      {
         streamoff soReal;
         if (soReal = so, soReal <= iLenPrefix) {
            init((char const *)pUcPrefix, iLenPrefix, MemStreamBuf::Constant);
            gbump((int)soReal);
            return so;
         }else if (soReal -= iLenPrefix, soReal <= iLenBody) {
            init((char const *)pUcBody, iLenBody, MemStreamBuf::Constant);
            gbump((int)soReal);
            return so;
         }else if (soReal -= iLenBody, soReal <= iLenSuffix) {
            init((char const *)pUcSuffix, iLenSuffix, MemStreamBuf::Constant);
            gbump((int)soReal);
            return so;
         }
      }
      break;
   case ios::out:
   default:
      break;
   }
   return EOF;
}

/*------------------------------------------------BracketedStreamBuf::seekpos-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos BracketedStreamBuf::seekpos(streampos sp, ios__openmode om) {
   return seekoff(sp, ios::beg, om);
}

/*---------------------------------------------------------------EntStreamMem-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
EntStreamMem::EntStreamMem(
   UnicodeString const & ucsContent
) :
   istream(&buf),
   buf(
      (char const *)(UCS_2 const *)ucsContent,
      ucsContent.length() * sizeof(UCS_2),
      MemStreamBuf::Constant
   )
{
   if (!buf) {
      clear(rdstate() | ios::failbit);
   }
}

/*------------------------------------------------------EntStreamMemBracketed-+
|                                                                             |
+----------------------------------------------------------------------------*/
EntStreamMemBracketed::EntStreamMemBracketed(
   UnicodeString const & ucsPrefix,
   UnicodeString const & ucsBody,
   UnicodeString const & ucsSuffix
) :
   istream(&buf), buf(ucsPrefix, ucsBody, ucsSuffix)
{
   if (!buf) {
      clear(rdstate() | ios::failbit);
   }
}

/*===========================================================================*/
