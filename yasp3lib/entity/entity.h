/*
* $Id: entity.h,v 1.10 2002-04-26 04:31:11 jlatone Exp $
*
* Entity Declarations
*/

#if !defined ENTITY_HEADER && defined __cplusplus
#define ENTITY_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "../parser/yspenum.h"
#include "../syntax/rsvdname.h"
#include "../element/attrib.h"
#include "externid.h"

#ifdef __MWERKS_NEEDS_MEMCMP_FIX__
#define memcmp memcmpFix4PalmOS30
#include "../../palmiolib/memcmp.h"
#endif

class SgmlDecl;

/*--------------------------------------------------------------------Notation-+
| Notation Decl (NCB)                                                          |
+-----------------------------------------------------------------------------*/
class Notation : public RefdKeyRep {   // ncb
public:
   Notation();                       // for Nil (here for hp and nec compilers)
   Notation(
      UnicodeString const & ucsName,
      ExternId const & exidArg
   );
   Notation(
      UnicodeString const & ucsName,
      ExternId const & exidArg,
      Attlist const & attlstArg
   );
   bool attachAttlist(Attlist const & attlistArg);
   void setExternId(ExternId const & exidArg);

   UnicodeString const & inqName() const; // Notation name
   ExternId const & inqExternId() const;
   UnicodeString const & inqPublicId() const;
   UnicodeString const & inqSystemId() const;
   Attlist const & inqAttlist() const;  // Data attributes defined

   operator void *() const;             // isOk?
   bool operator!() const;           // is not ok?
   bool operator==(Notation const & source) const;
   bool operator!=(Notation const & source) const;

   Notation * clone(UnicodeString const & ucsNameArg) const;

   // Debugging
   friend ostream & operator<<(ostream& out, Notation const & source);

private:
   ExternId exid;                       // External Identifier
   Attlist attlst;                      // Attribute Definition List

public:
   static Notation const Nil;           // should be here: CSET2 bug (csd 11)
};

/* -- INLINES -- */
inline Notation::Notation()
{ }
inline Notation::Notation(
   UnicodeString const & ucsName,
   ExternId const & exidArg
) :
   RefdKeyRep(ucsName), exid(exidArg)
{ }
inline Notation::Notation(
   UnicodeString const & ucsName,
   ExternId const & exidArg,
   Attlist const & attlstArg
) :
   RefdKeyRep(ucsName), exid(exidArg), attlst(attlstArg)
{ }
inline void Notation::setExternId(ExternId const & exidArg) {
   exid = exidArg;
}
inline UnicodeString const & Notation::inqName() const {
   return key();
}
inline ExternId const & Notation::inqExternId() const {
   return exid;
}
inline Attlist const & Notation::inqAttlist() const {
   return attlst;
}
inline UnicodeString const & Notation::inqPublicId() const {
   return inqExternId().inqPublicId();
}
inline UnicodeString const & Notation::inqSystemId() const {
   return inqExternId().inqSystemId();
}
inline Notation::operator void *() const {
   if (!exid) return 0; else return (void *)this;
}
inline bool Notation::operator!() const {
   if (!exid) return true; else return false;
}
inline bool Notation::operator!=(Notation const & source) const {
   if (*this == source) return false; else return true;
}

/*----------------------------------------------------------------DataNotation-+
| Data notation for an entity (NAD)                                            |
|                                                                              |
| A DataNotation object describes the notation and data attributes             |
| associated to any external C/N/S DATA entities.                              |
+-----------------------------------------------------------------------------*/
class DataNotation {                 // NAD: Entity Notation Structure
   // friend DataNotation Yasp::parseDataNotation();
public:
   DataNotation();                   // for Nil (here for hp and nec compilers)
   DataNotation(
      Notation const * pNcb,
      Attlist const & attlstArg
   );
   UnicodeString const & inqName() const;
   ExternId const & inqExternId() const;      // ExternId of the notation
   Attlist const & inqAttlist() const;        // Data attributes specified
   Attlist const & inqDefaultAttlist() const; // Data attributes defaulted
   Notation const & inqNotation() const;      // Notation

   operator void *() const;          // isOk?
   bool operator!() const;        // is not ok?
   bool operator==(DataNotation const & source) const;
   bool operator!=(DataNotation const & source) const;

   void extract(
      Writer & out,
      SgmlDecl const & sdcl,
      e_EntKind kind
   ) const;

   // Debugging...
   friend ostream& operator<<(ostream& out, DataNotation const & source);

private:
   RefdKey kiNcb;               // Notation Identifier (magic pointer)
   Attlist attlst;                   // Attribute Specification List

public:
   static DataNotation const Nil;    // should be here: CSET2 bug (csd 11)
};

