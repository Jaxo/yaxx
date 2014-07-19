/* $Id: DecTest.cpp,v 1.12 2002/10/15 10:53:17 pgr Exp $ */

/*---------+
| Includes |
+---------*/
#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif

#include <new.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <memory.h>

#if defined(_MSC_VER)
#include <fcntl.h>
#include <io.h>
#endif

#include "DecTest.h"
class Tokenizer;
class Context;
class Reader;

/*----------+
| Constants |
+----------*/

static char const * LOCATION        = "In file: %s, line %d: ";
static char const * RUNNINGWHAT     = "Running test file: \"%s\"...\n";
static char const * INTERNALERROR   = "** Internal Error in %s at line %d **\n";
static char const * NOINFILE        = "Cannot access input file: \"%s\".\n";
static char const * INVINFILE       = "Cannot read: \"%s\".  Check permissions.\n";
static char const * BADPRECISION    = "Precision directive should be followed by a positive number.  Found: \"%s\".\n";
static char const * BADMAXEXPONENT  = "MaxExponent directive should be followed by a positive number.  Found: \"%s\".\n";
static char const * BADROUNDINGKWD  = "Rounding directive: \"%s\" is not a proper keyword.\n";
static char const * BADEXTENDED     = "Extended directive should be followed by a boolean.  Found: \"%s\".\n";
static char const * BADID           = "A test identifier was expected.  Found: \"%s\".\n";
static char const * BADOPERATION    = "An operation was expected.  Found: \"%s\".\n";
static char const * BADOPERAND      = "Invalid syntax for operand: \"%s\".\n";
static char const * NORESULT        = "Invalid result syntax.  Must be a non empty string, preceeded by \"-> \".  Found: \"%s\".\n";
static char const * BADCONDITION    = "Invalid condition: \"%s\".\n";
static char const * BADDECTEXT      = "The file name of a test was expected.  Found: \"%s\".\n";

static char const * BADCONTEXT      = "Cannot run the test case.  Bad context.  One or more required directives were not specified.\n";
static char const * TESTINVALID     = "Test case is invalid.  Skipping.\n";
static char const * FAILED          = "Test case \"%s\" has failed:\n";
static char const * INVRESULT       = "\tresult is \"%s\" and \"%s\" was expected.\n";
static char const * INVCOND0        = "\tcondition is: \"";
static char const * INVCOND1        = "\".  \"";
static char const * INVCOND2        = "\" was expected.\n";
static char const * UNKCONDITION    = "(Unknown Condition: %08x)";

static char const sCnvOvFlw[]       = "CONVERSION_OVERFLOW";
static char const sCnvSyntax[]      = "CONVERSION_SYNTAX";
static char const sCnvUnFlw[]       = "CONVERSION_UNDERFLOW";
static char const sDivZero[]        = "DIVISION_BY_ZERO";
static char const sDivImp[]         = "DIVISION_IMPOSSIBLE";
static char const sDivUndef[]       = "DIVISION_UNDEFINED";
static char const sInexact[]        = "INEXACT";
static char const sNoMemory[]       = "INSUFFICIENT_STORAGE";
static char const sInvCtxt[]        = "INVALID_CONTEXT";
static char const sInvOp[]          = "INVALID_OPERATION";
static char const sLostDig[]        = "LOST_DIGITS";
static char const sOvFlw[]          = "OVERFLOW";
static char const sRounded[]        = "ROUNDED";
static char const sUnflw[]          = "UNDERFLOW";

static char const sCeiling[]        = "CEILING";
static char const sDown[]           = "DOWN";
static char const sFloor[]          = "FLOOR";
static char const sHalfDown[]       = "HALF_DOWN";
static char const sHalfEven[]       = "HALF_EVEN";
static char const sHalfUp[]         = "HALF_UP";
static char const sUp[]             = "UP";

static char const sPrecision[]      = "PRECISION";
static char const sRounding[]       = "ROUNDING";
static char const sMaxExponent[]    = "MAXEXPONENT";
static char const sVersion[]        = "VERSION";
static char const sExtended[]       = "EXTENDED";
static char const sDecTest[]        = "DECTEST";

