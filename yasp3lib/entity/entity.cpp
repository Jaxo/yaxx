/*
* $Id: entity.cpp,v 1.6 2002-08-28 02:07:40 jlatone Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include "entity.h"
#include "../syntax/sdcl.h"

Entity const Entity::Nil;
DataNotation const DataNotation::Nil;
Notation const Notation::Nil;

static UnicodeString const ndataTagName("#NDATA");
static UnicodeString const subdocTagName("#SUBDOC");


/*-------------------------------------------------------EntityExt::EntityExt-+
| Description:                                                                |
|   This constructor for conveniency: it is a short cut to quickly build      |
|   an external entity from a name and a system id.                           |
|   The public id is optional.                                                |
+----------------------------------------------------------------------------*/
EntityExt::EntityExt(
   UnicodeString const & ucsName,
   UnicodeString const & ucsSystemId,
   UnicodeString const & ucsPublicId
) :
   Entity(ucsName, EntityFlag(false), ENTK_SGML),
   exid(
      ucsSystemId,           // System Identifier
      UnicodeString::Nil,    // Surrounding Quotes for SID
      ucsPublicId,           // Public Identifier
      UnicodeString::Nil,    // Surrounding Quotes for PID
      ExternId::TEXT         // YSP assigned (vs. pid declared)
   ),
   nad(DataNotation::Nil)
{}

/*------------------------------------------------------------EntityExt::clone-+
| "virtual constructor"                                                        |
+-----------------------------------------------------------------------------*/
Entity * EntityExt::clone(
   UnicodeString const & ucsNameArg,
   TpKeyList const & listNotation       // NCB's in permanent storage
) const {
   DataNotation nadCopy;
   if (nad) {
      nadCopy = DataNotation(
         (Notation const *)listNotation.find(nad.inqNotation().inqName()),
         nad.inqAttlist().copy()
      );
   }
   return new EntityExt(ucsNameArg, inqFlag(), inqKind(), exid.copy(), nadCopy);
}

/*---------------------------------------------------------EntityExt (inquire)-+
|                                                                              |
+-----------------------------------------------------------------------------*/
ExternId const & EntityExt::inqExternId() const {
   return exid;
}
DataNotation const & EntityExt::inqDataNotation() const {
   return nad;
}

/*-------------------------------------------------------EntityExt::operator==-+
|                                                                              |
+-----------------------------------------------------------------------------*/
bool EntityExt::operator==(Entity const & source) const
{
   if (
      (inqKind() == source.inqKind()) &&
      (inqFlag() == source.inqFlag()) &&
      (inqExternId() == source.inqExternId()) &&
      (inqDataNotation() == source.inqDataNotation())
   ) {
      return true;
   }else {
      return false;
   }
}

/*------------------------------------------------------------EntityInt::clone-+
| "virtual constructor"                                                        |
+-----------------------------------------------------------------------------*/
Entity * EntityInt::clone(
   UnicodeString const & ucsNameArg,
   TpKeyList const &                   // unused
) const {
   return new EntityInt(
      ucsNameArg, inqFlag(), inqKind(), ucsLit.copy(), inqDelim()
   );
}

/*---------------------------------------------------------EntityInt (inquire)-+
|                                                                              |
+-----------------------------------------------------------------------------*/
UnicodeString const & EntityInt::inqLiteral() const {
   return ucsLit;
}
Delimiter::e_Ix EntityInt::inqDelim() const {
   return dlmLit;
}

/*-------------------------------------------------------EntityInt::operator==-+
|                                                                              |
+-----------------------------------------------------------------------------*/
bool EntityInt::operator==(Entity const & source) const {
   if (
      (inqKind() == source.inqKind()) &&
      (inqFlag() == source.inqFlag()) &&
      (inqLiteral() == source.inqLiteral())
   ) {
      return true;
   }else {
      return false;
   }
}

/*----------------------------------------------------------Entity::(virtuals)-+
|                                                                              |
+-----------------------------------------------------------------------------*/
Entity * Entity::clone(
   UnicodeString const & ucsNameArg,
   TpKeyList const &                   // unused
) const {
   return new Entity(ucsNameArg, b, kind);
}
Entity::~Entity() {
}
UnicodeString const & Entity::inqLiteral() const {
   return UnicodeString::Nil;
}
Delimiter::e_Ix Entity::inqDelim() const {
   return Delimiter::IX_NOT_FOUND;
}
ExternId const & Entity::inqExternId() const {
   return ExternId::Nil;
}
DataNotation const & Entity::inqDataNotation() const {
   return DataNotation::Nil;
}
bool Entity::operator==(Entity const & source) const {
   if (
      (inqKind() == source.inqKind()) &&
      (inqFlag() == source.inqFlag()) &&
      (inqLiteral() == source.inqLiteral()) &&
      (inqExternId() == source.inqExternId()) &&
      (inqDataNotation() == source.inqDataNotation())
   ) {
      return true;
   }else {
      return false;
   }
}

