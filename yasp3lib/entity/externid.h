/*
* $Id: externid.h,v 1.5 2011-07-29 10:26:39 pgr Exp $
*
* External Identifiers
*/

#if !defined EXTERNID_HEADER && defined __cplusplus
#define EXTERNID_HEADER

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include <string.h>
#include "../../toolslib/ucstring.h"

#ifdef __MWERKS_NEEDS_MEMCMP_FIX__
#define memcmp memcmpFix4PalmOS30
#include "../../palmiolib/memcmp.h"
#endif

class Writer;
class Syntax;
class SgmlDecl;

/*----------------------------------------------------------------ExternIdFlag-+
| External Identifier Flags                                                    |
+-----------------------------------------------------------------------------*/
struct ExternIdFlag {
   ExternIdFlag();
   bool operator==(ExternIdFlag const & source) const;
   bool operator!=(ExternIdFlag const & source) const;

   unsigned int identified      : 1;
   unsigned int eonChecked      : 1;
   unsigned int knownByEon      : 1;
   unsigned int formal          : 1;
   unsigned int availPublicText : 1;
   unsigned int providePtdv     : 1;
   unsigned int publicTextDs    : 1;
   unsigned int alternate       : 1;
   unsigned int defaulted       : 1;
   unsigned int registered      : 1;
   unsigned int isoRegistration : 1;
   unsigned int publicId        : 1;
   unsigned int systemId        : 1;
};

/* -- INLINES -- */
inline ExternIdFlag::ExternIdFlag() {
   memset(this, 0, sizeof *this);
}
inline bool ExternIdFlag::operator==(ExternIdFlag const & source) const {
   if (!memcmp(this, &source, sizeof *this)) return true; else return false;
}
inline bool ExternIdFlag::operator!=(ExternIdFlag const & source) const {
   if (*this == source) return false; else return true;
}

/*--------------------------------------------------------------------ExternId-+
| External Identifier                                                          |
+-----------------------------------------------------------------------------*/
class ExternId {              // exid: external identifier
public:
   enum e_TextClass {         //  Class Field
      INVALID = 0,            // Invalid, or Irrelevant
      CAPACITY,    /* EON */  // capacity set
      CHARSET,     /* EON */  // character data
      DOCUMENT,    /* EON */  // sgml document
      DTD,         /* EON */  // document type declaration subset
      ELEMENTS,               // element set
      ENTITIES,               // entity set
      LPD,         /* EON */  // link type declaration subset
      NONSGML,                // non sgml data entity
      NOTATION,               // character data
      SHORTREF,               // short reference set
      SUBDOC,                 // sgml subdocument entity
      SYNTAX,                 // concrete syntax
      TEXT         /* ??? */  // sgml text entity
   };
   ExternId();                            // Nil constructor
   ExternId(                              // regular
      UnicodeString const & ucsSystemId,  // System Identifier
      UnicodeString const & ucsDlmSystemId,  // for stringize
      UnicodeString const & ucsPublicId,  // Public Identifier
      UnicodeString const & ucsDlmPublicId,  // for stringize
      e_TextClass clsAssigned = INVALID   // YSP assigned (vs. pid declared)
   );
   ExternId(e_TextClass clsAssigned);     // for SYSTEM implied
   ExternId(ExternId const& source);      // copy constructor
   ~ExternId();

   ExternId copy() const;                 // deep copy

   ExternId& operator=(ExternId const& source); // assignment
   bool operator==(ExternId const & source) const;
   bool operator!=(ExternId const & source) const;

   void formalize(Syntax const & syntax, bool isTrueWarning);
   void extract(Writer & out, SgmlDecl const & sdcl) const;

   UnicodeString const & inqSystemId() const; // System ID:
   UnicodeString const & inqPublicId() const; // Public ID

   UnicodeString const & inqDelimSystemId() const;
   UnicodeString const & inqDelimPublicId() const;

   UnicodeString inqOwnId() const;         // Owner identifier
   UnicodeString inqTxtDesc() const;       // Public text description
   UnicodeString inqLang() const;          // lang, or designating seq
   UnicodeString inqPtdv() const;          // Public Text Display Version
   UnicodeString inqSoi() const;           // EON System Object Id

   e_TextClass inqDefinedClass() const;    // class, as defined in the public id
   e_TextClass inqAssignedClass() const;   // logical class assigned by Parser

   // Are true or false
   bool isIdentified() const;      // A SOI has been found
   bool isEonChecked() const;      // EON was asked to get an SOI
   bool isKnownByEon() const;      // EON was able to get an SOI
   bool isFormal() const;          // PID has a formal structure
   bool isAvailPublicText() const; // Available public text
   bool isProvidePtdv() const;     // System must provide Pub.Text Disp.Vers.
   bool isPublicTextDs() const;    // Pub.Text Design. Sequence vs. Language
   bool isAlternate() const;       // Take alternate system spec if any
   bool isDefaulted() const;       // Defaulted entity
   bool isRegistered() const;      // Owner is registered
   bool isIsoRegistration() const; // Registration is ISO
   bool isPublic() const;          // A Public Identifier was furnished
   bool isSystem() const;          // A System Identifier was furnished