/* -- INLINES -- */
inline DataNotation::DataNotation()
{ }
inline DataNotation::DataNotation(
   Notation const * pNcb,
   Attlist const & attlstArg
) :
   kiNcb(pNcb), attlst(attlstArg)
{ }
inline UnicodeString const & DataNotation::inqName() const {
   return kiNcb.key();
}
inline ExternId const & DataNotation::inqExternId() const {
   Notation const * pNcb = (Notation const *)kiNcb.inqData();
   if (pNcb) return pNcb->inqExternId(); else return ExternId::Nil;
}
inline Attlist const & DataNotation::inqAttlist() const {
   return attlst;
}
inline Attlist const & DataNotation::inqDefaultAttlist() const {
   Notation const * pNcb = (Notation const *)kiNcb.inqData();
   if (pNcb) return pNcb->inqAttlist(); else return Attlist::Nil;
}
inline Notation const & DataNotation::inqNotation() const {
   Notation const * pNcb = (Notation const *)kiNcb.inqData();
   if (pNcb) return *pNcb; else return Notation::Nil;
}
inline DataNotation::operator void *() const {
   if (kiNcb.isPresent()) return (void *)this; else return 0;
}
inline bool DataNotation::operator!() const {
   if (kiNcb.isPresent()) return false; else return true;
}
inline bool DataNotation::operator!=(DataNotation const & source) const {
   if (*this == source) return false; else return true;
}
/*------------------------------------------------------------------EntityFlag-+
| Entity flags                                                                 |
+-----------------------------------------------------------------------------*/
struct YASP3_API EntityFlag {
   EntityFlag();
   EntityFlag(bool isSpecialDoc);  // for an external entity
   bool operator==(EntityFlag const & source) const;
   bool operator!=(EntityFlag const & source) const;

   unsigned int external   : 1;
   unsigned int rcdata     : 1;
   unsigned int parameter  : 1;
   unsigned int notation   : 1;
   unsigned int deflt      : 1;
   unsigned int specialDoc : 1;
   unsigned int specialDtd : 1;
};

/* -- INLINES -- */
inline EntityFlag::EntityFlag() {
   memset(this, 0, sizeof *this);
}
inline EntityFlag::EntityFlag(bool isSpecialDoc) {
   memset(this, 0, sizeof *this);
   external = 1;
   rcdata = 1;
   specialDoc = isSpecialDoc? 1 : 0;
}
inline bool EntityFlag::operator==(EntityFlag const & source) const {
   if (!memcmp(this, &source, sizeof *this)) return true; else return false;
}
inline bool EntityFlag::operator!=(EntityFlag const & source) const {
   if (*this == source) return false; else return true;
}

/*----------------------------------------------------------------------Entity-+
| Entity                                                                       |
+-----------------------------------------------------------------------------*/
class YASP3_API Entity : public RefdKeyRep { // ent
public:
   Entity();                      // for Nil
   Entity(                        // regular constructor
      UnicodeString const & ucsName,
      EntityFlag bArg,
      e_EntKind kindArg
   );
   UnicodeString const & inqName() const;  // Entity name
   e_EntKind  inqKind() const;
   EntityFlag inqFlag() const;
   YaspEntClass inqEntClass() const;

   // are true or false
   bool isExternal() const;    // true if external entity, false if internal
   bool isInternal() const;    // true if internal entity, false if external
   bool isParameter() const;   // true if parameter entity, false if general
   bool isGeneral() const;     // true if general entity, false if parameter
   bool isDefault() const;     // true if this entity instanciates #DEFAULT
   bool isRcdata() const;      // false if CDATA/SDATA/NDATA/PI or SUBDOC
   bool isSpecialDoc() const;  // true if primary entity
   bool isSpecialDtd() const;  // true if document entity

   void stringize(YaspType type, Writer & out) const;
   void extract(Writer & out, SgmlDecl const & sdcl) const;

   operator void *() const;       // isOk?
   bool operator!() const;     // is not ok?
   bool operator!=(Entity const & source) const;

   UnicodeString const & inqPublicId() const;
   UnicodeString const & inqSystemId() const;

   // Debugging...
   friend ostream & operator<<(ostream& out, Entity const & source);

   // Virtuals
   virtual Entity * clone(
      UnicodeString const & ucsNameArg, TpKeyList const &
   ) const;
   virtual ~Entity();
   virtual UnicodeString const & inqLiteral() const;     // literal (Nil if Ext)
   virtual Delimiter::e_Ix inqDelim() const;             // NOT_FOUND if Ext
   virtual ExternId const & inqExternId() const;         // Nil if Internal
   virtual DataNotation const & inqDataNotation() const; // Nil if Internal
   virtual bool operator==(Entity const & source) const;

private:
   EntityFlag b;                  // bit field
   e_EntKind kind;                // value field

   Entity & operator=(Entity const& source); // no! (should be virtual)
   Entity(Entity const & source);            // no! (should be virtual)

public:
   static Entity const Nil;    // should be here: CSET2 bug
};

