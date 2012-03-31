/*
* $Id: charset.h,v 1.10 2011-07-29 10:26:40 pgr Exp $
*
* The Charset class describes the SGML partitions for the character set used.
* Note: by design, the EOF_SIGNAL will pass properly the tests.
*       However, uppercasing the EOF_SIGNAL will result in a zero.
*       (you shouldn't do this)
*/

#if !defined CHARSET_HEADER && defined __cplusplus
#define CHARSET_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include <iostream>
#include "../parser/yspenum.h"
#include "../../toolslib/ucstring.h"

/*---------------------------------------------------------------FuncharTable-+
| This class should not be used (private to Charset)                          |
+----------------------------------------------------------------------------*/
class YASP3_API FuncharTable {  // private instance in Charset
public:
   FuncharTable();
   FuncharTable(FuncharTable const & source);
   FuncharTable & operator=(FuncharTable const& source);
   ~FuncharTable();
   UCS_2 value(UnicodeString const & name) const;

   bool replace(              // for RE/RS/SPACE only
      e_FctCls clsArg,
      UCS_2 ucValArg
   );
   bool add(                  // no RE/RS/SPACE here!
      UnicodeString & ucsNameArg,
      e_FctCls clsArg,
      UCS_2 ucValArg
   );
private:
   class Vector {               // to map :
   public:
      UnicodeString ucsName;    // a function name (ex: "TAB")
      UCS_2 ucVal;          // to a corresponding char number (ex: 9)
      e_FctCls cls;             // Function Class (ex: FCTCLS_SEPCHAR)
   };
   Vector * fctable;            // Function Character <-> Char Nmbr
   unsigned int iCountFc;       // count of entries in fncr table
};

/*--------------------------------------------------------------------Charset-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API Charset {
public:
   /*-------------------+
   | Public Definitions |
   +-------------------*/
   static UCS_2 const ucBSEQ;
   enum e_Class {
      CLS_NAMESTRT,
      CLS_NAME,
      CLS_DIGIT
   };
   enum e_CharType {
      TYPE_REGULAR,
      TYPE_SEPA_RE,
      TYPE_SEPA_RS,
      TYPE_SEPA_TAB,
      TYPE_SEPA_SPACE,
      TYPE_SEPA_OTHER,
      TYPE_MSI,
      TYPE_MSO,
      TYPE_MSS,
      TYPE_FUNCTION,
      TYPE_NON_SGML
   };

   /*-----------------+
   | Public Functions |
   +-----------------*/
   Charset();                          // Charset of Reference Syntax
   ~Charset();

   UCS_2 RE() const;               // what's a RE?
   UCS_2 RS() const;               // what's a RS?
   UCS_2 inqUcSpace() const;       // what's a SPACE?
   UCS_2 inqUcTab() const;         // what's a TAB? (can be 0)
   UnicodeString const REstr() const;
   UnicodeString const RSstr() const;
   UnicodeString const SPACEstr() const;
   UnicodeString const TABstr() const;

   // classification
   bool isRegular(int uc) const;    // nothing special? (surely: data)
   bool isNonsgml(int uc) const;    // a NONSGML?
   bool isName(int uc) const;       // a name character? ([52])
   bool isNamestart(int uc) const;  // can start a name? ([53])
   bool isSeparator(int uc) const;  // an S Separator? ([5])
   bool isDigit(int uc) const;      // a digit?
   bool isCref(int uc) const;       // match the CREF constraint?
   bool isBlank(int uc) const;      // a Blank (SPACE+SEPCHAR)? ([4.25])

   bool isREchar(int uc) const;     // a RE?
   bool isRSchar(int uc) const;     // a RS?
   bool isSPACEchar(int uc) const;  // a SPACE?
   bool isTABchar(int uc) const;    // a TAB (if tab is defined!)
   bool isMinData(int uc) const;    // minimum data? ([77,78])
   bool isMsChar(int uc) const;     // a function character? ([54]-1st line)
   bool isMsiChar(int uc) const;    // a MSI?
   bool isMsoChar(int uc) const;    // a MSO?
   bool isMssChar(int uc) const;    // a MSS?
   bool isCaseGeneral() const;      // NAMECASE  GENERAL  (YES)
   bool isCaseEntity() const;       //           ENTITY   (NO)
   UCS_2 uppercase(int uc) const;  // uppercase this character
   void uppercase(UCS_2 * pUc) const; // uppercase this string
   UCS_2 funchar(
      UnicodeString const & name
   ) const;

   e_CharType charType(int uc) const;
   bool isClass(int c, Charset::e_Class cls) const;
   bool setClass(int c, Charset::e_Class cls);

   // the 2 following should be temporaries
   void setCaseGeneral(bool is) { bCaseGeneral = is; }
   void setCaseEntity(bool is)  { bCaseEntity = is; }

   #if defined COM_JAXO_CHARSET_WANT_DEBUG
      void showMe(unsigned char chType);    // debug
   #endif