   operator void *() const;           // isOk?
   bool operator!() const;         // is not ok?

private:
   class ExternIdRep {
   public:
      int iReferenceCount;
      UnicodeString ucsSid;          // System Identifier
      UnicodeString ucsPid;          // Public Identifier
      UnicodeString ucsDlmSid;
      UnicodeString ucsDlmPid;
      /*
      | iff is.VALIDFPI, redefine FPI subfields of "pid" (related to buffer)
      */
      UnicodeSubstring sbsOwnid;     // Owner identifier
      UnicodeSubstring sbsTxtDesc;   // Public text description
      UnicodeSubstring sbsLang;      // lang, or designating seq
      UnicodeSubstring sbsPtdv;      // Public Text Display Version

      ExternIdFlag b;                // all flags in here
      e_TextClass clsAssigned;       // Text Class assigned by the Parser
      e_TextClass clsDefined;        // Text Class as specified
      UnicodeString m_soi;           // EON System Object Idr (when solved)

      ExternIdRep(
         UnicodeString const & ucsSystemId,
         UnicodeString const & ucsDlmSystemId,
         UnicodeString const & ucsPublicId,
         UnicodeString const & ucsDlmPublicId,
         e_TextClass clsAssigned
      );
      ExternIdRep(
         e_TextClass clsAssigned
      );

   private:
      ExternIdRep& operator=(ExternIdRep const& source); // no!
      ExternIdRep(ExternIdRep const& source);            // no!
   };
   ExternIdRep *pRep;

public:
   static ExternId const Nil;        // should be here: CSET2 bug (csd 11)
};

/* -- INLINES -- */
inline ExternId::ExternId() : pRep(0) {
}
inline ExternId::ExternId(ExternId const& source) {
   pRep = source.pRep;
   if (pRep) ++pRep->iReferenceCount;
}
inline ExternId::~ExternId() {
   if ((pRep) && (!--pRep->iReferenceCount)) {
      delete pRep;
   }
   pRep = 0;
}
inline ExternId& ExternId::operator=(ExternId const& source) {
   this->~ExternId();
   return * new(this) ExternId(source);
}
inline bool ExternId::operator!=(ExternId const & source) const {
   if (*this == source) return false; else return true;
}
inline UnicodeString const & ExternId::inqSystemId() const {
   if (pRep) return pRep->ucsSid; else return UnicodeString::Nil;
}
inline UnicodeString const & ExternId::inqPublicId() const {
   if (pRep) return pRep->ucsPid; else return UnicodeString::Nil;
}
inline UnicodeString const & ExternId::inqDelimSystemId() const {
   if (pRep) return pRep->ucsDlmSid; else return UnicodeString::Nil;
}
inline UnicodeString const & ExternId::inqDelimPublicId() const {
   if (pRep) return pRep->ucsDlmPid; else return UnicodeString::Nil;
}
inline UnicodeString ExternId::inqOwnId() const {
   if (pRep) return pRep->sbsOwnid; else return UnicodeString::Nil;
}
inline UnicodeString ExternId::inqTxtDesc() const {
   if (pRep) return pRep->sbsTxtDesc; else return UnicodeString::Nil;
}
inline UnicodeString ExternId::inqLang() const {
   if (pRep) return pRep->sbsLang; else return UnicodeString::Nil;
}
inline UnicodeString ExternId::inqPtdv() const {
   if (pRep) return pRep->sbsPtdv; else return UnicodeString::Nil;
}
inline UnicodeString ExternId::inqSoi() const {
   if (pRep) return pRep->m_soi; else return UnicodeString::Nil;
}
inline ExternId::e_TextClass ExternId::inqDefinedClass()  const {
   if (pRep) return pRep->clsDefined; else return INVALID;
}
inline ExternId::e_TextClass ExternId::inqAssignedClass() const {
   if (pRep) return pRep->clsAssigned; else return INVALID;
}
inline bool ExternId::isIdentified() const {
   if (pRep && pRep->b.identified) return true; else return false;
}
inline bool ExternId::isEonChecked() const {
   if (pRep && pRep->b.eonChecked) return true; else return false;
}
inline bool ExternId::isKnownByEon() const {
   if (pRep && pRep->b.knownByEon) return true; else return false;
}
inline bool ExternId::isFormal() const {
   if (pRep && pRep->b.formal) return true; else return false;
}
inline bool ExternId::isAvailPublicText() const {
   if (pRep && pRep->b.availPublicText) return true; else return false;
}
inline bool ExternId::isProvidePtdv() const {
   if (pRep && pRep->b.providePtdv) return true; else return false;
}
inline bool ExternId::isPublicTextDs() const {
   if (pRep && pRep->b.publicTextDs) return true; else return false;
}
inline bool ExternId::isAlternate() const {
   if (pRep && pRep->b.alternate) return true; else return false;
}
inline bool ExternId::isDefaulted() const {
   if (pRep && pRep->b.defaulted) return true; else return false;
}
inline bool ExternId::isRegistered() const {
   if (pRep && pRep->b.registered) return true; else return false;
}
inline bool ExternId::isIsoRegistration() const {
   if (pRep && pRep->b.isoRegistration) return true; else return false;
}
inline bool ExternId::isPublic() const {
   if (pRep && pRep->b.publicId) return true; else return false;
}
inline bool ExternId::isSystem() const {
   if (pRep && pRep->b.systemId) return true; else return false;
}
inline ExternId::operator void *() const {
   if (pRep) return (void *)this; else return 0;
}
inline bool ExternId::operator!() const {
   if (!pRep) return true; else return false;
}
#endif /* EXTERNID_HEADER ====================================================*/
