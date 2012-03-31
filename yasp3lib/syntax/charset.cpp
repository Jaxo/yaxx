/*
* $Id: charset.cpp,v 1.8 2011-07-29 10:26:40 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include <string.h>
#include "rsvdname.h"
#include "charset.h"


UCS_2 const Charset::ucBSEQ = (UCS_2)'B';
#define CHARSET_DEBUG_SETTYPE  /* don't define for deep debug only */


/*-----------------------------------------------------------Charset::Charset-+
| Constructor for XML reference concrete syntax                               |
| Set RE, RS, SPACE and Build achUcType, uctab                                |
|                                                                             |
| Latest Update: as of W3C Working Draft 07-Aug-97                            |
|                http://www.w3.org/TR/WD-xml-970807                           |
|                                                                             |
| Base XML productions:                                                       |
|                                                                             |
|  [1]  S               ::= (#x20 | #x9 | #xd | #xa)+                         |
|  [2]  Char            ::= #x9 | #xA | #xD |                                 |
|                           [#x20-#xFFFD] |                                   |
|                           [#x00010000-#x7FFFFFFF]                           |
|  [82] BaseChar        ::= [ list of values ]                                |
|  [83] Ideographic     ::= [ list of values ]                                |
|  [84] CombiningChar   ::= [ list of values ]                                |
|  [85] Letter          ::= [ list of values ]                                |
|  [86] Digit           ::= [ list of values ]                                |
|  [87] Ignorable       ::= [ list of values ]                                |
|  [88] Extender        ::= [ list of values ]                                |
|  [85] Letter          ::=  BaseChar | Ideographic                           |
|                                                                             |
|  [3]  MiscName        ::=  '.' | '-' | '_' | ':' |                          |
|                            CombiningChar | Ignorable | Extender             |
|  [4]  NameChar        ::=  Letter | Digit | MiscName                        |
|  [5]  Name            ::=  (Letter | '_' | ':') (NameChar)*                 |
|                                                                             |
|                                                                             |
| That we interpret into partions (no intersection):                          |
|                                                                             |
|  XCH_PART_NON_SGML    ::=  [#x0-#x8] | [#xB-#xC] | [#xE-#x1F] |             |
|                            [#xFFFE-#xFFFF]                                  |
|  XCH_PART_WHITE_SPACE ::=  (#x20 | #x9 | #xd | #xa)                         |
|  XCH_PART_DIGIT       ::=  [list of values]                                 |
|  XCH_PART_LETTER      ::=  XCH_RSV_BASE_CHAR | XCH_RSV_IDEOGRAPHIC |        |
|                       ::=  '_' | ':'                                        |
|  XCH_PART_MISC_NAME   ::=  '-' | '.' |                                      |
|                            XCH_RSV_COMBINING_CHAR |                         |
|                            XCH_RSV_IGNORABLE |                              |
|                            XCH_RSV_EXTENDER                                 |
|                                                                             |
| We add XCH_PART_MARKUP_SUPPRESS to recognize mso, msi, mss.                 |
|                                                                             |
| Then, we have the following methods:                                        |
|    isRegular()    not (XCH_PART_WHITE_SPACE |                               |
|                        XCH_PART_MARKUP_SUPPRESS | XCH_PART_NON_SGML)        |
|    isNonSgml()    XCH_PART_NON_SGML                                         |
|    isName()       XCH_PART_LETTER |  XCH_PART_MISC_START |                  |
|                   XCH_PART_DIGIT | XCH_PART_MISC_NAME                       |
|    isNameStart()  XCH_PART_LETTER | XCH_PART_MISC_START                     |
|    isSeparator()  XCH_PART_WHITE_SPACE                                      |
|    isDigit()      XCH_PART_DIGIT                                            |
|    isCref()       XCH_PART_DIGIT | XCH_PART_LETTER | XCH_PART_MISC_START    |
|    isBlank()      XCH_PART_WHITE_SPACE && non RE or RS                      |
|                                                                             |
+----------------------------------------------------------------------------*/
Charset::Charset()
{

   bCaseGeneral = true;
   bCaseEntity = false;
   ucRE = (UCS_2)'\r';
   ucRS = (UCS_2)'\n';
   ucSPACE = (UCS_2)' ';
   ucTAB = (UCS_2)'\t';
   ucMSI = 0;
   ucMSO = 0;
   ucMSS = 0;

   #if !defined COM_JAXO_DENY_UCS_TABLES
   buildUCSTables();
   #endif

   ucsRE = UnicodeString(ucRE);
   ucsRS = UnicodeString(ucRS);
   ucsSPACE = UnicodeString(ucSPACE);
   ucsTAB = UnicodeString(ucTAB);
}