private:

   enum e_cType {
      XCH_PART_NON_SGML        = 1 << 0,
      XCH_PART_CHARACTER       = 1 << 1,
      XCH_PART_WHITE_SPACE     = 1 << 2,
      XCH_PART_LETTER          = 1 << 3,
        XCH_RSV_BASE_CHAR = XCH_PART_LETTER,
        XCH_RSV_IDEOGRAPHIC = XCH_PART_LETTER,
        XCH_RSV_MISC_START = XCH_PART_LETTER,
      XCH_PART_DIGIT           = 1 << 4,
      XCH_PART_MISC_NAME       = 1 << 5,
        XCH_RSV_COMBINING_CHAR = XCH_PART_MISC_NAME,
        XCH_RSV_IGNORABLE      = XCH_PART_MISC_NAME,
        XCH_RSV_EXTENDER       = XCH_PART_MISC_NAME,
      XCH_PART_MARKUP_SUPPRESS = 1 << 6      // MSO, MSI, MSS, other FUNCHAR
   };

   #if defined COM_JAXO_DENY_UCS_TABLES
      unsigned char getXmlType(int uc) const;

   #else
      enum { CODESET_SIZE = 0x10000 };
      unsigned char * safeTable;
      unsigned char * achUcType;
      unsigned short upper_EOF;           // Should be before upperXlateTable!
      static unsigned char const upperXlateTable[257]; // upper case table

      void buildUCSTables();
      unsigned char getXmlType(int uc) const;
   #endif

   UCS_2 ucRE;                     // Record End Character
   UCS_2 ucRS;                     // Record Start Character
   UCS_2 ucSPACE;                  // Space Character
   UCS_2 ucTAB;                    // Tab character ('\0' if none)
   UCS_2 ucMSI;
   UCS_2 ucMSO;
   UCS_2 ucMSS;

   UnicodeString ucsRE;
   UnicodeString ucsRS;
   UnicodeString ucsSPACE;
   UnicodeString ucsTAB;

   FuncharTable fctb;                  // Function characters

   bool bCaseGeneral;
   bool bCaseEntity;

   bool isAlnum(int uc) const;
   void setTypeInRange(int iStart, int iEnd, unsigned char chType);
   void setTypeSingle(int i, unsigned char chType);
};

/*--- INLINES ---*/

/* =======  COM_JAXO_DENY_UCS_TABLES   ====== */
#if defined COM_JAXO_DENY_UCS_TABLES
#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif

inline UCS_2 Charset::uppercase(int uc) const {
   if ((unsigned int)uc < 0x100) {
      return toupper(uc);
   }else {
      return uc;
   }
}

inline void Charset::uppercase(UCS_2 * pUc) const {
   for (UCS_2 uc = *pUc; uc; uc = *++pUc) {
      if ((unsigned int)uc < 0x100) {
         *pUc = toupper(uc);
      }
   }
}

/* ======  !COM_JAXO_DENY_UCS_TABLES   ====== */
#else
inline unsigned char Charset::getXmlType(int uc) const {
   return achUcType[uc];
}

inline UCS_2 Charset::uppercase(int uc) const {
   if (uc < (int)sizeof upperXlateTable-1) {
      return upperXlateTable[uc];
   }else {
      return uc;
   }
}

inline void Charset::uppercase(UCS_2 * pUc) const {
   for (UCS_2 uc = *pUc; uc; uc = *++pUc) {
      if (uc < (int)sizeof upperXlateTable-1) {
         *pUc = upperXlateTable[uc];
      }
   }
}

