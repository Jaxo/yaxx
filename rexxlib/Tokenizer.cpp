/* $Id: Tokenizer.cpp,v 1.81 2002-09-05 02:29:14 jlatone Exp $ */

#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif

#include "Tokenizer.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

KeywordId const Tokenizer::keywordsInDo[] = {
   _KWD__TO,
   _KWD__BY,
   _KWD__FOR,
   _KWD__UNTIL,
   _KWD__WHILE
};
KeywordId const Tokenizer::keywordsInIf[] = { _KWD__THEN };
KeywordId const Tokenizer::keywordsInParse[] = { _KWD__WITH };

/*-------------------------------------------------------Tokenizer::Tokenizer-+
|                                                                             |
+----------------------------------------------------------------------------*/
Tokenizer::Tokenizer(
   streambuf & in,
   UnicodeComposer & erh
) :
   m_erh(erh),
   m_startPos(0),
   m_in(in),
   m_lineno(1),
   m_pMaxBuf(m_tokenBuf + sizeof m_tokenBuf),
   m_isBlankFound(false),
   m_isCommentFound(false),
   m_cur(';')
{
}

/*------------------------------------------------------------Tokenizer::next-+
|                                                                             |
+----------------------------------------------------------------------------*/
Token Tokenizer::next(ReservedKeywords rsvd)
{
   m_lenToken = -1;
   m_isBlankBefore = m_isBlankFound;
   m_isBlankFound = false;
   m_isCommentFound = false;
   m_startPos = (int)m_in.pubseekoff(0, ios::cur, ios::in) - 1;

   if ((rsvd != RSVD_NONE) && isalpha(m_cur)) {
      int c = m_cur;
      char * pCur = m_tokenBuf;
      for (;;) {
         *pCur++ = (c >= 'a')? c + 'A'-'a' : c;
         c = m_in.sgetc();                     // peek
         if (!isalpha(c)) break;
         c = m_in.sbumpc();                    // ok, read
      }
      m_lenToken = pCur - m_tokenBuf;
      if (!isSymbol(c)) {          // keywords are fully alpha symbols
         int count;
         KeywordId const * ids;
         switch (rsvd) {
         case RSVD_WITHIN_DO:
            count = sizeof keywordsInDo / sizeof keywordsInDo[0];
            ids = keywordsInDo;
            break;
         case RSVD_WITHIN_IF:
            count = sizeof keywordsInIf / sizeof keywordsInIf[0];
            ids = keywordsInIf;
            break;
         case RSVD_WITHIN_PARSE:
            count = sizeof keywordsInParse / sizeof keywordsInParse[0];
            ids = keywordsInParse;
            break;
         default:
            count = 0;
            break;
         }
         for (; count--; ++ids) {
            if (termEquals(*ids)) {
               nextChar();
               return (Token)(*ids + TK_KEYWORD);
            }
         }
      }
      m_cur = m_tokenBuf[--m_lenToken];  // restart at last known alpha
      return getSymbol();
   }

   switch (m_cur) {

   case '\n':
      ++m_lineno;
      /* fall thru */
   case ';':
      nextChar();
      return TK_SEMICOLON;

   case '+':
       nextChar();
       return TK_PLUS;

   case '-':
       nextChar();
       return TK_MINUS;

   case '%':
      nextChar();
      return TK_IDIV;

   case '(':
      nextChar();
      return TK_LEFTPAR;

   case ')':
      nextChar();
      return TK_RIGHTPAR;

   case ',':
      nextChar();
      return TK_COMMA;

   case ':':
      m_erh << ECE__ERROR << _REX__20_0 << endm;
      break;

   case '\'':
   case '\"':
      return getQuoted();

   case '.':
      if (isSymbol(m_in.sgetc())) return getSymbol();
      nextChar();
      return TK_DOT;

   case '|':
      if (nextChar() == '|')  {
         nextChar();
         return TK_CONCAT;
      }
      return TK_OR;

   case '&':
       if (nextChar() == '&')  {
          nextChar();
          return TK_XOR;
       }
       return TK_AND;

   case '/':
      switch (nextChar()) {
      case '/':
         nextChar();
         return TK_MOD;
      case '=':
         m_erh << ECE__ERROR << _REX__35_0 << endm;
         break;
      }
      return TK_DIV;

   case '*':
       switch (nextChar()) {
       case '*':
          nextChar();
          return TK_POWER;
       case '/':
          m_erh << ECE__ERROR << _REX__35_0 << endm;
          break;
       }
       return TK_MUL;

   case '=':
       switch (nextChar())  {
       case '=':
          nextChar();
          return TK_STRICT_EQ;
       case '>':
       case '<':
          m_erh << ECE__ERROR << _REX__35_0 << endm;
          break;
       }
       return TK_NORMAL_EQ;

   case '<':
      switch (nextChar())  {
      case '<':
         if (nextChar() == '=') {
            nextChar();
            return TK_STRICT_LE;
         }
         return TK_STRICT_LT;
      case '=':
         nextChar();
         return TK_NORMAL_LE;
      case '>':
         nextChar();
         return TK_NORMAL_NE;
      }
      return TK_NORMAL_LT;

   case '>':
      switch (nextChar())  {
      case '>':
         if (nextChar() == '=') {
            nextChar();
            return TK_STRICT_GE;
         }
         return TK_STRICT_GT;
      case '=':
         nextChar();
         return TK_NORMAL_GE;
      case '<':
         nextChar();
         return TK_NORMAL_NE;
      }
      return TK_NORMAL_GT;

   case '^':
   case '\\':
        switch (nextChar())  {
        case '=':
           if (nextChar() == '=') {
              nextChar();
              return TK_STRICT_NE;
           }
           return TK_NORMAL_NE;
        case '>':
           if (nextChar() == '>') {
              nextChar();
              return TK_STRICT_LE;
           }
           return TK_NORMAL_LE;
        case '<':
           if (nextChar() == '<') {
              nextChar();
              return TK_STRICT_GE;
           }
           return TK_NORMAL_GE;
        }
        return TK_NOT;

   case EOF:
      m_startPos = 1 + m_startPos;         // b/c EOF is not 'real'
      								// and b/c gcc doesn't like operator++ or operator+ !!!
      return TK_EOF;

   default:
      if (isSymbol(m_cur)) {
         return getSymbol();
      }
      m_erh << ECE__ERROR << _REX__13_0 << endm;
      break;
   }
   assert (false);          // never hit!
   return TK_EOF;
}

