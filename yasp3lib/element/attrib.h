/*
* $Id: attrib.h,v 1.9 2002-04-14 23:26:04 jlatone Exp $
*
* Manage attribute lists of elements / data entities
*/

#if !defined ATTRIB_HEADER && defined __cplusplus
#define ATTRIB_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "../../toolslib/Writer.h"
#include "../../toolslib/tplist.h"
#include "../../toolslib/arglist.h"
#include "../parser/yspenum.h"
#include "../syntax/delim.h"
class SgmlDecl;


/*-----------------------------------------------------------------AttlistFlag-+
| Flags seen during the parse of the attlist declaration                       |
+-----------------------------------------------------------------------------*/
struct YASP3_API AttlistFlag {        // Attlist decl flag:
   AttlistFlag();
   unsigned int entity    : 1;  //   attlist has ENTITY/ENTITIES
   unsigned int id        : 1;  //   attlist has ID
   unsigned int notation  : 1;  //   attlist has NOTATION
   unsigned int required  : 1;  //   attlist has REQUIRED
   unsigned int current   : 1;  //   attlist has CURRENT
   unsigned int conref    : 1;  //   attlist has CONREF
};
/* -- INLINES -- */
inline AttlistFlag::AttlistFlag() :
   entity (0), id(0), notation(0), required(0), current(0), conref(0)
{}

/*------------------------------------------------------------------Attribute-+
| Represents the declaration for a single attribute in an ATTLIST declaration.|
+----------------------------------------------------------------------------*/
class YASP3_API Attribute : public RefdKeyRep {  // att
   friend class Attlist;
public:
   Attribute();                           // Nil constructor
   Attribute(
      UnicodeString const & ucsName,      // attribute name
      UnicodeString const & ucsSpecArg,   // specified value (also for default)
      ArgListSimple const & arglEnumArg,  // if the value is one of an enum
      e_AttDclVal dclvArg,                // type of declared value
      e_AttDftVal dftvArg,                // type of default value
      unsigned int bSpecifiedArg,         // Otherwise, this is the default
      Delimiter::e_Ix dlmLitArg           // one of: LIT, LITA, NOT_FOUND
   );
   Attribute(                             // no value specified, so far
      UnicodeString const & ucsName,
      e_AttDclVal dclv
   );

   UnicodeString const & inqName() const;
   UnicodeString const & inqValue() const;
   e_AttDclVal inqTypeDeclaredValue() const;
   e_AttDftVal inqTypeDefaultValue() const;
   Delimiter::e_Ix inqDelim() const;
   bool isSpecified() const;           // specified vs. default
   bool isValidSpec() const;           // ucsSpec is valid
   bool isReportable(bool isFullAttlist) const;

   operator void *() const;               // isOk?
   bool operator!() const;             // is not ok?

   bool operator==(Attribute const & source) const;
   bool operator!=(Attribute const & source) const;

   void extract(Writer & out, SgmlDecl const & sdcl) const;
   void stringize(Writer & uost) const;

   UnicodeString const & findToken(UCS_2 const * pUcToken) const;
   Attribute * clone() const;

   void setDefaultVal(
      e_AttDftVal dclvArg
   );
   void setValueSpec(
      UnicodeString const & ucsSpecArg,
      Delimiter::e_Ix dlmLitArg,  // what lit surrounded the value spec?
      bool isSpecification = false
   );
   void addToken(UCS_2 const * pUcToken);

private:
   UnicodeString ucsSpec;        // specified value (also for default)
   ArgListSimple arglEnum;       // if the value is one of an enum
   e_AttDclVal   dclv;           // type of declared value
   e_AttDftVal   dftv;           // type of default value
   unsigned int bSpecified : 1;  // Otherwise, this is the default
   Delimiter::e_Ix dlmLit;       // one of: LIT, LITA, NOT_FOUND

public:
   static const Attribute Nil;
};

