/* $Id: Tokenizer.h,v 1.57 2002-08-01 07:04:13 pgr Exp $ */

#ifndef COM_JAXO_YAXX_TOKENIZER_H_INCLUDED
#define COM_JAXO_YAXX_TOKENIZER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "../reslib/resources.h"
#include "../toolslib/uccompos.h"
#include "Constants.h"
#include "RexxString.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*---------------------------------------------------------- class Tokenizer -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Tokenizer {
public:
   enum ReservedKeywords {
      RSVD_NONE,
      RSVD_WITHIN_DO,
      RSVD_WITHIN_IF,
      RSVD_WITHIN_PARSE
   };
   Tokenizer(streambuf & in, UnicodeComposer & erh);
   Token next(ReservedKeywords rsvd);

   bool isAbbutal() const;
   bool isAssigned() const;
   RexxString const getLastTerm() const;
   streampos getLastPos() const;
   bool termEquals(KeywordId id) const;
   int termCompares(KeywordId id) const;
   int getLineNo() const;

   static RexxString const stringizeOp(Token tk);
   static bool isSymbol(char c);

protected:
   UnicodeComposer & m_erh;

private:
   static KeywordId const keywordsInDo[];
   static KeywordId const keywordsInIf[];
   static KeywordId const keywordsInParse[];
   static struct { int count; KeywordId * ids; } const kywdlist[];
   bool m_isBlankFound;
   bool m_isCommentFound;
   bool m_isBlankBefore;
   streambuf & m_in;
   char m_tokenBuf[500];
   char const * const m_pMaxBuf;
   int m_lenToken;
   int m_cur;               // current character
   int m_lineno;            // current line no
   streampos m_startPos;    // start of current token

   int nextChar();
   Token getSymbol();
   Token getQuoted();
};

/* -- INLINES -- */

inline bool Tokenizer::isAbbutal() const {
   return !m_isBlankBefore;
}

inline bool Tokenizer::isAssigned() const {
   return (m_cur == '=');  // && (m_in.sgetc() != '=');  >>>PGR: not in XJ18
}

inline int Tokenizer::getLineNo() const {
   return m_lineno;
}

inline RexxString const Tokenizer::getLastTerm() const {
   assert (m_lenToken >= 0);
   return RexxString(m_tokenBuf, m_lenToken);
}

inline streampos Tokenizer::getLastPos() const {
   return m_startPos;
}

inline bool Tokenizer::termEquals(KeywordId id) const {
   char const * psz = getKeyword(id);
   char const * pch = m_tokenBuf;
   int len = m_lenToken;
   for (;;) {
      if (--len < 0) return *psz == 0;
      if (!*psz || (*pch++ != *psz++)) return false;
   }
}

inline int Tokenizer::termCompares(KeywordId id) const
{
   char const * psz = getKeyword(id);
   char const * pch = m_tokenBuf;
   int len = m_lenToken;
   for (;;) {
      if (--len < 0) return (*psz)? -1 : 0;
      if (!*psz) return 1;
      if (*pch != *psz) return(*pch - *psz);
      ++pch, ++psz;
   }
}
#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