/*-----------------------------------------------------------Entity::stringize-+
| Effects:                                                                     |
|   Make an informative string when an entity reference has been found.        |
|                                                                              |
|   Used if an NDATA is directly referenced, to create #NDATA element,         |
|   and if an element specifies an attribute of type ENTITY                    |
|   which specifies an NDATA entity.                                           |
|                                                                              |
|   When NDATA entities are referenced, they are automatically                 |
|   surrounded by a pseudo-element.  Given these declarations:                 |
|      <!NOTATION fax   "/bin/faxviewer">                                      |
|      <!ENTITY   fig1  SYSTEM "/tmp/fig1.g4" NDATA fax>                       |
|   any references to entity 'fig1' are treated AS IF the file contained       |
|   this tag (order of attributes matches order of info in ENTITY dcl):        |
|      <#NDATA                                                                 |
|              nd.ent="fig1"                                                   |
|              nd.pub="+//whatsoever//tmp/fig1.g4//TIFF//"                            |
|              nd.sys="/tmp/fig1.g4"                                           |
|              nn.not="fax"                                                    |
|              nn.pub="+//CCITT//The viewer//C//"                              |
|              nn.sys="/bin/faxviewer">                                        |
|   Thus the user has a handle to define it in the style sheet, with           |
|   all relevant info available.                                               |
| Returns:                                                                     |
|   Success flag.                                                              |
+-----------------------------------------------------------------------------*/
void Entity::stringize(
   YaspType type,
   Writer & uost
) const {
   switch (inqKind()) {
   case ENTK_NDATA:
      if (type == YSP__ENTITY_REF) {
         uost
            << ndataTagName << (UCS_2)' '
            <<   "nd.ent='" << inqName()
            << "' nd.pub='" << inqExternId().inqPublicId()
            << "' nd.sys='" << inqExternId().inqSystemId()
            << "' nd.not='" << inqDataNotation().inqName()
            << "' nn.pub='" << inqDataNotation().inqExternId().inqPublicId()
            << "' nn.sys='" << inqDataNotation().inqExternId().inqSystemId();
      }else {
         uost << inqDataNotation().inqNotation().inqName();
      }
      break;
   case ENTK_SUBDOC:
      uost
         << subdocTagName << (UCS_2)' '
         <<   "sd.ent='" << inqName()
         << "' sd.pub='" << inqExternId().inqPublicId()
         << "' sd.sys='" << inqExternId().inqSystemId();
      break;
   default:
      break;
   }
}

/*-------------------------------------------------------------Entity::extract-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void Entity::extract(Writer & out, SgmlDecl const & sdcl) const
{
   UnicodeString const & ucsSpace = sdcl.charset().inqUcSpace(); // = for MSVC

   out << sdcl.delimList()[Delimiter::IX_MDO];
   if (b.specialDtd) {
      out << sdcl.reservedName()[ReservedName::IX_DOCTYPE]
          << ucsSpace << key() << ucsSpace;
      inqExternId().extract(out, sdcl);
      return;   // do not write the MDC!
   }
   out << sdcl.reservedName()[ReservedName::IX_ENTITY];
   if (b.parameter) {
       out << ucsSpace << sdcl.delimList()[Delimiter::IX_PERO];
   }
   if (!key()) {
      out << ucsSpace
          << sdcl.delimList()[Delimiter::IX_RNI]
          << sdcl.reservedName()[ReservedName::IX_DEFAULT];
   }else {
      out << ucsSpace << key();
   }
   if (isExternal() ) {
      out << ucsSpace;
      inqExternId().extract(out, sdcl);
      switch (kind) {
      case ENTK_SDATA:
      case ENTK_NDATA:
      case ENTK_CDATA:
         out << ucsSpace;
         inqDataNotation().extract(out, sdcl, kind);
         break;
      case ENTK_SUBDOC:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_SUBDOC];
         break;
      default:
         break;
      }
   }else {
      switch (kind) {
      case ENTK_CDATA:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_CDATA];
         break;
      case ENTK_SDATA:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_SDATA];
         break;
      case ENTK_MD:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_MD];
         break;
      case ENTK_PI:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_PI];
         break;
      case ENTK_STARTTAG:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_STARTTAG];
         break;
      case ENTK_ENDTAG:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_ENDTAG];
         break;
      case ENTK_MS:
         out << ucsSpace << sdcl.reservedName()[ReservedName::IX_MS];
         break;
      default: // ENTK_TEXT or INVALID?
         break;
      }
      out << ucsSpace
          << sdcl.delimList()[inqDelim()]
          << inqLiteral()
          << sdcl.delimList()[inqDelim()];
   }
   out << sdcl.delimList()[Delimiter::IX_MDC];
}

/*-------------------------------------------------ostream& operator<<(Entity)-+
| Display the contents of an Entity                                            |
+-----------------------------------------------------------------------------*/
ostream & operator<<(ostream& out, Entity const & entdcl)
{
#if !defined MWERKS_NEEDS_Q_FIX  
   int const iRecursion = out.width();

   out << "" << '\"';
   if (entdcl.isSpecialDtd()) {
      out << "#DOCTYPE ";
   }else if (entdcl.isSpecialDoc()) {
      out << "#PRIMARY ";
   }
   if (!entdcl.inqName()) {
      out << "#DEFAULT\"";
   }else {
      out << (entdcl.inqName()) << '\"';
   }
   out << " type: ";
   if (entdcl.isParameter()) {
      out << "parameter ";
   }else {
      out << "general ";
   }
   if (entdcl.isExternal()) {
      out << "external ";
   }else {
      out << "internal ";
   }
   switch(entdcl.kind) {
   case ENTK_TEXT:     out << "TEXT";         break;
   case ENTK_CDATA:    out << "CDATA";        break;
   case ENTK_SDATA:    out << "SDATA";        break;
   case ENTK_PI:       out << "PI";           break;
   case ENTK_STARTTAG: out << "STARTTAG";     break;
   case ENTK_ENDTAG:   out << "ENDTAG";       break;
   case ENTK_MS:       out << "MS";           break;
   case ENTK_MD:       out << "MD";           break;
   case ENTK_NDATA:    out << "NDATA";        break;
   case ENTK_SUBDOC:   out << "SUBDOC";       break;
   case ENTK_SGML:     out << "SGML";         break;
   default:            out << "?????";        break;
   }
   out << endl;
   out.width(iRecursion + 6);
   out << "" << "Value";
   if (entdcl.isInternal()) {
      out << ": \"" << entdcl.inqLiteral() << '\"';
   }else {
      out << " in \"" << entdcl.inqExternId().inqSystemId() << '\"' << endl;
      switch(entdcl.kind) {
      case ENTK_CDATA:
      case ENTK_SDATA:
      case ENTK_NDATA:
         out.width(iRecursion + 6);
         out << entdcl.inqDataNotation();
         break;
      default:
         break;
      }
   }
#endif
   return out;
}

