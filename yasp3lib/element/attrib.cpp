/*
* $Id: attrib.cpp,v 1.7 2002-04-01 10:21:21 pgr Exp $
*
* Manage attribute lists of elements / data entities
*/

/*---------+
| Includes |
*---------*/
#include "attrib.h"
#include "../syntax/dlmlist.h"
#include "../syntax/sdcl.h"

Attribute const Attribute::Nil;
Attlist const Attlist::Nil;

/*-------------------------------------------------------Attribute::Attribute-+
| Nil constructor                                                             |
+----------------------------------------------------------------------------*/
Attribute::Attribute(
) :
   dclv(ADCLV_INVALID),
   dftv(ADFTV_INVALID),
   dlmLit(Delimiter::IX_NOT_FOUND),
   bSpecified(0)
{
}

/*-------------------------------------------------------Attribute::Attribute-+
| Constructor (regular)                                                       |
+----------------------------------------------------------------------------*/
Attribute::Attribute(
   UnicodeString const & ucsName,      // attribute name
   UnicodeString const & ucsSpecArg,   // specified value (also for default)
   ArgListSimple const & arglEnumArg,  // if the value is one of an enum
   e_AttDclVal dclvArg,                // type of declared value
   e_AttDftVal dftvArg,                // type of default value
   unsigned int bSpecifiedArg,         // Otherwise, this is the default
   Delimiter::e_Ix dlmLitArg           // one of: LIT, LITA, NOT_FOUND
) :
   RefdKeyRep(ucsName),
   ucsSpec(ucsSpecArg),
   arglEnum(arglEnumArg),
   dclv(dclvArg),
   dftv(dftvArg),
   dlmLit(dlmLitArg)
{
   if (bSpecifiedArg) bSpecified = 1; else bSpecified = 0;
}

/*-------------------------------------------------------Attribute::Attribute-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
Attribute::Attribute(
   UnicodeString const & ucsName,
   e_AttDclVal dclvArg
) :
   RefdKeyRep(ucsName),
   dclv(dclvArg),
   dftv(ADFTV_INVALID),
   dlmLit(Delimiter::IX_NOT_FOUND),
   bSpecified(0)
{
}

/*-----------------------------------------------------------Attribute::clone-+
| Deep copy                                                                   |
+----------------------------------------------------------------------------*/
Attribute * Attribute::clone() const {
   return new Attribute(
      key().copy(), ucsSpec.copy(), arglEnum.copy(),
      dclv, dftv, bSpecified, dlmLit
   );
}

/*------------------------------------------------------Attribute::operator==-+
| Equality                                                                    |
+----------------------------------------------------------------------------*/
bool Attribute::operator==(Attribute const & source) const {
   if (
      (ucsSpec == source.ucsSpec) &&
      (arglEnum == source.arglEnum) &&
      (dclv == source.dclv) &&
      (dftv == source.dftv)
   ) {
      return true;
   }else {
      return false;
   }
}

/*----------------------------------------------------Attribute::setValueSpec-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Attribute::setValueSpec(
   UnicodeString const & ucsSpecArg,
   Delimiter::e_Ix dlmLitArg,
   bool isSpecification
) {
   ucsSpec = ucsSpecArg;
   dlmLit = dlmLitArg;
   if (!ucsSpec) {
      if ((dclv == ADCLV_ENUM) || (dclv == ADCLV_NOTATION)) {
         dftv = ADFTV_REQUIRED;
      }else {
         dftv = ADFTV_IMPLIED;
      }
   }
   if (isSpecification) bSpecified = 1; else bSpecified = 0;
}

/*---------------------------------------------------------Attribute::extract-+
| Write the attribute in a normalized form.                                   |
+----------------------------------------------------------------------------*/
void Attribute::extract(Writer & uost, SgmlDecl const & sdcl) const
{
   if (isValidSpec()) {
      if (*key()) {            // don't do this for unnamed attribute!
         uost << key() << sdcl.delimList()[Delimiter::IX_VI];
      }
      UnicodeString const & sepa = sdcl.delimList()[dlmLit]; // equal for MSVC
      uost << sepa << ucsSpec << sepa;
   }
}

/*-------------------------------------------------------Attribute::stringize-+
| Write the attribute in a normalized form, using the ref concrete syntax.    |
+----------------------------------------------------------------------------*/
void Attribute::stringize(Writer & uost) const
{
   if (isValidSpec()) {
      if (*key()) {            // don't do this for unnamed attribute!
         uost << key() << DelimList::rcsValue(Delimiter::IX_VI);
      }
      UnicodeString const & sepa = DelimList::rcsValue(dlmLit);
      uost << sepa << ucsSpec << sepa;
   }
}

/*--------------------------------------------------------Attlist::defineAttr-+
| Add an attribute to the list                                                |
+----------------------------------------------------------------------------*/
Attribute * Attlist::defineAttr(
   UnicodeString const & ucsName,
   e_AttDclVal dclv
) {
   Attribute * pAtt = new Attribute(ucsName, dclv);
   if (pAtt) {
      if (!*pAtt) {
         delete pAtt;
         pAtt = 0;
      }else {
         if (!insert(pAtt)) pAtt = 0;
      }
   }
   return pAtt;
}

/*-----------------------------------------------------------Attlist::replace-+
| Make a deep copy of an attribute, and replace it in the attribute list.     |
| Note: pAttSource *must* belong to the list!                                 |
+----------------------------------------------------------------------------*/
Attribute * Attlist::replace(Attribute const * pAttSource)
{
   Attribute * pAtt = new Attribute(pAttSource->key(), pAttSource->dclv);
   if (pAtt) {
      if (!*pAtt) {
         delete pAtt;
         pAtt = 0;
      }else {
         if (!replaceAt(pAtt, ix(pAttSource))) pAtt = 0;
      }
   }
   return pAtt;
}

