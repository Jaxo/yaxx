/* $Id: sdcl.cpp,v 1.1 2002-02-26 11:34:58 pgr Exp $ */

#include "sdcl.h"

/*---------------------SgmlDecl::UnrecognizedDelimiter::UnrecognizedDelimiter-+
|                                                                             |
+----------------------------------------------------------------------------*/
SgmlDecl::UnrecognizedDelimiter::UnrecognizedDelimiter() {
   m_buf[0] = 0xFFFF;      // Unicode NAC (Not A Character)
   m_buf[1] = '0' - 1;
}

/*--------------------------SgmlDecl::UnrecognizedDelimiter::makeUnrecognized-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString SgmlDecl::UnrecognizedDelimiter::makeUnrecognized() {
   ++m_buf[1];
   return m_buf;
}

/*---------------------------------------------------------SgmlDecl::SgmlDecl-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
SgmlDecl::SgmlDecl(SyntaxFlavour flv) : dlmlst(chs) {
   resetSyntax(flv);
}

/*--------------------------------------------------------SgmlDeclresetSyntax-+
|                                                                             |
+----------------------------------------------------------------------------*/
void SgmlDecl::resetSyntax(SyntaxFlavour flv)
{
   UnrecognizedDelimiter fact;
   m_flv = flv;

   switch (flv) {
   case XML:
      ft.set(ReservedName::IX_EMPTYNRM, 1);
      dlmlst.replace(Delimiter::IX_CRO, "&#");
      dlmlst.replace(Delimiter::IX_ERO, "&");
      dlmlst.replace(Delimiter::IX_HCRO, "&#x");
      dlmlst.replace(Delimiter::IX_NESTC, "/");
      dlmlst.replace(Delimiter::IX_NET, ">");
      dlmlst.replace(Delimiter::IX_PIC, "?>");
      dlmlst.suppress_shortrefs();
      break;
   case XML_WITH_NO_ENTREF:
      ft.set(ReservedName::IX_EMPTYNRM, 1);
      dlmlst.replace(Delimiter::IX_CRO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_ERO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_HCRO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_NESTC, "/");
      dlmlst.replace(Delimiter::IX_NET, ">");
      dlmlst.replace(Delimiter::IX_PIC, "?>");
      dlmlst.suppress_shortrefs();
      break;
   case HTML:
      ft.set(ReservedName::IX_EMPTYNRM, 0);
      dlmlst.replace(Delimiter::IX_CRO, "&#");
      dlmlst.replace(Delimiter::IX_ERO, "&");
      dlmlst.replace(Delimiter::IX_HCRO, "&#x");
      dlmlst.replace(Delimiter::IX_NESTC, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_NET, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_PIC, "?>");
      dlmlst.suppress_shortrefs();
      break;
   case HTML_WITH_NO_ENTREF:
      ft.set(ReservedName::IX_EMPTYNRM, 0);
      dlmlst.replace(Delimiter::IX_CRO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_ERO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_HCRO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_NESTC, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_NET, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_PIC, ">");
      dlmlst.suppress_shortrefs();
      break;
   case SGML_WITH_NO_ENTREF:
      ft.set(ReservedName::IX_EMPTYNRM, 0);
      dlmlst.replace(Delimiter::IX_CRO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_ERO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_HCRO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_NESTC, "/");
      dlmlst.replace(Delimiter::IX_NET, "/");
      dlmlst.replace(Delimiter::IX_PIC, ">");
      dlmlst.suppress_shortrefs();
      break;
   default:   // case SGML:
      ft.set(ReservedName::IX_EMPTYNRM, 0);
      dlmlst.replace(Delimiter::IX_CRO, "&#");
      dlmlst.replace(Delimiter::IX_ERO, "&");
      dlmlst.replace(Delimiter::IX_HCRO, fact.makeUnrecognized());
      dlmlst.replace(Delimiter::IX_NESTC, "/");
      dlmlst.replace(Delimiter::IX_NET, "/");
      dlmlst.replace(Delimiter::IX_PIC, ">");
      dlmlst.suppress_shortrefs();
      break;
   }
}

/*===========================================================================*/
