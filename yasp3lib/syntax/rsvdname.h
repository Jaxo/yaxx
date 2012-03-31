/*
* $Id: rsvdname.h,v 1.9 2002-04-24 15:22:03 jlatone Exp $
*
* All Reserved Names of ISO 8879
*/

#if !defined RSVDNAME_HEADER && defined __cplusplus
#define RSVDNAME_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "../../toolslib/ucstring.h"
#include "../parser/yspenum.h"
#include "quantity.h"
#include "delim.h"

/*---------------------------------------------------------------ReservedName-+
| This class describes all reserved names of ISO 8879                         |
+----------------------------------------------------------------------------*/
class YASP3_API ReservedName {      // rsvnm
public:
   /*-------------------+
   | Public Definitions |
   +-------------------*/
   enum e_Ix {
      IX_AND     , IX_ANY     , IX_APPINFO , IX_ASN1    , IX_ATTCAP  ,
      IX_ATTCHCAP, IX_ATTCNT  , IX_ATTLIST , IX_ATTSPLEN, IX_AVGRPCAP,
      IX_BASESET , IX_BSEQLEN , IX_CAPACITY, IX_CDATA   , IX_CHANGES ,
      IX_CHARSET , IX_COM     , IX_CONCUR  , IX_CONREF  , IX_CONTROLS,
      IX_CRO     , IX_CURRENT , IX_DATATAG , IX_DEFAULT , IX_DELIM   ,
      IX_DELIMLEN, IX_DESCSET , IX_DOCTYPE , IX_DOCUMENT, IX_DSC     ,
      IX_DSO     , IX_DTAGLEN , IX_DTD     , IX_DTEMPLEN, IX_DTGC    ,
      IX_DTGO    , IX_ELEMCAP , IX_ELEMENT , IX_ELEMENTS, IX_EMPTY   ,
      IX_EMPTYNRM, IX_ENDTAG  , IX_ENTCAP  , IX_ENTCHCAP, IX_ENTITIES,
      IX_ENTITY  , IX_ENTLVL  , IX_ERO     , IX_ETAGO   , IX_EXCLUDE ,
      IX_EXGRPCAP, IX_EXNMCAP , IX_EXPLICIT, IX_FEATURES, IX_FIXED   ,
      IX_FORMAL  , IX_FUNCHAR , IX_FUNCTION, IX_GENERAL , IX_GRPC    ,
      IX_GRPCAP  , IX_GRPCNT  , IX_GRPGTCNT, IX_GRPLVL  , IX_GRPO    ,
      IX_HCRO    , IX_ID      , IX_IDCAP   , IX_IDLINK  , IX_IDREF   ,
      IX_IDREFCAP, IX_IDREFS  , IX_IGNORE  , IX_IMPLICIT, IX_IMPLIED ,
      IX_INCLUDE , IX_INITIAL , IX_INSTANCE, IX_KEEPRSRE, IX_LCNMCHAR,
      IX_LCNMSTRT, IX_LINK    , IX_LINKTYPE, IX_LIT     , IX_LITA    ,
      IX_LITLEN  , IX_LKNMCAP , IX_LKSETCAP, IX_LPD     , IX_MAPCAP  ,
      IX_MD      , IX_MDC     , IX_MDO     , IX_MINIMIZE, IX_MINUS   ,
      IX_MODEL   , IX_MS      , IX_MSC     , IX_MSICHAR , IX_MSOCHAR ,
      IX_MSSCHAR , IX_NAME    , IX_NAMECASE, IX_NAMELEN , IX_NAMES   ,
      IX_NAMING  , IX_NDATA   , IX_NESTC   , IX_NET     , IX_NMTOKEN ,
      IX_NMTOKENS, IX_NO      , IX_NONE    , IX_NONSGML , IX_NORMSEP ,
      IX_NOTATION, IX_NOTCAP  , IX_NOTCHCAP, IX_NUMBER  , IX_NUMBERS ,
      IX_NUTOKEN , IX_NUTOKENS, IX_O       , IX_OMITTAG , IX_OPT     ,
      IX_OR      , IX_OTHER   , IX_PACK    , IX_PCDATA  , IX_PERO    ,
      IX_PI      , IX_PIC     , IX_PILEN   , IX_PIO     , IX_PLUS    ,
      IX_POSTLINK, IX_PUBLIC  , IX_QUANTITY, IX_RANK    , IX_RCDATA  ,
      IX_RE      , IX_REFC    , IX_REP     , IX_REQUIRED, IX_RESTORE ,
      IX_RNI     , IX_RS      , IX_SCOPE   , IX_SDATA   , IX_SDIF    ,
      IX_SEPCHAR , IX_SEQ     , IX_SEQUENCE, IX_SGML    , IX_SGMLREF ,
      IX_SHORTREF, IX_SHORTTAG, IX_SHUNCHAR, IX_SIMPLE  , IX_SPACE   ,
      IX_SRCNT   , IX_SRLEN   , IX_STAGO   , IX_STARTTAG, IX_SUBDOC  ,
      IX_SWITCHES, IX_SYNTAX  , IX_SYSTEM  , IX_TAB     , IX_TAGC    ,
      IX_TAGLEN  , IX_TAGLVL  , IX_TEMP    , IX_TEXT    , IX_TOTALCAP,
      IX_UCNMCHAR, IX_UCNMSTRT, IX_UNPACK  , IX_UNUSED  , IX_USELINK ,
      IX_USEMAP  , IX_VALIDATE, IX_VI      , IX_YES     ,
      IX_NOT_FOUND, MAX_SIZE = IX_NOT_FOUND  // must be last entries
   };
   enum e_ReplaceCode {
      OK,
      NOT_A_RESERVED_NAME,
      SGML_DECL_NOT_REPLACEABLE,  // cannot replace: name belongs to SGML dcl
      SYST_DECL_NOT_REPLACEABLE,  // cannot replace: name belongs to SYSTEM dcl
      ALREADY_REPLACED,
      REPLACEMENT_NAME_IS_KEYWORD,
      REPLACEMENT_NAME_IS_IN_USE, // and ixReplaced tells what it replaces
      NO_MEMORY                   // memory shortage
   };
   class ChoiceList;
   class Choice {
      friend class ChoiceList;
   public:
      Choice(e_Ix ixArg) : ix(ixArg), iValue(ixArg) {};
      Choice(e_Ix ixArg, int iValueArg) : ix(ixArg), iValue(iValueArg) {};
   private:
      e_Ix ix;
      int iValue;
   };
   class ChoiceList {
   public:
      ChoiceList(
         ReservedName const & rsvdnameArg,
         Choice const * listArg
      );
      bool find(
         UCS_2 const * pUcToFind,
         int & iResult
      ) const;
      UnicodeString stringize(
         UnicodeString const & separator
      ) const;
   private:
      ReservedName const & rsvdname;
      Choice const * list;
   };