/*-----------------------------------------------------Notation::attachAttlist-+
| Attach an attlist to a given notation                                        |
| Return false if a non-empty attlist was already attached                     |
+-----------------------------------------------------------------------------*/
bool Notation::attachAttlist(Attlist const & attlistArg)
{
   if (!attlst.count()) {
      attlst = attlistArg;
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------------------------------Notation::clone-+
|                                                                              |
+-----------------------------------------------------------------------------*/
Notation * Notation::clone(UnicodeString const & ucsNameArg) const {
   return new Notation(ucsNameArg, inqExternId().copy(), inqAttlist().copy());
}

/*---------------------------------------------------------Notation:operator==-+
| Equality                                                                     |
+-----------------------------------------------------------------------------*/
bool Notation::operator==(Notation const & source) const {
   if (
      (inqExternId() == source.inqExternId()) &&
      (inqAttlist() == source.inqAttlist())
   ) {
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------------------------DataNotation::extract-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void DataNotation::extract(
   Writer & out,
   SgmlDecl const & sdcl,
   e_EntKind kind
) const {
   UnicodeString const & ucsSpace = sdcl.charset().inqUcSpace(); // = for MSVC
   switch (kind) {
   case ENTK_SDATA:
      out << sdcl.reservedName()[ReservedName::IX_SDATA];
      break;
   case ENTK_NDATA:
      out << sdcl.reservedName()[ReservedName::IX_NDATA];
      break;
   default: // ENTK_CDATA:
      out << sdcl.reservedName()[ReservedName::IX_CDATA];
      break;
   }
   out << ucsSpace << kiNcb.key();
   if (attlst.count()) {
      out << sdcl.delimList()[Delimiter::IX_DSO];
      attlst.extract(out, sdcl);
      out << sdcl.delimList()[Delimiter::IX_DSC];
   }
}

/*-----------------------------------------------------DataNotation:operator==-+
| Equality                                                                     |
+-----------------------------------------------------------------------------*/
bool DataNotation::operator==(DataNotation const & source) const {
   if (
      (inqAttlist() == source.inqAttlist()) &&
      (inqName() == source.inqName()) &&           // notation name
      (inqNotation() == source.inqNotation())
   ) {
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------------ostream& operator<<(DataNotation)-+
| Display the contents of a DataNotation                                       |
+-----------------------------------------------------------------------------*/
ostream & operator<<(ostream& out, DataNotation const & nad)
{
#if !defined MWERKS_NEEDS_Q_FIX  
   if (!nad) {
      out << "" << "**unknown** interpreter";
   }else {
      int const iRecursion = out.width();
      out << nad.inqNotation() << endl;
// PGR - will fix and reenable this when back from vacation
#ifdef BOOGABOO_CREEK   // OS__HPUX and MSVC does not like it.
      out.width(iRecursion+10);
      out << nad.inqAttlist();
#endif
   }
#endif
   return out;
}

/*-----------------------------------------------ostream& operator<<(Notation)-+
| Display the contents of an Notation                                          |
+-----------------------------------------------------------------------------*/
ostream & operator<<(ostream& out, Notation const & ncb)
{
    int const iRecursion = out.width();
    out
      << "" << "Interpreter: (" << ncb.inqName() << ") \""
      << ncb.inqExternId().inqSystemId() << '\"';
    return out;
}

/*============================================================================*/