#endif
/* ================================================= */

inline UCS_2 Charset::RE() const {
   return ucRE;
}
inline UCS_2 Charset::RS() const {
   return ucRS;
}
inline UCS_2 Charset::inqUcSpace() const {
   return ucSPACE;
}
inline UCS_2 Charset::inqUcTab() const {
   return ucTAB;
}
inline UnicodeString const Charset::REstr() const {
   return ucsRE;
}
inline UnicodeString const Charset::RSstr() const {
   return ucsRS;
}
inline UnicodeString const Charset::SPACEstr() const {
   return ucsSPACE;
}
inline UnicodeString const Charset::TABstr() const {
   return ucsTAB;
}
inline bool Charset::isCref(int uc) const {  //>>>PGR: REVIEW ME
   if (getXmlType(uc) & (XCH_PART_DIGIT | XCH_PART_LETTER)) {
      return true;
   }else {
      return false;
   }
}

inline bool Charset::isRegular(int uc) const {
   if (getXmlType(uc) & (
         XCH_PART_WHITE_SPACE | XCH_PART_MARKUP_SUPPRESS | XCH_PART_NON_SGML
      )
   ) {
      return false;
   }else {
      return true;
   }
}
inline bool Charset::isNonsgml(int uc) const {
   if (getXmlType(uc) == XCH_PART_NON_SGML) return true; else return false;
}
inline bool Charset::isName(int uc) const {
   if (
      getXmlType(uc) & (XCH_PART_LETTER | XCH_PART_DIGIT | XCH_PART_MISC_NAME)
   ) {
      return true;
   }else {
      return false;
   }
}
inline bool Charset::isNamestart(int uc) const {
   if (getXmlType(uc) == XCH_PART_LETTER) {
      return true;
   }else {
      return false;
   }
}
inline bool Charset::isSeparator(int uc) const {
   if (getXmlType(uc) == XCH_PART_WHITE_SPACE) {
      return true;
   }else {
      return false;
   }
}
inline bool Charset::isDigit(int uc) const {
   if (getXmlType(uc) == XCH_PART_DIGIT) return true; else return false;
}
inline bool Charset::isAlnum(int uc) const {
   if (getXmlType(uc) & (XCH_PART_DIGIT | XCH_PART_LETTER)) {
      return true;
   }else {
      return false;
   }
}

inline bool Charset::isBlank(int uc) const {   // B-Seq
   if ((getXmlType(uc) == XCH_PART_WHITE_SPACE) &&
      (uc != ucRE) && (uc != ucRS)
   ) {
      return true;
   }else {
      return false;
   }
}

inline bool Charset::isMsChar(int uc) const {
   if (uc & XCH_PART_MARKUP_SUPPRESS) return true; else return false;
}

inline bool Charset::isREchar(int uc) const {
   if (uc == ucRE) return true; else return false;
}
inline bool Charset::isRSchar(int uc) const {
   if (uc == ucRS) return true; else return false;
}
inline bool Charset::isSPACEchar(int uc) const {
   if (uc == ucSPACE) return true; else return false;
}
inline bool Charset::isTABchar(int uc) const {
   if ((ucTAB) && (uc == ucTAB)) return true; else return false;
}


inline bool Charset::isMsiChar(int uc) const {
   if (uc == ucMSI) return true; else return false;
}
inline bool Charset::isMsoChar(int uc) const {
   if (uc == ucMSO) return true; else return false;
}
inline bool Charset::isMssChar(int uc) const {
   if (uc == ucMSS) return true; else return false;
}
inline bool Charset::isCaseGeneral() const {
   return bCaseGeneral;
}
inline bool Charset::isCaseEntity() const {
   return bCaseEntity;
}
inline UCS_2 Charset::funchar(UnicodeString const & name) const {
   return fctb.value(name);
}
inline bool Charset::isClass(int c, Charset::e_Class cls) const {
   switch (cls) {
   case CLS_NAMESTRT:         return isNamestart(c);
   case CLS_NAME:             return isName(c);
   default: /* CLS_DIGIT: */  return isDigit(c);
   }
}

#endif /* CHARSET_HEADER ====================================================*/