   /*-----------------+
   | Public Functions |
   +-----------------*/
   ReservedName();

   static UnicodeString const & rcsName(e_Ix ix);
   static UnicodeString const & rcsName(e_MsKey msk);

   UnicodeString const & operator[](e_Ix ix) const;
   UnicodeString const & operator[](Quantity::e_Ix ix) const;
   UnicodeString const & operator[](e_MsKey msk) const;
   UnicodeString operator[](Delimiter::e_Ix ix) const;

   e_ReplaceCode replace(
      UnicodeString const pszReplaced,
      UnicodeString const pszReplacement,
      e_Ix & ixReplaced
   );

private:
   #ifdef __MWERKS_NEEDS_STATICINIT_FIX__
   void aUcsRefer__static_init(void);
   #endif
   static e_Ix convert(e_MsKey msk);
   UnicodeString aUcsVarnt[MAX_SIZE+1];
};

/*--- INLINES ---*/
inline UnicodeString const & ReservedName::operator[](e_Ix ix) const {
   return aUcsVarnt[ix];
}
inline UnicodeString const & ReservedName::operator[](e_MsKey msk) const {
   return aUcsVarnt[convert(msk)];
}
inline ReservedName::ChoiceList::ChoiceList(
   ReservedName const & rsvdnameArg,
   ReservedName::Choice const * listArg
) :
   rsvdname(rsvdnameArg),
   list(listArg)
{
}
inline bool ReservedName::ChoiceList::find(
   UCS_2 const * pUcToFind,
   int & iResult
) const {
   bool isOk;
   UcStringFlash const ucsToFind(pUcToFind);
   ReservedName::Choice const * pChoice;
   for (pChoice = list; ; ++pChoice) {
      UnicodeString const & ucsFound = rsvdname[pChoice->ix];
      if (!ucsFound) {
         isOk = false;
         break;
      }
      if (ucsFound == ucsToFind) {
         isOk = true;
         break;
      }
   }
   iResult = pChoice->iValue;
   return isOk;
}

#endif /* RSVDNAME_HEADER ===================================================*/