/*-------------------------------------------------------Tokenizer::getSymbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
Token Tokenizer::getSymbol()
{
   char * pCur;
   int dotCount;
   bool isExponentFound = false;
   bool isConstant;
   bool isNumber;

   if (m_lenToken > 0) {     // tokenBuf is partially filled with alphabetics
      pCur = m_tokenBuf + m_lenToken;
      dotCount = 0;
      isConstant = isNumber = false;
   }else {
      pCur = m_tokenBuf;
      dotCount = (m_cur == '.')? 1 : 0;
      isConstant = isNumber = dotCount || isdigit(m_cur);
   }
   if (islower(m_cur)) m_cur += 'A' - 'a';
   while (
      *pCur++ = m_cur,
      nextChar(),
      (m_cur == ':') || (       // comments and blanks are OK before ':'
         !m_isBlankFound &&     // otherwise, no blanks are allowed
         (!m_isCommentFound || (m_cur == '(')) // comments? only b4 '('
      )
   ) {
      if (pCur >= m_pMaxBuf) {
         m_erh << ECE__ERROR << _REX__30_1 << sizeof m_tokenBuf << endm;
         return TK_EOF;
      }
      switch (m_cur) {
      case 'E':
      case 'e':
         m_cur = 'E';
         if (isExponentFound) isNumber = false;
         isExponentFound = true;
         continue;

      case '+':
      case '-':
         if (!isNumber || ((*(pCur-1)!='E') && (*(pCur-1)!='e'))) break;
         continue;

      case '.':
         if (isNumber && dotCount) {
            isNumber = false;
         }
         ++dotCount;
         continue;

      case ':':
         nextChar();
         m_lenToken = pCur - m_tokenBuf;
         return TK_LABEL;

      case '(':
         nextChar();
         m_lenToken = pCur - m_tokenBuf;
         return TK_FUNCTION;

      case EOF:
         break;     // end of symbol

      default:
         if (isSymbol(m_cur)) {
            if (!isdigit(m_cur)) {
               isNumber = false;
               if (islower(m_cur)) m_cur += 'A' - 'a';
            }
           continue;
         }
         break;  // end of symbol
      }
      break;     // end of symbol
   }
   m_lenToken = pCur - m_tokenBuf;
   if (isConstant) {
      return isNumber? TK_NUMBER_CONSTANT_SYMBOL : TK_OTHER_CONSTANT_SYMBOL;
   }
   if (dotCount) {
      return ((dotCount==1)&&(*(pCur-1)=='.'))? TK_STEM_SYMBOL : TK_COMPOUND_SYMBOL;
   }
   return TK_SIMPLE_SYMBOL;
}

/*-------------------------------------------------------Tokenizer::getQuoted-+
| Get a literal, hexadecimal or binary string                                 |
+----------------------------------------------------------------------------*/
Token Tokenizer::getQuoted()
{
   char quote = m_cur;
   char *pCur = m_tokenBuf;

   while (
      ((m_cur = m_in.sgetc()) != quote) ||
      (m_in.sbumpc(), (m_cur = m_in.sgetc()) == quote)
   ) {
      m_in.sbumpc();
      switch (m_cur) {
      case EOF:
      case '\n':
         m_erh << ECE__ERROR << (quote=='\''? _REX__6_2 : _REX__6_3) << endm;
         return TK_EOF;
      case '\r':
         break;
//    case '\n':
//       ++m_lineno;
//       *pCur++ = ' ';
//       break;
      default:
         *pCur++ = m_cur;
         break;
      }
      if (pCur >= m_pMaxBuf) {
         m_erh << ECE__ERROR << _REX__30_2 << sizeof m_tokenBuf << endm;
         return TK_EOF;
      }
   }
   nextChar();
   if (!m_isBlankFound && (!m_isCommentFound || m_cur == '(')) {
      switch (m_cur) {
      case '(':
         nextChar();
         m_lenToken = pCur - m_tokenBuf;
         return TK_FUNCTION_LITERAL;

      case 'B': case 'b':
      case 'X': case 'x':
         if (!isSymbol(m_in.sgetc())) { // check next char
            char buffer[sizeof m_tokenBuf / 2];
            char *t_p;
            int length = sizeof buffer;
            if ((m_cur == 'b') || (m_cur == 'B')) {
               t_p = RexxString::binToChar(
                  m_tokenBuf, pCur - m_tokenBuf, buffer, &length, 0
               );
            }else {
               t_p = RexxString::hexToChar(
                  m_tokenBuf, pCur - m_tokenBuf, buffer, &length
               );
            }
            if (!t_p) {
               m_erh << ECE__ERROR << _REX__15_0 << endm;
               return TK_EOF;
            }
            memcpy(m_tokenBuf, t_p, length);
            pCur = m_tokenBuf + length;
            nextChar();
         }
         break;
      }
   }
   m_lenToken = pCur - m_tokenBuf;
   return TK_STRING_CONSTANT_SYMBOL;
}