static char const sAbs[]            = "ABS";
static char const sAdd[]            = "ADD";
static char const sCompare[]        = "COMPARE";
static char const sDivide[]         = "DIVIDE";
static char const sDivideint[]      = "DIVIDEINT";
static char const sInteger[]        = "INTEGER";
static char const sMax[]            = "MAX";
static char const sMin[]            = "MIN";
static char const sMinus[]          = "MINUS";
static char const sMultiply[]       = "MULTIPLY";
static char const sPlus[]           = "PLUS";
static char const sPower[]          = "POWER";
static char const sRemainder[]      = "REMAINDER";
static char const sRemaindernear[]  = "REMAINDERNEAR";
static char const sRescale[]        = "RESCALE";
static char const sSubtract[]       = "SUBTRACT";
static char const sToEng[]          = "TOENG";
static char const sToSci[]          = "TOSCI";

enum Token {
   EOFILE,
   EMPTY,
   COMMENTS,
   TESTS,
   PRECISION_DIRECTIVE,
   MAXEXPONENT_DIRECTIVE,
   ROUNDING_DIRECTIVE,
   VERSION_DIRECTIVE,
   EXTENDED_DIRECTIVE,
   DECTEST_DIRECTIVE
};

static struct {
   char const * name;
   int len;
   Context::Condition value;
} const condValues[] = {
   { sCnvOvFlw,    sizeof sCnvOvFlw -1,  Context::COND_CNVOVFLW  },
   { sCnvSyntax,   sizeof sCnvSyntax -1, Context::COND_CNVSYNTAX },
   { sCnvUnFlw,    sizeof sCnvUnFlw -1,  Context::COND_CNVUNFLW  },
   { sDivZero,     sizeof sDivZero -1,   Context::COND_DIVZERO   },
   { sDivImp,      sizeof sDivImp -1,    Context::COND_DIVIMP    },
   { sDivUndef,    sizeof sDivUndef -1,  Context::COND_DIVUNDEF  },
   { sInexact,     sizeof sInexact -1,   Context::COND_INEXACT   },
   { sNoMemory,    sizeof sNoMemory -1,  Context::COND_NOMEMORY  },
   { sInvCtxt,     sizeof sInvCtxt -1,   Context::COND_INVCTXT   },
   { sInvOp,       sizeof sInvOp -1,     Context::COND_INVOP     },
   { sLostDig,     sizeof sLostDig -1,   Context::COND_LOSTDIG   },
   { sOvFlw,       sizeof sOvFlw -1,     Context::COND_OVFLW     },
   { sRounded,     sizeof sRounded -1,   Context::COND_ROUNDED   },
   { sUnflw,       sizeof sUnflw -1,     Context::COND_UNFLW     }
};

static struct {
   char const * name;
   int len;
   Context::Rounding value;
} const roundValues[] = {
   { sCeiling,  sizeof sCeiling - 1,   Context::ROUNDING_CEILING   },
   { sDown,     sizeof sDown - 1,      Context::ROUNDING_DOWN      },
   { sFloor,    sizeof sFloor - 1,     Context::ROUNDING_FLOOR     },
   { sHalfDown, sizeof sHalfDown - 1,  Context::ROUNDING_HALF_DOWN },
   { sHalfEven, sizeof sHalfEven - 1,  Context::ROUNDING_HALF_EVEN },
   { sHalfUp,   sizeof sHalfUp - 1,    Context::ROUNDING_HALF_UP   },
   { sUp,       sizeof sUp - 1,        Context::ROUNDING_UP        }
};

static struct DirctvValue {
   char const * name;
   int len;
   Token value;
} const dirctvValues[] = {
   { sPrecision,   sizeof sPrecision - 1,   PRECISION_DIRECTIVE   },
   { sRounding,    sizeof sRounding - 1,    ROUNDING_DIRECTIVE    },
   { sMaxExponent, sizeof sMaxExponent - 1, MAXEXPONENT_DIRECTIVE },
   { sVersion,     sizeof sVersion - 1,     VERSION_DIRECTIVE     },
   { sExtended,    sizeof sExtended - 1,    EXTENDED_DIRECTIVE    },
   { sDecTest,     sizeof sDecTest - 1,     DECTEST_DIRECTIVE     }
};