/*----------------------------------------------------------Charset::~Charset-+
|                                                                             |
+----------------------------------------------------------------------------*/
Charset::~Charset() {
   #if !defined COM_JAXO_DENY_UCS_TABLES
   delete [] safeTable;
   #endif
}

/*----------------------------------------------------------Charset::isMinData-+
| For picky SGML parsers...                                                    |
+-----------------------------------------------------------------------------*/
bool Charset::isMinData(int uc) const
{
   if (isAlnum(uc)) {
      return true;
   }else {
      switch (uc) {
      case (UCS_2)' ':
      case (UCS_2)'-':
      case (UCS_2)'.':
      case (UCS_2)'\'':
      case (UCS_2)'(':
      case (UCS_2)')':
      case (UCS_2)'+':
      case (UCS_2)',':
      case (UCS_2)'/':
      case (UCS_2)':':
      case (UCS_2)'=':
      case (UCS_2)'?':
         return true;
      default:
         if ((uc == ucRE) || (uc == ucRS) || (uc == ucSPACE)) {
            return true;
         }
         break;
      }
   }
   return false;
}

#if defined COM_JAXO_CHARSET_WANT_DEBUG
/*-------------------------------------------------------------Charset::showMe-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void Charset::showMe(unsigned char chType)
{
   int iStart = -1;
   int i=0;
   do {
      if (iStart != -1) {
         if (
            (i == CODESET_SIZE) ||
            !(achUcType[i] & chType)
         ) {
            cout << ios::hex << iStart;
            if (i > iStart+1) cout << " - " << ios::hex << i-1;
            cout << endl;
            iStart = -1;
         }
      }else {
         if (achUcType[i] & chType) {
            iStart = i;
         }
      }
   }while (++i <= CODESET_SIZE);
}
#endif

/*-----------------------------------------------------------Charset::charType-+
|                                                                              |
+-----------------------------------------------------------------------------*/
Charset::e_CharType Charset::charType(int uc) const
{
   switch (getXmlType(uc)) {
   case XCH_PART_WHITE_SPACE:
      if (uc == ucSPACE)  return TYPE_SEPA_SPACE;
      if (uc == ucTAB)    return TYPE_SEPA_TAB;
      if (uc == ucRE)     return TYPE_SEPA_RE;
      if (uc == ucRS)     return TYPE_SEPA_RS;
      return TYPE_SEPA_OTHER;
   case XCH_PART_MARKUP_SUPPRESS:
      if (uc == ucMSI)    return TYPE_MSI;
      if (uc == ucMSO)    return TYPE_MSO;
      if (uc == ucMSS)    return TYPE_MSS;
      return TYPE_FUNCTION;
   case XCH_PART_NON_SGML:
      return TYPE_NON_SGML;
   default:
      return TYPE_REGULAR;
   }
}

/*----------------------------------------------------------Charset::setClass-+
| Implementation Note: the current class of the character has to be examined  |
| first, so we dont change -- for example -- a BaseChar to MiscChar.          |
+----------------------------------------------------------------------------*/
#if !defined COM_JAXO_DENY_UCS_TABLES
bool Charset::setClass(int uc, Charset::e_Class cls)
{
   switch (cls) {
   case CLS_NAME:
      if (!isName(uc)) achUcType[uc] = XCH_PART_MISC_NAME;
      break;
   case CLS_NAMESTRT:
      if (!isNamestart(uc)) achUcType[uc] = XCH_RSV_MISC_START;
      break;
   default: /* CLS_DIGIT: */
      achUcType[uc] = XCH_PART_DIGIT;
      break;
   }
   return true;
}
#endif