/*--------------------------------------------------------Tokenizer::nextChar-+
| Process continuations, comments and intermediate blanks and return the      |
| next relevant character.                                                    |
+----------------------------------------------------------------------------*/
// state table for jumping over nested comments
static char const fgOther[] = { 0 , 0 , 2 , 2 };
static char const fgAster[] = { 0 , 2 , 3 , 3 };
static char const fgSlash[] = { 1 , 1 , 1 , 0 };
inline int Tokenizer::nextChar()
{
   streampos markPos = 0;
   for (;;) {
      m_cur = m_in.sbumpc();
      switch (m_cur) {
      #ifdef __MWERKS__
      case 0x00:
         continue;       // forget about  MemoDB bullshit
      #endif
      case ' ':
      case '\t':
         if (!markPos) m_isBlankFound = true;
         continue;
      case '\r':
         continue;
      case '\n':
         if (markPos) {
            markPos = 0;
            m_isBlankFound = true;
            ++m_lineno;
            continue;
         }
         break;
      case ',':
         if (!markPos) {           // start look-ahead for continuation
            markPos = m_in.pubseekoff(0, ios::cur, ios::in);
            assert (markPos > 0);
            continue;
         }
         break;
      case '/':
         if (m_in.sgetc() == '*') {
            int level = 0;
            char st = 2;
            if (!markPos) m_isCommentFound = true;
            m_in.sbumpc();       // skip over '*' in "/*"
            for (;;) {
               switch (m_in.sbumpc()) {
               case EOF:
                  m_erh << ECE__ERROR << _REX__6_1 << endm;
                  st=0;
                  level=0;
                  break;
               case '*':
                  st = fgAster[st];
                  break;
               case '/':
                  if (st == 2) ++level;
                  st = fgSlash[st];
                  break;
               case '\n':
                  ++m_lineno;
                  /* fall thru */
               default:
                  st = fgOther[st];
                  break;
               }
               if (st == 0) {
                  if (!level--) break;
                  st = 2;
               }
            }
            continue;
         }
         break;
      default:
         break;
      }
      if (markPos) {
         if ((int)m_in.pubseekoff(markPos, ios::beg, ios::in) == EOF) {
            m_erh << _REX__30_0 << endm;
         }
         m_cur = ',';
      }
      return m_cur;
   }
}