static struct OpValue {
   struct Operation {
      int (Test::*pUnaryOp)(char const *, Context const &);
      int (Test::*pBinaryOp)(char const *, char const *, Context const &);
   };
   char const * name;
   int len;
   Operation value;
} const opValues[] = {
   { sAbs,           sizeof sAbs - 1,           &Test::abs,         0   },
   { sAdd,           sizeof sAdd - 1,           0, &Test::add           },
   { sCompare,       sizeof sCompare - 1,       0, &Test::compare       },
   { sDivide,        sizeof sDivide - 1,        0, &Test::divide        },
   { sDivideint,     sizeof sDivideint - 1,     0, &Test::divideInt     },
   { sInteger,       sizeof sInteger - 1,       &Test::integer,     0   },
   { sMax,           sizeof sMax - 1,           0, &Test::max           },
   { sMin,           sizeof sMin - 1,           0, &Test::min           },
   { sMinus,         sizeof sMinus - 1,         &Test::minus,       0   },
   { sMultiply,      sizeof sMultiply - 1,      0, &Test::multiply      },
   { sPlus,          sizeof sPlus - 1,          &Test::plus,        0   },
   { sPower,         sizeof sPower - 1,         0, &Test::power         },
   { sRemainder,     sizeof sRemainder - 1,     0, &Test::remainder     },
   { sRemaindernear, sizeof sRemaindernear - 1, 0, &Test::remainderNear },
   { sRescale,       sizeof sRescale - 1,       0, &Test::rescale       },
   { sSubtract,      sizeof sSubtract - 1,      0, &Test::subtract      },
   { sToEng,         sizeof sToEng - 1,         &Test::toEng,       0   },
   { sToSci,         sizeof sToSci - 1,         &Test::toSci,       0   }
};

/*---------------------------------------------------- class SearchableTable -+
|                                                                             |
+----------------------------------------------------------------------------*/
class SearchableTable {
public:
   int find(char const * name, int len) const;
protected:
   virtual int getSize() const = 0;
   virtual char const * getKeyAt(int ix) const = 0;
   virtual int getKeyLenAt(int ix) const = 0;
};

/*---------------------------------------------------------- class CondTable -+
|                                                                             |
+----------------------------------------------------------------------------*/
static class CondTable : public SearchableTable {
public:
   bool storeValue(char const * name, int & value) const {
      int ix = find(name, strlen(name));
      if (ix >= 0) {
         value |= condValues[ix].value;
         return true;
      }
      return false;
   }
private:
   int getSize() const { return sizeof condValues / sizeof condValues[0]; }
   char const * getKeyAt(int ix) const { return condValues[ix].name; }
   int getKeyLenAt(int ix) const { return condValues[ix].len; }
} const condTable;

/*--------------------------------------------------------- class RoundTable -+
|                                                                             |
+----------------------------------------------------------------------------*/
static class RoundTable : public SearchableTable {
public:
   bool storeValue(char const * name, Context::Rounding & value) const {
      int ix = find(name, strlen(name));
      if (ix >= 0) {
         value = roundValues[ix].value;
         return true;
      }
      return false;
   }
private:
   int getSize() const { return sizeof roundValues / sizeof roundValues[0]; }
   char const * getKeyAt(int ix) const { return roundValues[ix].name; }
   int getKeyLenAt(int ix) const { return roundValues[ix].len; }
} const roundTable;

/*-------------------------------------------------------- class DirctvTable -+
|                                                                             |
+----------------------------------------------------------------------------*/
static class DirctvTable : public SearchableTable {
public:
   Token getValue(char const * name, int len) const {
      int ix = find(name, len);
      return (ix >= 0)? dirctvValues[ix].value : TESTS;
   }
private:
   int getSize() const { return sizeof dirctvValues / sizeof dirctvValues[0]; }
   char const * getKeyAt(int ix) const { return dirctvValues[ix].name; }
   int getKeyLenAt(int ix) const { return dirctvValues[ix].len; }
} const dirctvTable;