/*---------------------------------------------------------------FuncharTable-+
| Constructor for reference concrete syntax                                   |
| //>>> JUST A STUB - No time today (01/22/96)                                |
| //>>> If you fix me, also think about turning on XCH_PART_MARKUP_SUPPRESS!  |
+----------------------------------------------------------------------------*/
FuncharTable::FuncharTable()
{
   fctable = new Vector[iCountFc = 4];

   //>>>PGR: this should be pointing variant! (static init problem?)
   fctable[0].ucsName = ReservedName::rcsName(ReservedName::IX_RE);
   fctable[1].ucsName = ReservedName::rcsName(ReservedName::IX_RS);
   fctable[2].ucsName = ReservedName::rcsName(ReservedName::IX_SPACE);

   //>>>PGR: no - only if ref syntax!
   fctable[3].ucsName = ReservedName::rcsName(ReservedName::IX_TAB);

   fctable[0].ucVal = (UCS_2)'\r';
   fctable[1].ucVal = (UCS_2)'\n';
   fctable[2].ucVal = (UCS_2)' ';
   fctable[3].ucVal = (UCS_2)'\t';

   fctable[0].cls = FCTCLS_RE;
   fctable[1].cls = FCTCLS_RS;
   fctable[2].cls = FCTCLS_SPACE;
   fctable[3].cls = FCTCLS_SEPCHAR;
}

/*----------------------------------------------------------------FuncharTable-+
| Copy Constructor (deep)                                                      |
+-----------------------------------------------------------------------------*/
FuncharTable::FuncharTable(FuncharTable const & source)
{
   fctable = new Vector[iCountFc = source.iCountFc];
   for (unsigned int i = 0; i < iCountFc; ++ i) {
       fctable[i] = source.fctable[i];
   }
}

/*-------------------------------------------------------------------operator=-+
| Assignement                                                                  |
+-----------------------------------------------------------------------------*/
FuncharTable & FuncharTable::operator=(FuncharTable const & source)
{
   this->~FuncharTable();
   new(this) FuncharTable(source);
   return *this;
}

/*---------------------------------------------------------------~FuncharTable-+
| Destructor                                                                   |
+-----------------------------------------------------------------------------*/
FuncharTable::~FuncharTable()
{
   delete [] fctable;
}

/*--------------------------------------------------------FuncharTable::value-+
|                                                                             |
+----------------------------------------------------------------------------*/
UCS_2 FuncharTable::value(UnicodeString const & name) const
{
   for (unsigned int i = 0; i < iCountFc; ++i) {
      if (name == fctable[i].ucsName) {
         return fctable[i].ucVal;
      }
   }
   return 0;
}

/*============================================================================+
|   UCS Tables                                                                |
+============================================================================*/
#if defined COM_JAXO_DENY_UCS_TABLES

/*--------------------------------------------------------Charset::getXmlType-+
|                                                                             |
+----------------------------------------------------------------------------*/
unsigned char Charset::getXmlType(int uc) const
{
   if ((unsigned short)uc < 0x100) {
      if (isalpha((unsigned char)uc)) {
         return XCH_RSV_BASE_CHAR;
      }else if (isdigit((unsigned char)uc)) {
         return XCH_PART_DIGIT;
      }else {
         switch ((unsigned char)uc) {
         case '_':
         case ':':
            return XCH_RSV_MISC_START;
         case '-':
         case '.':
            return XCH_PART_MISC_NAME;
         case 0x09:
         case 0x0A:
         case 0x20:
         case 0x0D:
            return XCH_PART_WHITE_SPACE;
         default:
            if ((unsigned char)uc > 0x20) return XCH_PART_CHARACTER;
            break;
         }
         return XCH_PART_NON_SGML;
      }
   }else {
      if ((unsigned short)uc != 0xFFFF) {
         return XCH_RSV_BASE_CHAR;
      }else {
         return XCH_PART_NON_SGML;
      }
   }
}


