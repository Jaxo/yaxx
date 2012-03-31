/*
* $Id: sdcl.h,v 1.6 2002-04-14 23:26:04 jlatone Exp $
*
* SgmlDecl class is mainly a container class for all components of
* the SGML declaration.
*/

#if !defined SDCL_HEADER && defined __cplusplus
#define SDCL_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "charset.h"
#include "rsvdname.h"
#include "features.h"
#include "quantity.h"
#include "dlmlist.h"

class EntityManager;

/*-------------------------------------------------------------------SgmlDecl-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API SgmlDecl {
public:
   enum SyntaxFlavour {
      SGML = 0,                           // Default
      XML,
      HTML,
      NO_ENTREF = 1 << 3,
      SGML_WITH_NO_ENTREF = SGML | NO_ENTREF,
      HTML_WITH_NO_ENTREF = HTML | NO_ENTREF,
      XML_WITH_NO_ENTREF = XML | NO_ENTREF
   };

   SgmlDecl(SyntaxFlavour flv);           // Reference Concrete Syntax
   SgmlDecl(EntityManager & em);          // Syntax Parsed thru em

   bool isXML() const;
   bool isSGML() const;
   bool isHTML() const;
   bool recognizeEntRef() const;

   Charset const & charset() const;
   ReservedName const & reservedName() const;
   Features const & features() const;
   Quantity const & quantity() const;
   DelimList const & delimList() const;

   void resetSyntax(SyntaxFlavour flv);

private:
   class UnrecognizedDelimiter {
   public:
      UnrecognizedDelimiter();
      UnicodeString makeUnrecognized();
   private:
      UCS_2  m_buf[2];
   };
   SyntaxFlavour m_flv;
   Charset chs;
   ReservedName rsvnm;
   Features ft;
   Quantity qnty;
   DelimList dlmlst;
};

/*-- INLINES --*/
inline bool SgmlDecl::isSGML() const {
   return ((m_flv & ~NO_ENTREF) == SGML);
}
inline bool SgmlDecl::isXML() const {
   return ((m_flv & ~NO_ENTREF) == XML);
}
inline bool SgmlDecl::isHTML() const {
   return ((m_flv & ~NO_ENTREF) == HTML);
}
inline bool SgmlDecl::recognizeEntRef() const {
   return (!(m_flv & NO_ENTREF));
}
inline Charset const & SgmlDecl::charset() const {
   return chs;
}
inline ReservedName const & SgmlDecl::reservedName() const {
   return rsvnm;
}
inline Features const & SgmlDecl::features() const {
   return ft;
}
inline Quantity const & SgmlDecl::quantity() const {
   return qnty;
}
inline DelimList const & SgmlDecl::delimList() const {
   return dlmlst;
}

#endif /* SDCL_HEADER =======================================================*/