/*------------------------------------------------------------ class OpTable -+
|                                                                             |
+----------------------------------------------------------------------------*/
static class OpTable : public SearchableTable {
public:
   bool storeValue(char const * name, int len, OpValue::Operation & value) const {
      int ix = find(name, len);
      if (ix >= 0) {
         value = opValues[ix].value;
         return true;
      }
      return false;
   }
private:
   int getSize() const { return sizeof opValues / sizeof opValues[0]; }
   char const * getKeyAt(int ix) const { return opValues[ix].name; }
   int getKeyLenAt(int ix) const { return opValues[ix].len; }
} const opTable;

/*----------------------------------------------------------- class TestCase -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TestCase {
public:
   TestCase(Tokenizer & tk, Context const & m_ctxt);
   void run(Test & test);
   bool checkResult(char const * result, int conditions);

private:
   char const * m_id;
   OpValue::Operation m_op;
   char const * m_operand1;
   char const * m_operand2;
   char const * m_result;
   int m_conditions;
   bool m_isOk;
   Context const & m_ctxt;
   Reader const & m_rdr;
   static void showConditions(int conditions);
};

/*------------------------------------------------------------- class Buffer -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Buffer {
public:
   Buffer(int length = 16);
   ~Buffer();

   int length() const;
   operator char const *() const;
   void append(char c);
   void deleteCharAt(int index);
   void stripSpaces();
   void empty();

private:
   char * m_value;
   int m_count;
   int m_capacity;

   void expandCapacity(int minimumCapacity);
   Buffer & operator=(Buffer const & source); // no!
   Buffer(Buffer const & source);             // no!
};

/*------------------------------------------------------------- class Reader -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Reader : public Buffer {
private:
   int m_lineNo;
   char const * m_path;
   FILE * m_f;

public:
   Reader(char const * path);
   char const * nextLine();
   void locate(FILE * f = stderr) const;
   ~Reader();
};

/*---------------------------------------------------------- class Tokenizer -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Tokenizer : public Reader {
public:
   Tokenizer(char const * path);
   Token next();
   char const * getStringValue() const;
   bool storePositiveValue(int & value);
   bool storePositiveOrZeroValue(int & value);
   bool storeBooleanValue(bool & b);
   bool storeStringValue(char const *& val);
   bool storeRounding(Context::Rounding & round);
   bool storeOperandValue(char const *& val);
   bool storeOperation(OpValue::Operation & op);
   bool storeResult(char const *& val);
   bool storeConditions(int & conditions);

private:
   char const * m_pCur;

   bool toNextField(char const * pCur);
   bool storeUnquoted(char const *& val);
};

/*-------------------------------------------------------- class ContextImpl -+
|                                                                             |
+----------------------------------------------------------------------------*/
class ContextImpl : public Context {
   public: int m_precision;
   public: int m_maxExponent;
   public: Rounding m_round;
   public: bool m_bExtended;

   public: ContextImpl() {
      m_precision = 0;
      m_maxExponent = -1;
      m_round = ROUNDING_INVALID;
      m_bExtended = false;
   }

   public: int inqPrecision() const     { return m_precision;   }
   public: int inqMaxExponent() const   { return m_maxExponent; }
   public: Rounding inqRounding() const { return m_round;       }
   public: bool isExtended() const      { return m_bExtended;   }

   public: bool isOk() const {
      return (
         (m_precision > 0) &&
         (m_maxExponent >= 0) &&
         (m_round != ROUNDING_INVALID)
      );
   }
};

/*------------------------------------------------------SearchableTable::find-+
|                                                                             |
+----------------------------------------------------------------------------*/
int SearchableTable::find(char const * name, int len) const {
   int const size = getSize();
   for (int i=0; i < size; ++i) {
      if (len == getKeyLenAt(i)) {
         char const * p = getKeyAt(i);
         for (int j=0; ; ++j) {
            if (j == len) return i;
            if (toupper(name[j]) != *p++) break;
         }
      }
   }
   return -1;
}