/* ======  !COM_JAXO_DENY_UCS_TABLES   ====== */
#else

unsigned char const Charset::upperXlateTable[] = {
   "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
   "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
   "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F"
   "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F"
   "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F"
   "\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F"
   "\x60\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F"
   "\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x7B\x7C\x7D\x7E\x7F"
   "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F"
   "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F"
   "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF"
   "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF"
   "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF"
   "\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF"
   "\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF"
   "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF"
};

#ifndef CHARSET_DEBUG_SETTYPE
#define CHARSET_DEBUG_SETTYPE \
   if (achUcType[i] > 2) \
   cerr << "*** Setting: " << (int)chType \
   << " and found " << hex << i << " already been: " \
   << (int)(achUcType[i]) << endl
#endif

/*----------------------------------------------------Charset::setTypeInRange-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline void Charset::setTypeInRange(
   int iStart, int iEnd, unsigned char chType
) {
   for (int i=iStart; i <= iEnd; ++i) {
      CHARSET_DEBUG_SETTYPE;
      achUcType[i] = chType;
   }
}

/*-----------------------------------------------------Charset::setTypeSingle-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline void Charset::setTypeSingle(int i, unsigned char chType) {
   CHARSET_DEBUG_SETTYPE;
   achUcType[i] = chType;
}


/*----------------------------------------------------Charset::buildUCSTables-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Charset::buildUCSTables()
{
   safeTable = new unsigned char[1 + CODESET_SIZE]; // for EOF_SIGNAL: -1
   safeTable[0] = XCH_PART_NON_SGML;
   achUcType = safeTable + 1;
   memset(achUcType, XCH_PART_CHARACTER, CODESET_SIZE);

   upper_EOF = 0;

   //------------- NonSgml --------------
   setTypeInRange(0x0, 0x8,  XCH_PART_NON_SGML);
   setTypeSingle( 0xB,       XCH_PART_NON_SGML);
   setTypeSingle( 0xC,       XCH_PART_NON_SGML);
   setTypeInRange(0xE, 0x1F, XCH_PART_NON_SGML);
   setTypeSingle( 0xFFFE,    XCH_PART_NON_SGML);
   setTypeSingle( 0xFFFF,    XCH_PART_NON_SGML);

   //------------- WhiteSpace --------------
   setTypeSingle( 0x9,   XCH_PART_WHITE_SPACE);
   setTypeSingle( 0xA,   XCH_PART_WHITE_SPACE);
   setTypeSingle( 0xD,   XCH_PART_WHITE_SPACE);
   setTypeSingle( 0x20,  XCH_PART_WHITE_SPACE);

   //------------- Miscellaneous -----------
   setTypeSingle( '_', XCH_RSV_MISC_START);
   setTypeSingle( ':', XCH_RSV_MISC_START);
   setTypeSingle( '-', XCH_PART_MISC_NAME);
   setTypeSingle( '.', XCH_PART_MISC_NAME);

   //------------- BaseChar ----------------
   setTypeInRange(0x0041, 0x005A, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0061, 0x007A, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x00AA, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x00B5, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x00BA, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x00C0, 0x00D6, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x00D8, 0x00F6, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x00F8, 0x00FF, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0100, 0x017F, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0180, 0x01F5, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x01FA, 0x0217, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0250, 0x02A8, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x02B0, 0x02B8, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x02BB, 0x02C1, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x02E0, 0x02E4, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x037A, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0386, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0388, 0x038A, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x038C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x038E, 0x03A1, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x03A3, 0x03CE, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x03D0, 0x03D6, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x03DA, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x03DC, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x03DE, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x03E0, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x03E2, 0x03F3, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0401, 0x040C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x040E, 0x044F, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0451, 0x045C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x045E, 0x0481, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0490, 0x04C4, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x04C7, 0x04C8, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x04CB, 0x04CC, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x04D0, 0x04EB, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x04EE, 0x04F5, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x04F8, 0x04F9, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0531, 0x0556, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0559, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0561, 0x0587, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x05D0, 0x05EA, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x05F0, 0x05F2, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0621, 0x063A, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0641, 0x064A, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0671, 0x06B7, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x06BA, 0x06BE, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x06C0, 0x06CE, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x06D0, 0x06D3, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x06D5, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x06E5, 0x06E6, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0905, 0x0939, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x093D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0958, 0x0961, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0985, 0x098C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x098F, 0x0990, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0993, 0x09A8, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x09AA, 0x09B0, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x09B2, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x09B6, 0x09B9, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x09DC, 0x09DD, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x09DF, 0x09E1, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x09F0, 0x09F1, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A05, 0x0A0A, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A0F, 0x0A10, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A13, 0x0A28, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A2A, 0x0A30, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A32, 0x0A33, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A35, 0x0A36, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A38, 0x0A39, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A59, 0x0A5C, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0A5E, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A72, 0x0A74, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A85, 0x0A8B, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0A8D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A8F, 0x0A91, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0A93, 0x0AA8, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0AAA, 0x0AB0, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0AB2, 0x0AB3, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0AB5, 0x0AB9, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0ABD, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0AE0, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B05, 0x0B0C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B0F, 0x0B10, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B13, 0x0B28, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B2A, 0x0B30, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B32, 0x0B33, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B36, 0x0B39, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0B3D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B5C, 0x0B5D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B5F, 0x0B61, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B85, 0x0B8A, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B8E, 0x0B90, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B92, 0x0B95, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B99, 0x0B9A, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0B9C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0B9E, 0x0B9F, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0BA3, 0x0BA4, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0BA8, 0x0BAA, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0BAE, 0x0BB5, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0BB7, 0x0BB9, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C05, 0x0C0C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C0E, 0x0C10, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C12, 0x0C28, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C2A, 0x0C33, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C35, 0x0C39, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C60, 0x0C61, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C85, 0x0C8C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C8E, 0x0C90, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0C92, 0x0CA8, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0CAA, 0x0CB3, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0CB5, 0x0CB9, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0CDE, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0CE0, 0x0CE1, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0D05, 0x0D0C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0D0E, 0x0D10, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0D12, 0x0D28, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0D2A, 0x0D39, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0D60, 0x0D61, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0E01, 0x0E2E, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0E30, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0E32, 0x0E33, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0E40, 0x0E45, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0E81, 0x0E82, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0E84, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0E87, 0x0E88, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0E8A, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0E8D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0E94, 0x0E97, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0E99, 0x0E9F, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0EA1, 0x0EA3, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0EA5, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0EA7, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0EAA, 0x0EAB, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0EAD, 0x0EAE, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0EB0, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0EB2, 0x0EB3, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x0EBD, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0EC0, 0x0EC4, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0EDC, 0x0EDD, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0F40, 0x0F47, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x0F49, 0x0F69, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x10A0, 0x10C5, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x10D0, 0x10F6, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1100, 0x1159, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x115F, 0x11A2, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x11A8, 0x11F9, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1E00, 0x1E9B, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1EA0, 0x1EF9, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1F00, 0x1F15, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1F18, 0x1F1D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1F20, 0x1F45, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1F48, 0x1F4D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1F50, 0x1F57, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x1F59, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x1F5B, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x1F5D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1F5F, 0x1F7D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1F80, 0x1FB4, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FB6, 0x1FBC, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x1FBE, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FC2, 0x1FC4, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FC6, 0x1FCC, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FD0, 0x1FD3, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FD6, 0x1FDB, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FE0, 0x1FEC, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FF2, 0x1FF4, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x1FF6, 0x1FFC, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x207F, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x2102, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x2107, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x210A, 0x2113, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x2115, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x2118, 0x211D, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x2124, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x2126, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0x2128, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x212A, 0x2131, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x2133, 0x2138, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x2160, 0x2182, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x3041, 0x3094, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x30A1, 0x30FA, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x3105, 0x312C, XCH_RSV_BASE_CHAR);
   setTypeInRange(0x3131, 0x318E, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xAC00, 0xD7A3, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB00, 0xFB06, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB13, 0xFB17, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB1F, 0xFB28, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB2A, 0xFB36, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB38, 0xFB3C, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0xFB3E, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB40, 0xFB41, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB43, 0xFB44, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB46, 0xFB4F, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFB50, 0xFBB1, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFBD3, 0xFD3D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFD50, 0xFD8F, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFD92, 0xFDC7, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFDF0, 0xFDFB, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFE70, 0xFE72, XCH_RSV_BASE_CHAR);
   setTypeSingle( 0xFE74, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFE76, 0xFEFC, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFF21, 0xFF3A, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFF41, 0xFF5A, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFF66, 0xFF6F, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFF71, 0xFF9D, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFFA0, 0xFFBE, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFFC2, 0xFFC7, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFFCA, 0xFFCF, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFFD2, 0xFFD7, XCH_RSV_BASE_CHAR);
   setTypeInRange(0xFFDA, 0xFFDC, XCH_RSV_BASE_CHAR);

   //------------- IdeoGraphic -------------
   setTypeSingle( 0x3007, XCH_RSV_IDEOGRAPHIC);
   setTypeInRange(0x3021, 0x3029, XCH_RSV_IDEOGRAPHIC);
   setTypeInRange(0x4E00, 0x9FA5, XCH_RSV_IDEOGRAPHIC);
   setTypeInRange(0xF900, 0xFA2D, XCH_RSV_IDEOGRAPHIC);

   //------------- CombiningChar -----------
   setTypeInRange(0x0300, 0x0345, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0360, 0x0361, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0483, 0x0486, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0591, 0x05A1, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x05A3, 0x05B9, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x05BB, 0x05BD, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x05BF, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x05C1, 0x05C2, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x05C4, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x064B, 0x0652, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0670, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x06D6, 0x06DC, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x06DD, 0x06DF, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x06E0, 0x06E4, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x06E7, 0x06E8, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x06EA, 0x06ED, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0901, 0x0903, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x093C, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x093E, 0x094C, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x094D, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0951, 0x0954, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0962, 0x0963, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0981, 0x0983, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x09BC, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x09BE, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x09BF, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x09C0, 0x09C4, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x09C7, 0x09C8, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x09CB, 0x09CD, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x09D7, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x09E2, 0x09E3, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0A02, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0A3C, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0A3E, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0A3F, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0A40, 0x0A42, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0A47, 0x0A48, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0A4B, 0x0A4D, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0A70, 0x0A71, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0A81, 0x0A83, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0ABC, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0ABE, 0x0AC5, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0AC7, 0x0AC9, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0ACB, 0x0ACD, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0B01, 0x0B03, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0B3C, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0B3E, 0x0B43, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0B47, 0x0B48, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0B4B, 0x0B4D, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0B56, 0x0B57, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0B82, 0x0B83, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0BBE, 0x0BC2, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0BC6, 0x0BC8, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0BCA, 0x0BCD, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0BD7, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0C01, 0x0C03, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0C3E, 0x0C44, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0C46, 0x0C48, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0C4A, 0x0C4D, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0C55, 0x0C56, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0C82, 0x0C83, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0CBE, 0x0CC4, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0CC6, 0x0CC8, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0CCA, 0x0CCD, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0CD5, 0x0CD6, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0D02, 0x0D03, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0D3E, 0x0D43, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0D46, 0x0D48, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0D4A, 0x0D4D, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0D57, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0E31, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0E34, 0x0E3A, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0E47, 0x0E4E, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0EB1, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0EB4, 0x0EB9, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0EBB, 0x0EBC, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0EC8, 0x0ECD, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0F18, 0x0F19, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0F35, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0F37, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0F39, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0F3E, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0F3F, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0F71, 0x0F84, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0F86, 0x0F8B, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0F90, 0x0F95, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0F97, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0F99, 0x0FAD, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x0FB1, 0x0FB7, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x0FB9, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x20D0, 0x20DC, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x20E1, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0x302A, 0x302F, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x3099, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0x309A, XCH_RSV_COMBINING_CHAR);
   setTypeSingle( 0xFB1E, XCH_RSV_COMBINING_CHAR);
   setTypeInRange(0xFE20, 0xFE23, XCH_RSV_COMBINING_CHAR);

   //------------- Digit -------------------
   setTypeInRange(0x0030, 0x0039, XCH_PART_DIGIT);
   setTypeInRange(0x0660, 0x0669, XCH_PART_DIGIT);
   setTypeInRange(0x06F0, 0x06F9, XCH_PART_DIGIT);
   setTypeInRange(0x0966, 0x096F, XCH_PART_DIGIT);
   setTypeInRange(0x09E6, 0x09EF, XCH_PART_DIGIT);
   setTypeInRange(0x0A66, 0x0A6F, XCH_PART_DIGIT);
   setTypeInRange(0x0AE6, 0x0AEF, XCH_PART_DIGIT);
   setTypeInRange(0x0B66, 0x0B6F, XCH_PART_DIGIT);
   setTypeInRange(0x0BE7, 0x0BEF, XCH_PART_DIGIT);
   setTypeInRange(0x0C66, 0x0C6F, XCH_PART_DIGIT);
   setTypeInRange(0x0CE6, 0x0CEF, XCH_PART_DIGIT);
   setTypeInRange(0x0D66, 0x0D6F, XCH_PART_DIGIT);
   setTypeInRange(0x0E50, 0x0E59, XCH_PART_DIGIT);
   setTypeInRange(0x0ED0, 0x0ED9, XCH_PART_DIGIT);
   setTypeInRange(0x0F20, 0x0F29, XCH_PART_DIGIT);
   setTypeInRange(0xFF10, 0xFF19, XCH_PART_DIGIT);

   //------------- Ignorable ---------------
   setTypeInRange(0x200C, 0x200F, XCH_RSV_IGNORABLE);
   setTypeInRange(0x202A, 0x202E, XCH_RSV_IGNORABLE);
   setTypeInRange(0x206A, 0x206F, XCH_RSV_IGNORABLE);
   setTypeSingle( 0xFEFF, XCH_RSV_IGNORABLE);       // ???????

   //------------- Extender ----------------
   setTypeSingle( 0x00B7, XCH_RSV_EXTENDER);
   setTypeSingle( 0x02D0, XCH_RSV_EXTENDER);
   setTypeSingle( 0x02D1, XCH_RSV_EXTENDER);
   setTypeSingle( 0x0387, XCH_RSV_EXTENDER);
   setTypeSingle( 0x0640, XCH_RSV_EXTENDER);
   setTypeSingle( 0x0E46, XCH_RSV_EXTENDER);
   setTypeSingle( 0x0EC6, XCH_RSV_EXTENDER);
   setTypeSingle( 0x3005, XCH_RSV_EXTENDER);
   setTypeInRange(0x3031, 0x3035, XCH_RSV_EXTENDER);
   setTypeInRange(0x309B, 0x309E, XCH_RSV_EXTENDER);
   setTypeInRange(0x30FC, 0x30FE, XCH_RSV_EXTENDER);
   setTypeSingle( 0xFF70, XCH_RSV_EXTENDER);
   setTypeSingle( 0xFF9E, XCH_RSV_EXTENDER);
   setTypeSingle( 0xFF9F, XCH_RSV_EXTENDER);
}

#endif

/*============================================================================*/