/*STATIC-----------------------------------------------Tokenizer::stringizeOp-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString const Tokenizer::stringizeOp(Token tk)
{
   char const * psz;
   switch (tk) {
   case TK_AND:       psz = "&";     break;
   case TK_COMMA:     psz = ",";     break;
   case TK_CONCAT:    psz = "||";    break;
   case TK_DIV:       psz = "/";     break;
   case TK_DOT:       psz = ".";     break;
   case TK_EOF:       psz = "(EOF)"; break;
   case TK_IDIV:      psz = "%";     break;
   case TK_LEFTPAR:   psz = "(";     break;
   case TK_MINUS:     psz = "-";     break;
   case TK_MOD:       psz = "//";    break;
   case TK_MUL:       psz = "*";     break;
   case TK_NORMAL_EQ: psz = "=";     break;
   case TK_NORMAL_GE: psz = "\\<";   break;
   case TK_NORMAL_GT: psz = ">";     break;
   case TK_NORMAL_LE: psz = "\\>";   break;
   case TK_NORMAL_LT: psz = "<";     break;
   case TK_NORMAL_NE: psz = "\\=";   break;
   case TK_NOT:       psz = "\\";    break;
   case TK_OR:        psz = "|";     break;
   case TK_PLUS:      psz = "+";     break;
   case TK_POWER:     psz = "**";    break;
   case TK_RIGHTPAR:  psz = ")";     break;
   case TK_SEMICOLON: psz = ";";     break;
   case TK_STRICT_EQ: psz = "==";    break;
   case TK_STRICT_GE: psz = "\\<<";  break;
   case TK_STRICT_GT: psz = ">>";    break;
   case TK_STRICT_LE: psz = "\\>>";  break;
   case TK_STRICT_LT: psz = "<<";    break;
   case TK_STRICT_NE: psz = "\\==";  break;
   case TK_XOR:       psz = "&&";    break;
   default:           psz = "???";   break;
   }
   return RexxString(psz);
}

/*STATIC--------------------------------------------------Tokenizer::isSymbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::isSymbol(char c) {
   return (
      (isalnum(c)) || (c == '!') || (c == '#') || (c == '$') ||
      (c == '.') || (c == '?') || (c == '@') || (c == '_')
   );
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