/*-------------------------------------------------------------Buffer::Buffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
Buffer::Buffer(int length) {
   assert (length >= 0);
   m_value = new char[length];
   m_capacity = length;
   m_count = 0;
}

/*------------------------------------------------------------Buffer::~Buffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
Buffer::~Buffer() {
   delete [] m_value;
}

/*----------------------------------------------Buffer::operator char const *-+
|                                                                             |
+----------------------------------------------------------------------------*/
Buffer::operator char const *() const {
   return m_value;
}

/*-------------------------------------------------------------Buffer::length-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Buffer::length() const {
   return m_count;
}

/*--------------------------------------------------------------Buffer::empty-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Buffer::empty() {
   m_count = 0;
}

/*-------------------------------------------------------------Buffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Buffer::append(char c) {
   if (m_count+1 > m_capacity) {
      expandCapacity(m_count+1);
   }
   m_value[m_count++] = c;
}

/*-------------------------------------------------------Buffer::deleteCharAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Buffer::deleteCharAt(int index) {
   assert ((index >= 0) && (index < m_count));
   memmove(m_value+index, m_value+index+1, m_count-index-1);
   --m_count;
}

/*-----------------------------------------------------Buffer::expandCapacity-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Buffer::expandCapacity(int minimumCapacity) {
   m_capacity = (m_capacity + 1) * 2;
   if (m_capacity < 0) {
      m_capacity = INT_MAX;
   }else if (m_capacity < minimumCapacity) {
      m_capacity = minimumCapacity;
   }
   char * newValue = new char[m_capacity];
   memcpy(newValue, m_value, m_count);
   delete [] m_value;
   m_value = newValue;
}

/*--------------------------------------------------------Buffer::stripSpaces-+
| Remove leading and trailing spaces.                                         |
+----------------------------------------------------------------------------*/
void Buffer::stripSpaces()
{
   char * c_p = m_value;
   while (m_count && (*c_p <= ' ')) { ++c_p, --m_count; }
   memmove(m_value, c_p, m_count);
   c_p = m_value + m_count;
   while (m_count && (*--c_p <= ' ')) { --m_count; }
}

/*-------------------------------------------------------------Reader::Reader-+
|                                                                             |
+----------------------------------------------------------------------------*/
Reader::Reader(char const * path) {
   m_path = path;
   m_lineNo = 0;
   m_f = 0;
   struct stat buf;
   if (stat(m_path, &buf) < 0) {
      fprintf(stderr, NOINFILE, m_path);
   }else if (
      ((buf.st_mode & S_IFDIR) != 0) ||
      ((buf.st_mode & S_IFREG) == 0) ||
      ((buf.st_mode & S_IREAD) == 0)
   ) {
      fprintf(stderr, INVINFILE, m_path);
   }else {
      m_f = fopen(m_path, "rb");
      if (!m_f) {
         fprintf(stderr, NOINFILE, m_path);
      }else {
         fprintf(stderr, RUNNINGWHAT, m_path);
         fflush(stderr);  // so, if something bad happens, I know where I am
      }
   }
}

/*-----------------------------------------------------------Reader::nextLine-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * Reader::nextLine()
{
   int c;
   if (m_f && !feof(m_f)) {
      empty();
      while ((EOF != (c=fgetc(m_f))) && (c!='\r') && (c!='\n')) {
         append(c);
      }
      if ((c!=EOF) || (length() > 0)) {
         if ((c=='\r') && (c = fgetc(m_f), c != '\n')) {
            ungetc(c, m_f);
         }
         ++m_lineNo;
         stripSpaces();
         append('\0');
         return operator char const *();
      }
   }
   return 0;
}

/*-------------------------------------------------------------Reader::locate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Reader::locate(FILE * f) const {
   fprintf(f, LOCATION, m_path, m_lineNo);
}

/*------------------------------------------------------------Reader::~Reader-+
|                                                                             |
+----------------------------------------------------------------------------*/
Reader::~Reader() {
   if (m_f) fclose(m_f);
}