/* -- INLINES -- */
inline Attribute::operator void *() const {
   if (key().good()) return (void *)this; else return 0;
}
inline bool Attribute::operator!() const {
   if (key().good()) return false; else return true;
}
inline void Attribute::setDefaultVal(
   e_AttDftVal dftvArg
) {
   dftv = dftvArg;
}
inline UnicodeString const & Attribute::inqName() const {
   return key();
}
inline UnicodeString const & Attribute::inqValue() const {
   return ucsSpec;
}
inline e_AttDclVal Attribute::inqTypeDeclaredValue() const {
   return dclv;
}
inline e_AttDftVal Attribute::inqTypeDefaultValue() const {
   return dftv;
}
inline Delimiter::e_Ix Attribute::inqDelim() const {
   return dlmLit;
}
inline bool Attribute::isSpecified() const {
   if (bSpecified) return true; else return false;
}
inline bool Attribute::isValidSpec() const {
   if ((!ucsSpec) && (dlmLit == Delimiter::IX_NOT_FOUND)) {
      return false;
   }else {
      return true;
   }
}
inline bool Attribute::isReportable(bool isFullAttlist) const
{
   if ((isValidSpec()) && (isFullAttlist || (bSpecified && *key()))) {
      return true;
   }else {
      return false;
   }
}
inline UnicodeString const & Attribute::findToken(
    UCS_2 const * pUcToken
) const {
    return arglEnum[pUcToken];
}
inline void Attribute::addToken(UCS_2 const * pUcToken) {
    arglEnum += pUcToken;
}
inline bool Attribute::operator!=(Attribute const & source) const {
   if (*this == source) return false; else return true;
}

/*--------------------------------------------------------------------Attlist-+
| Represents the declaration for a single attribute in an ATTLIST declaration.|
|                                                                             |
| Note:                                                                       |
|   More than one element can point to the same instance of this object       |
+----------------------------------------------------------------------------*/
class YASP3_API Attlist : public TpKeyList {     // attlst
public:
   Attlist() {}
   Attlist(int iAverageCount) : TpKeyList(iAverageCount) {}

   Attribute const & operator[](UCS_2 const * pUc) const;
   Attribute const & operator[](int ix) const;

   bool isEntity() const;
   bool isId() const;
   bool isNotation() const;
   bool isRequired() const;
   bool isCurrent() const;
   bool isConref() const;

   Attlist copy() const;               // deep copy
   bool operator==(Attlist const & source) const;
   bool operator!=(Attlist const & source) const;

   void extract(Writer & out, SgmlDecl const & sdcl) const;
   void stringize(
      Writer & uost,
      bool isFullAttlist = true
   ) const;

   Attribute const * findToken(UCS_2 const * pUcToken) const;
   Attribute const * inqAttributePtr(UCS_2 const * pUcName) const;
   Attribute const * inqAttributePtr(int ix) const;
   Attribute * defineAttr(
      UnicodeString const & ucsName,
      e_AttDclVal dclv = ADCLV_INVALID
   );
   void setFlag(AttlistFlag bFlagArg);
   Attribute * replace(Attribute const * pAttSource);
   bool setValueSpec(
      UnicodeString const & ucsName,
      UnicodeString const & ucsSpec,
      Delimiter::e_Ix dlmLit = Delimiter::IX_NOT_FOUND,
      e_AttDclVal dclv = ADCLV_CDATA
   );
   bool getFirstEntity(UCS_2 * pUcEntName, UCS_2 ucSpace) const;

private:
   AttlistFlag b;                      // Attlist decl flag
public:
   static Attlist const Nil;
};

/* -- INLINES -- */
inline Attribute const & Attlist::operator[](UCS_2 const * pUc) const {
   Attribute const * pAtt = (Attribute const *)findData(pUc);
   if (pAtt) return *pAtt; else return Attribute::Nil;
}
inline Attribute const & Attlist::operator[](int ix) const {
   Attribute const * pAtt = (Attribute const *)findDataAt(ix);
   if (pAtt) return *pAtt; else return Attribute::Nil;
}
inline void Attlist::setFlag(AttlistFlag bArg) {
   b = bArg;
}
inline bool Attlist::isEntity() const {
   if (b.entity) return true; else return false;
}
inline bool Attlist::isId() const {
   if (b.id) return true; else return false;
}
inline bool Attlist::isNotation() const {
   if (b.notation) return true; else return false;
}
inline bool Attlist::isRequired() const {
   if (b.required) return true; else return false;
}
inline bool Attlist::isCurrent() const {
   if (b.current) return true; else return false;
}
inline bool Attlist::isConref() const {
   if (b.conref) return true; else return false;
}
inline bool Attlist::operator!=(Attlist const & source) const {
   if (*this == source) return false; else return true;
}
inline Attribute const * Attlist::inqAttributePtr(UCS_2 const * pUc) const {
   return (Attribute const *)findData(pUc);
}
inline Attribute const * Attlist::inqAttributePtr(int ix) const {
   return (Attribute const *)findDataAt(ix);
}
#endif /* ATTRIB_HEADER ======================================================*/