/* -- INLINES -- */
inline Entity::Entity(
) :
   kind(ENTK_INVALID) {
}
inline Entity::Entity(
   UnicodeString const & ucsName,
   EntityFlag bArg,
   e_EntKind kindArg
) :
   RefdKeyRep(ucsName), kind(kindArg), b(bArg) {
}
inline UnicodeString const & Entity::inqName() const {
   return key();
}
inline e_EntKind Entity::inqKind() const {
   return kind;
}
inline EntityFlag Entity::inqFlag() const {
   return b;
}
inline YaspEntClass Entity::inqEntClass() const {
   if (b.specialDoc || b.specialDtd) {
      return ENT_SPECIAL;
   }else if (b.parameter) {
      return ENT_PARAMETER;
   }else {
      return ENT_GENERAL;
   }
}
inline Entity::operator void *() const {
   if ((kind == ENTK_INVALID) && !key()) return 0; else return (void *)this;
}
inline bool Entity::operator!() const {
   if ((kind == ENTK_INVALID) && !key()) return true; else return false;
}
inline bool Entity::operator!=(Entity const & source) const {
   if (*this == source) return false; else return true;
}
inline bool Entity::isExternal() const {
   if (b.external) return true; else return false;
}
inline bool Entity::isInternal() const {
   if (b.external) return false; else return true;
}
inline bool Entity::isParameter() const {
   if (b.parameter) return true; else return false;
}
inline bool Entity::isGeneral() const {
   if (b.parameter) return false; else return true;
}
inline bool Entity::isDefault() const {
   if (b.deflt) return true; else return false;
}
inline bool Entity::isRcdata() const {
   if (b.rcdata) return true; else return false;
}
inline bool Entity::isSpecialDoc() const {
   if (b.specialDoc) return true; else return false;
}
inline bool Entity::isSpecialDtd() const {
   if (b.specialDtd) return true; else return false;
}
inline UnicodeString const & Entity::inqPublicId() const {
   return inqExternId().inqPublicId();
}
inline UnicodeString const & Entity::inqSystemId() const {
   return inqExternId().inqSystemId();
}

/*-------------------------------------------------------------------EntityExt-+
| External Entity                                                              |
+-----------------------------------------------------------------------------*/
class EntityExt : public Entity {   // entext
public:
   EntityExt(                 // regular constructor
      UnicodeString const & ucsName,
      EntityFlag b,
      e_EntKind kind,
      ExternId const & exidArg,
      DataNotation const & nadArg
   );
   // Following constructor is for conveniency.
   EntityExt(
      UnicodeString const & ucsName,
      UnicodeString const & ucsSystemId,
      UnicodeString const & ucsPublicId = UnicodeString::Nil
   );

   Entity * clone(
      UnicodeString const & ucsNameArg,
      TpKeyList const & listNotation       // NCB's in permanent storage
   ) const;
   ExternId const & inqExternId() const;
   DataNotation const & inqDataNotation() const;
   bool operator==(Entity const & source) const;

private:
   ExternId const exid;          // External Identifier
   DataNotation const nad;       // Notation + Attrib

   EntityExt& operator=(EntityExt const& source); // no!
   EntityExt(EntityExt const& source);            // no!
};
/* -- INLINES -- */
inline EntityExt::EntityExt(
   UnicodeString const & ucsName,
   EntityFlag bArg,
   e_EntKind kindArg,
   ExternId const & exidArg,
   DataNotation const & nadArg
) :
   Entity(ucsName, bArg, kindArg), exid(exidArg), nad(nadArg)
{ }

/*-------------------------------------------------------------------EntityInt-+
| Internal Entity                                                              |
+-----------------------------------------------------------------------------*/
class EntityInt : public Entity {   // entint
public:
   EntityInt(                    // regular constructor
      UnicodeString const & ucsName,
      EntityFlag b,
      e_EntKind kind,
      UnicodeString const & ucsLitArg,
      Delimiter::e_Ix dlmLitArg = Delimiter::IX_LIT
   );
   Entity * clone(
      UnicodeString const & ucsNameArg,
      TpKeyList const &
   ) const;
   Delimiter::e_Ix inqDelim() const;
   UnicodeString const & inqLiteral() const;
   bool operator==(Entity const & source) const;
private:
   UnicodeString ucsLit;         // literal
   Delimiter::e_Ix dlmLit;       // one of: LIT, LITA

   EntityInt & operator=(EntityInt const & source); // no!
   EntityInt(EntityInt const & source);             // no!
};
/* -- INLINES -- */
inline EntityInt::EntityInt(
   UnicodeString const & ucsName,
   EntityFlag bArg,
   e_EntKind kindArg,
   UnicodeString const & ucsLitArg,
   Delimiter::e_Ix dlmLitArg
) :
   Entity(ucsName, bArg, kindArg), ucsLit(ucsLitArg), dlmLit(dlmLitArg) {
}
#endif /* ENTITY_HEADER ======================================================*/