/*-------------------------------------------------------Tokenizer::Tokenizer-+
|                                                                             |
+----------------------------------------------------------------------------*/
Tokenizer::Tokenizer(char const * path) : Reader(path) {
}

/*------------------------------------------------------------Tokenizer::next-+
|                                                                             |
+----------------------------------------------------------------------------*/
Token Tokenizer::next() {
   m_pCur = nextLine();
   if (!m_pCur) {
      return EOFILE;
   }else if (*m_pCur == '\0') {
      return EMPTY;
   }else if ((*m_pCur == '-') && (*(m_pCur+1) == '-')) {
      return COMMENTS;
   }else {
      char const * pStart = m_pCur;
      char const * pCur = strchr(pStart, ':');
      if (pCur) {
         Token token = dirctvTable.getValue(pStart, pCur-pStart);
         if (token != TESTS) {
            while (*++pCur == ' ');
            m_pCur = pCur;
         }
         return token;
      }
      return TESTS;
   }
}

/*--------------------------------------------------Tokenizer::getStringValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * Tokenizer::getStringValue() const {
   return m_pCur;
}

/*-----------------------------------------------------Tokenizer::toNextField-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::toNextField(char const * pCur) {
   if (*pCur == ' ') {
      while (*++pCur == ' ');
   }else if (*pCur != '\0') {
      return false;
   }
   m_pCur = pCur;
   return true;
}

/*---------------------------------------------------Tokenizer::storeUnquoted-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeUnquoted(char const *& val)
{
   char const * pCur = m_pCur;
   char quote = *pCur;
   val = ++pCur;
   for (;;) {
      if (!*pCur) return false;
      if (*pCur++ == quote) {
         if (*pCur == quote) {
            deleteCharAt(pCur - operator char const *());
         }else {
            break;
         }
      }
   }
   if (!toNextField(pCur)) {
      return false;
   }
   *(char *)(pCur-1) = '\0';           // violates constness
   return true;
}

/*----------------------------------------Tokenizer::storePositiveOrZeroValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storePositiveOrZeroValue(int & value) {
   char const * pCur = m_pCur;
   char c;
   value = 0;
   while (c=*pCur, ((c <= '9') && (c >= '0'))) {
      value = ((value +(value<<2))<<1) + c - '0';   /* value*10 +c - '0' */
      ++pCur;
   }
   if ((value < 0) || (pCur == m_pCur)) {
      return false;
   }
   return toNextField(pCur);
}

/*----------------------------------------------Tokenizer::storePositiveValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storePositiveValue(int & value) {
   char const * pCur = m_pCur;
   char c;
   value = 0;
   while (c=*pCur, ((c <= '9') && (c >= '0'))) {
      value = ((value +(value<<2))<<1) + c - '0';   /* value*10 +c - '0' */
      ++pCur;
   }
   if (!value) {
      return false;
   }
   return toNextField(pCur);
}

