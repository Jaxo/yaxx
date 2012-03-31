/*
* $Id: Escaper.cpp,v 1.1 2002-06-23 09:59:21 pgr Exp $
*
* Parse escape sequences
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include <string.h>
#include "Escaper.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------Escaper::Decoder::Decoder-+
| Copy operator                                                               |
+----------------------------------------------------------------------------*/
Escaper::Decoder::Decoder(Decoder const & source) {
   memcpy(this, &source, sizeof source);
}

/*----------------------------------------------------Escaper::Decoder::match-+
|                                                                             |
+----------------------------------------------------------------------------*/
Escaper::Decoder::Answer Escaper::Decoder::match(char const * p, int max) {
   char const * pS = getCode();
   int sMax = strlen(pS);
   int i = (sMax > max)? max : sMax;
   while (i--) if (*p++ != *pS++) return NOT_ME;
   return (max < sMax)? ME_DUNNO : ME_MATCH;
}

/*---------------------------------------------------Escaper::registerDecoder-+
| Registering a decoder must always be done as in Java:                       |
|   Escaper::registerDecoder(new MyDecoder());                                |
+----------------------------------------------------------------------------*/
void Escaper::registerDecoder(Escaper::Decoder * pNewDecoder) {
   m_decoders.add(pNewDecoder);
}

/*-------------------------------------------------------Escaper::findDecoder-+
|                                                                             |
+----------------------------------------------------------------------------*/
Escaper::Decoder::Answer Escaper::findDecoder(char const * p, int len)
{
   LinkedList::Iterator it((LinkedList &)m_decoders); // violates constness
   int max = m_decoders.size();
   for (int ix=0; ix < max; ++ix) {
      Decoder * pDecoder = (Decoder *)it();
      Decoder::Answer answ = pDecoder->match(p, len);
      if (answ != Decoder::NOT_ME) {
         m_decoder = pDecoder;
         return answ;
      }
   }
   return Decoder::NOT_ME;
}

/*-----------------------------------------------------------Escaper::process-+
| Main routine                                                                |
+----------------------------------------------------------------------------*/
void Escaper::process(char const * pBeg, int len)
{
   int k;
   char const * pCur = (char const *)memchr(pBeg, ESCAPE_CHAR, len);
   char const * pEnd = pBeg + len;
   if (m_wait.size >= 0) {
      k = sizeof m_wait.buf - m_wait.size;
      if (pCur) {
         if ((pCur - pBeg) < k) k = pCur - pBeg;
      }else {
         if (len < k) k = len;
      }
      memcpy(m_wait.buf + m_wait.size, pBeg, k);
      switch (findDecoder(m_wait.buf, k+m_wait.size)) {
      case Decoder::NOT_ME:
         output(&m_wait.esc, m_wait.size+1);
         m_wait.size = -1;
         break;
      case Decoder::ME_DUNNO:
         m_wait.size += k;
         return;
      default:          // valid escape sequence
         m_decoder->action();
         k = strlen(m_decoder->getCode()) - m_wait.size;
         assert (k > 0);
         m_wait.size = -1;
         pBeg += k;
         len -= k;
         break;
      }
   }
   for (;;) {
      if (pCur) {
         len = pEnd - ++pCur;
         switch (findDecoder(pCur, len)) {
         case Decoder::NOT_ME:
            break;
         case Decoder::ME_DUNNO:
            assert (len <= sizeof m_wait.buf);
            memcpy(m_wait.buf, pCur, len);
            if (pCur > pBeg) output(pBeg, pCur-pBeg-1);
            m_wait.size = len;
            return;
         default:          // valid escape sequence
            if (pCur > pBeg) output(pBeg, pCur-pBeg-1);
            k = strlen(m_decoder->getCode());
            pCur += k;
            len -= k;
            pBeg = pCur;
            m_decoder->action();
            break;
         }
         pCur = (char const *)memchr(pCur, ESCAPE_CHAR, len);
      }else {
         if (pEnd > pBeg) output(pBeg, pEnd - pBeg);
         return;
      }
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