/*------------------------------------------------------Attlist::setValueSpec-+
| Set the value for a given attribute.                                        |
| If the attribute doesn't exist, one is created on the fly.                  |
|                                                                             |
| Note: this function is provided for external callers.                       |
+----------------------------------------------------------------------------*/
bool Attlist::setValueSpec(
   UnicodeString const & ucsName,
   UnicodeString const & ucsSpec,
   Delimiter::e_Ix dlmLit,
   e_AttDclVal dclv
) {
   Attribute * pAtt;
   Attribute const * pAttSource = inqAttributePtr(ucsName);
   if (!pAttSource) {
      pAtt = defineAttr(ucsName, dclv);
   }else {
      pAtt = replace(pAttSource);
   }
   if (pAtt) {
      pAtt->setValueSpec(ucsSpec, dlmLit, true);
      return true;
   }else {
      return false;
   }
}

/*--------------------------------------------------------------Attlist::copy-+
| Deep copy                                                                   |
+----------------------------------------------------------------------------*/
Attlist Attlist::copy() const
{
   int const iLast = count();

   Attlist target(iLast);          // where average count is the exact count!
   for (int ixAtt=0; ixAtt < iLast; ++ixAtt) {
      if (!target.insertLast(inqAttributePtr(ixAtt)->clone())) {
         return Attlist();
      }
   }
   return target;
}

/*--------------------------------------------------------Attlist::operator==-+
| Equality                                                                    |
+----------------------------------------------------------------------------*/
bool Attlist::operator==(Attlist const & source) const {
   int const iLast = count();
   if (iLast != (int)(source.count())) return false;
   for (int ixAtt=0; ixAtt < iLast; ++ixAtt) {
      Attribute const * pAtt1 = inqAttributePtr(ixAtt);
      Attribute const * pAtt2 = source.inqAttributePtr(pAtt1->key());
      if (!pAtt2 || (*pAtt1 != *pAtt2)) return false;
   }
   return true;
}

/*---------------------------------------------------------Attlist::findToken-+
| Find a token in one of the arglEnum                                         |
| Return the attribute it's coming from - or 0 if not found.                  |
+----------------------------------------------------------------------------*/
Attribute const * Attlist::findToken(UCS_2 const * pUcToken) const
{
   int const ixLast = count();
   for (int ixAtt=0; ixAtt < ixLast; ++ixAtt) {
      if ((inqAttributePtr(ixAtt)->findToken(pUcToken)).good()) {
         return inqAttributePtr(ixAtt);
      }
   }
   return 0;
}

/*-----------------------------------------------------------Attlist::extract-+
| Write the attribute list in a normalized form.                              |
+----------------------------------------------------------------------------*/
void Attlist::extract(Writer & uost, SgmlDecl const & sdcl) const
{
   UnicodeString const & ucsSpace = sdcl.charset().inqUcSpace(); // = for MSVC
// bool isFirst = true;

   int const ixLast = count();
   for (int ixAtt=0; ixAtt < ixLast; ++ixAtt) {
      Attribute const * pAtt = inqAttributePtr(ixAtt);
      if (pAtt->isValidSpec()) {
//       if (isFirst) {
//          isFirst = false;
//       }else {
            uost << ucsSpace;
//       }
         pAtt->extract(uost, sdcl);
      }
   }
}

/*---------------------------------------------------------Attlist::stringize-+
| Write the attribute list in a normalized form, using the ref concrete syntax|
| When isFullAttlist is false, only the attributes which have been properly   |
| specified  are stringized.  I.E.:                                           |
| <!ATTLIST foo   name NAME bozo  count NUMBER 1 >                            |
| <foo name="bar" badvalue>                                                   |
|    stringizes into:  "name=bar count=1 badvalue"  if isFullAttlist is true  |
|                      "name=bar"                   if isFullAttlist is false |
|                                                                             |
| Return false if overflow.                                                   |
+----------------------------------------------------------------------------*/
void Attlist::stringize(
   Writer & uost,
   bool isFullAttlist
) const
{
// bool isFirst = true;

   int const ixLast = count();
   for (int ixAtt=0; ixAtt < ixLast; ++ixAtt) {
      Attribute const * pAtt = inqAttributePtr(ixAtt);
      if (pAtt->isReportable(isFullAttlist)) {
//       if (isFirst) {
//          isFirst = false;
//       }else {
            uost << (UCS_2)' ';
//       }
         pAtt->stringize(uost);
      }
   }
}

/*----------------------------------------------------Attlist::getFirstEntity-+
|                                                                             |
+----------------------------------------------------------------------------*/
//>> PGR.  Feb 24, 1996.
//>> This piece of code will vanish when
//>> the value spec of attlist's is a simple TpKeyList (or ArgListSimple)

bool Attlist::getFirstEntity(
   UCS_2 * pUcEntName, UCS_2 ucSpace
) const {
   if (b.entity) {
      int const ixLast = count();
      for (int ixAtt=0; ixAtt < ixLast; ++ixAtt) {
         switch (inqAttributePtr(ixAtt)->dclv) {
         case ADCLV_ENTITY:
         case ADCLV_ENTITIES:
            {
               UCS_2 const * pUcSource = inqAttributePtr(ixAtt)->ucsSpec;
               if (pUcSource) {
                  while (*pUcSource && (*pUcSource != ucSpace)) {
                    *pUcEntName++ = *pUcSource++;
                  }
                  *pUcEntName = 0;
                  return true;
               }
            }
            break;
         default:
            break;
         }
      }
   }
   return false;
}
/*===========================================================================*/