/*-----------------------------------------------Tokenizer::storeBooleanValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeBooleanValue(bool & b) {
   switch (*m_pCur) {
   case '0':
      b = false;
      break;
   case '1':
      b = true;
      break;
   default:
      return false;
   }
   return toNextField(m_pCur+1);
}

/*------------------------------------------------Tokenizer::storeStringValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeStringValue(char const *& val)
{
   char const * pCur = m_pCur;
   if (*pCur == '\0') return false;
   val = pCur;
   while (*++pCur && (*pCur != ' '));
   if (!toNextField(pCur)) {
      return false;
   }
   *(char *)pCur = '\0';                // violates constness
   return true;
}

/*---------------------------------------------------Tokenizer::storeRounding-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeRounding(Context::Rounding & round)
{
   char const * sRound;
   if (!storeStringValue(sRound)) {
      locate();
      fprintf(stderr, INTERNALERROR, __FILE__, __LINE__);
      return false;
   }
   if (!roundTable.storeValue(sRound, round)) {
      m_pCur = sRound;  // backup!
      return false;
   }
   return true;
}

/*-----------------------------------------------Tokenizer::storeOperandValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeOperandValue(char const *& val) {
   char const * pCur = m_pCur;
   switch (*pCur) {
   case '\"':
   case '\'':
      return storeUnquoted(val);
   case '#':  // octothorpe, i.e.: a null reference
      val = 0;
      return toNextField(pCur+1);
   default:
      return storeStringValue(val);
   }
}

/*--------------------------------------------------Tokenizer::storeOperation-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeOperation(OpValue::Operation & op)
{
   char const * pCur = m_pCur;
   char const * pOp = pCur;

   while (*pCur && (*++pCur != ' '));
   if (opTable.storeValue(pOp, pCur - pOp, op)) {
      return toNextField(pCur);
   }else {
      return false;
   }
}

/*-----------------------------------------------------Tokenizer::storeResult-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeResult(char const *& val)
{
   if ((*m_pCur != '-') || (*(m_pCur+1) != '>') || !toNextField(m_pCur+2)) {
      return false;
   }else {
      switch (*m_pCur) {
      case '\"':
      case '\'':
         return storeUnquoted(val);
      default:
         return storeStringValue(val);
      }
   }
}

/*-------------------------------------------------Tokenizer::storeConditions-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tokenizer::storeConditions(int & conditions)
{
   conditions = 0;
   while (*m_pCur) {
      char const * sCond;
      if (!storeStringValue(sCond)) {
         locate();
         fprintf(stderr, INTERNALERROR, __FILE__, __LINE__);
         return false;
      }
      if ((strlen(sCond) >= 2) && (sCond[0] == '-') && (sCond[1] == '-')) {
         break;
      }
      if (!condTable.storeValue(sCond, conditions)) {
         m_pCur = sCond;  // backup!
         return false;
      }
   }
   return true;
}


/*---------------------------------------------------------TestCase::TestCase-+
|                                                                             |
+----------------------------------------------------------------------------*/
TestCase::TestCase(Tokenizer & tk, Context const & ctxt) : m_ctxt(ctxt), m_rdr(tk)
{
   char const * error;
   m_operand2 = 0;
   if (
      (error=BADID, !tk.storeStringValue(m_id)) ||
      (error=BADOPERATION, !tk.storeOperation(m_op)) ||
      (error=BADOPERAND, !tk.storeOperandValue(m_operand1)) ||
      (m_op.pBinaryOp && !tk.storeOperandValue(m_operand2)) ||
      (error=NORESULT, !tk.storeResult(m_result)) ||
      (error=BADCONDITION, !tk.storeConditions(m_conditions))
   ) {
      m_isOk = false;
      tk.locate();
      fprintf(stderr, error, tk.getStringValue());
   }else {
      m_isOk = true;
   }
}

/*--------------------------------------------------------------TestCase::run-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestCase::run(Test & test)
{
   if (!m_isOk) {
      printf(TESTINVALID);
   }else {
      if (m_op.pUnaryOp) {
         (test.*m_op.pUnaryOp)(m_operand1, m_ctxt);
      }else {
         (test.*m_op.pBinaryOp)(m_operand1, m_operand2, m_ctxt);
      }
   }
}

/*------------------------------------------------------TestCase::checkResult-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool TestCase::checkResult(char const * result, int conditions)
{
/* (DEBUG)
|  if (result == (char *)-1) {
|     fprintf(stderr, "==> ");
|     m_rdr.locate(stderr);
|     fprintf(stderr, "...\n");
|     fflush(stderr);
|     return true;
|  }
*/
   bool isOk = true;
   if (!result) result = "";  // undefined is like empty?
   if ((!m_conditions || m_ctxt.isExtended()) && strcmp(result, m_result)) {
      // in extended, results are defined even under error conditions
      m_rdr.locate(stdout);
      printf(FAILED, m_id);
      printf(INVRESULT, result, m_result);
      isOk = false;
   }
   if (conditions != m_conditions) {
      if (isOk) {
         m_rdr.locate(stdout);
         printf(FAILED, m_id);
      }
      printf(INVCOND0);
      showConditions(conditions);
      printf(INVCOND1);
      showConditions(m_conditions);
      printf(INVCOND2);
      isOk = false;
   }
   return isOk;
}

