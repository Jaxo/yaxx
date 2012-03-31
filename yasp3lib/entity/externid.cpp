/*
* $Id: externid.cpp,v 1.3 2002-04-01 10:21:22 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include "externid.h"
#include "../../toolslib/Writer.h"
#include "../syntax/sdcl.h"

ExternId const ExternId::Nil;

/*------------------------------------------ExternId::ExternIdRep::ExternIdRep-+
|                                                                              |
+-----------------------------------------------------------------------------*/
inline ExternId::ExternIdRep::ExternIdRep(
   UnicodeString const & ucsSystemId,  // System Identifier
   UnicodeString const & ucsDlmSystemId,
   UnicodeString const & ucsPublicId,  // Public Identifier
   UnicodeString const & ucsDlmPublicId,
   e_TextClass clsAssignedArg          // Parser assigned (vs. pid declared)
) :
   ucsSid(ucsSystemId),
   ucsPid(ucsPublicId),
   ucsDlmSid(ucsDlmSystemId),
   ucsDlmPid(ucsDlmPublicId),
   clsAssigned(clsAssignedArg),
   clsDefined(INVALID)               // until it is formalized
{
   iReferenceCount = 0;
   if (ucsPid.good()) b.publicId = 1;
   if (ucsSid.good()) b.systemId = 1;
}

inline ExternId::ExternIdRep::ExternIdRep(
   e_TextClass clsAssignedArg
) :
   clsAssigned(clsAssignedArg),
   clsDefined(INVALID)
{
   iReferenceCount = 0;
   b.systemId = 1;
}

/*----------------------------------------------------------ExternId::ExternId-+
|                                                                              |
+-----------------------------------------------------------------------------*/
ExternId::ExternId(
   UnicodeString const & ucsSystemId,  // System Identifier
   UnicodeString const & ucsDlmSystemId,
   UnicodeString const & ucsPublicId,  // Public Identifier
   UnicodeString const & ucsDlmPublicId,
   e_TextClass clsAssigned             // YSP assigned (vs. pid declared)
) {
   if (!ucsSystemId && !ucsPublicId) {
      pRep = 0;
   }else {
      pRep = new ExternIdRep(
         ucsSystemId, ucsDlmSystemId,
         ucsPublicId, ucsDlmPublicId,
         clsAssigned
      );
   }
   if (pRep) {
      ++pRep->iReferenceCount;
   }else {
      pRep = 0;
   }
}

/*----------------------------------------------------------ExternId::ExternId-+
| For: <!ENTITY foo SYSTEM -- SYSTEM implied -->                               |
+-----------------------------------------------------------------------------*/
ExternId::ExternId(e_TextClass clsAssigned)
{
   pRep = new ExternIdRep(clsAssigned);
   if (pRep) {
      ++pRep->iReferenceCount;
   }else {
      pRep = 0;
   }
}


/*--------------------------------------------------------------ExternId::copy-+
| Deep copy                                                                    |
+-----------------------------------------------------------------------------*/
ExternId ExternId::copy() const
{
   if (pRep) {
      return ExternId(
         pRep->ucsSid.copy(),
         pRep->ucsPid.copy(),
         pRep->ucsDlmSid.copy(),
         pRep->ucsDlmPid.copy(),
         pRep->clsAssigned
      );
   }else {
      return ExternId::Nil;
   }
}

/*----------------------------------------------------------ExternId::ExternId-+
| Equality                                                                     |
| >>PGR: needs to be enhanced (compare formalized pubid?)                      |
+-----------------------------------------------------------------------------*/
bool ExternId::operator==(ExternId const & source) const {
   if (
      (inqSystemId() == source.inqSystemId()) &&
      (inqPublicId() == source.inqPublicId())
   ) {
      return true;
   }else {
      return false;
   }
}

/*-----------------------------------------------------------ExternId::extract-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void ExternId::extract(Writer & out, SgmlDecl const & sdcl) const
{
   UnicodeString const & ucsSpace = sdcl.charset().inqUcSpace(); // = for MSVC

   if (inqPublicId().good()) {
      out << sdcl.reservedName()[ReservedName::IX_PUBLIC] << ucsSpace
          << inqDelimPublicId() << inqPublicId() << inqDelimPublicId();
      if (!inqSystemId()) return;
   }else {
      if (!inqSystemId()) return;
      out << sdcl.reservedName()[ReservedName::IX_SYSTEM];
   }
   out << ucsSpace << inqDelimSystemId() << inqSystemId() << inqDelimSystemId();
}

/*============================================================================*/