/*---------------------------------------------------TestCase::showConditions-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestCase::showConditions(int conditions)
{
   bool isSeparatorNeeded = false;
   for (int i=0; i < sizeof condValues / sizeof condValues[0]; ++i) {
      if (conditions & condValues[i].value) {
         if (isSeparatorNeeded) {
            printf(" ");
         }else {
            isSeparatorNeeded = true;
         }
         printf(condValues[i].name);
      }
   }
}

/*-----------------------------------------------Test::Statistics::Statistics-+
|                                                                             |
+----------------------------------------------------------------------------*/
Test::Statistics::Statistics() {
   m_nbrOfTestsRan = 0;
   m_nbrOfTestsFailing = 0;
   m_invalidTestFound = false;
}

/*-----------------------------------------------Test::Statistics::Statistics-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Test::Statistics::operator+=(Statistics const & source) {
   m_nbrOfTestsRan += source.m_nbrOfTestsRan;
   m_nbrOfTestsFailing += source.m_nbrOfTestsFailing;
   if (!source.m_invalidTestFound) m_invalidTestFound = false;
}

/*------------------------------------------------------------------Test::run-+
|                                                                             |
+----------------------------------------------------------------------------*/
Test::Statistics Test::run(char const * testPath)
{
   Tokenizer tk(testPath);
   ContextImpl ctxt;
   char const * error = 0;
   for (;;) {
      switch (tk.next()) {
      case EMPTY:
      case COMMENTS:
      case VERSION_DIRECTIVE:
         continue;
      case DECTEST_DIRECTIVE:
         {
            char const * name;
            if (!tk.storeStringValue(name)) {
               error = BADDECTEXT;
               break;
            }else {
               static char const sDecTest[] = ".decTest";
               int lenDir;
               int lenName = strlen(name);
               char * c_p = strrchr(testPath, '/');
               if (!c_p) c_p = strrchr(testPath, '\\');
               if (c_p) {
                  lenDir = c_p + 1 - testPath;
               }else {
                  lenDir = 0;
               }
               char * path = new char[lenDir + lenName + sizeof sDecTest];
               memcpy(path, testPath, lenDir);
               memcpy(path+lenDir, name, lenName);
               memcpy(path+lenDir+lenName, sDecTest, sizeof sDecTest);
               run(path);
               delete [] path;
               if (m_stats.m_invalidTestFound) break;
            }
         }
         continue;
      case EXTENDED_DIRECTIVE:
         if (!tk.storeBooleanValue(ctxt.m_bExtended)) {
            error = BADEXTENDED;
            break;
         }
         continue;
      case PRECISION_DIRECTIVE:
         if (!tk.storePositiveValue(ctxt.m_precision)) {
            error = BADPRECISION;
            break;
         }
         continue;
      case MAXEXPONENT_DIRECTIVE:
         if (!tk.storePositiveOrZeroValue(ctxt.m_maxExponent)) {
            error = BADMAXEXPONENT;
            break;
         }
         continue;
      case ROUNDING_DIRECTIVE:
         if (!tk.storeRounding(ctxt.m_round)) {
            error = BADROUNDINGKWD;
            break;
         }
         continue;
      case TESTS:
         if (!ctxt.isOk()) {
            error = BADCONTEXT;
            break;
         }else {
            ++m_stats.m_nbrOfTestsRan;
            char testCase[sizeof (TestCase)];
            m_pTestCase = new(&testCase) TestCase(tk, ctxt);
            m_pTestCase->run(*this);
            m_pTestCase->~TestCase();
         }
         continue;
      default: // case EOFILE:
         break;
      }
      break;
   }
   if (error) {
      tk.locate();
      fprintf(stderr, error, tk.getStringValue());
      m_stats.m_invalidTestFound = true;
   }
   return m_stats;
}

/*----------------------------------------------------------Test::checkResult-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Test::checkResult(char const * result, int conditions)
{
   if (!m_pTestCase->checkResult(result, conditions)) {
      ++m_stats.m_nbrOfTestsFailing;
      return false;
   }else {
      return true;
   }
}

/*===========================================================================*/
