/*
* $Id: dlmlist.cpp,v 1.3 2011-07-29 10:26:40 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include <string.h>
#include "features.h"
#include "charset.h"
#include "dlmlist.h"
#include "dlmbuild.h"

/*------------------+
| Local Definitions |
+------------------*/
enum e_BinaryRecMode {
   CON_MO     = 1 << MODE_CON,                        // With Shortrefs & NET
   CONB_MO    = (1 << MODE_CON_NOSR) | CON_MO,
   CONB2_MO   = (1 << MODE_MS_INCLUDE) | CONB_MO,
   CONB1_MO   = (1 << MODE_CON_XML_WNET) | CONB_MO,
   CONBB_MO   = (1 << MODE_CON_XML) | CONB1_MO | CONB2_MO,
   DS_MO      = 1 << MODE_DS,
   XDS_MO     = 1 << MODE_XDS,
   GRP_MO     = 1 << MODE_GRP,
   LITC_MO    = 1 << MODE_LITC,
   MD_MO      = 1 << MODE_MD,
   TAG_MO     = 1 << MODE_TAG,
   PI_MO      = 1 << MODE_PI,
   ASL_MO     = 1 << MODE_ASL,
   LITP_MO    = 1 << MODE_LITP,
   DCRCD_MO   = 1 << MODE_CON_RCDATA,
   DCFCD_MO   = 1 << MODE_CON_CDATA,
   MSRCD_MO   = 1 << MODE_MS_RCDATA,
   MSFCD_MO   = 1 << MODE_MS_CDATA,
   MSIGN_MO   = 1 << MODE_MS_IGNORE,
   INI_MO     = 1 << MODE_INI,
   COMMENT_MO = 1 << MODE_COMMENT,
   MDCOM_MO   = 1 << MODE_MD_COM,
   MDHCOM_MO  = 1 << MODE_MD_HCOM,
   BYMD_MO    = 1 << MODE_BYP_MD,
   BYGRP_MO   = 1 << MODE_BYP_GRP,
   BYDOD_MO   = 1 << MODE_BYP_DOCMD
};

/*-----------+
| Local Data |
+-----------*/
static UnicodeString const dAND(  '&'  );
static UnicodeString const dCOM(  "--" );
static UnicodeString const dCRO(  "&#" );
static UnicodeString const dDSC(  ']'  );
static UnicodeString const dDSO(  '['  );
static UnicodeString const dDTGC( ']'  );
static UnicodeString const dDTGO( '['  );
static UnicodeString const dERO(  '&'  );
static UnicodeString const dETAGO("</" );
static UnicodeString const dGRPC( ')'  );
static UnicodeString const dGRPO( '('  );
static UnicodeString const dHCRO;         // -- Web: &#x;
static UnicodeString const dLIT(  '\"' );
static UnicodeString const dLITA( '\'' );
static UnicodeString const dMDC(  '>'  );
static UnicodeString const dMDO(  "<!" );
static UnicodeString const dMINUS('-'  );
static UnicodeString const dMSC(  "]]" );
static UnicodeString const dNESTC('/'  ); // -- Web
static UnicodeString const dNET(  '/'  ); // -- Web: '>'
static UnicodeString const dOPT(  '?'  );
static UnicodeString const dOR(   '|'  );
static UnicodeString const dPERO( '%'  );
static UnicodeString const dPIC(  '>'  );
static UnicodeString const dPIO(  "<?" );
static UnicodeString const dPLUS( '+'  );
static UnicodeString const dREFC( ';'  );
static UnicodeString const dREP(  '*'  );
static UnicodeString const dRNI(  '#'  );
static UnicodeString const dSEQ(  ','  );
static UnicodeString const dSTAGO('<'  );
static UnicodeString const dTAGC( '>'  );
static UnicodeString const dVI(   '='  );

/*----------------------------------------------------------------------------+
| `aUcsDelimRefer' is a table of all (constant) delim string of the Ref CS;   |
| `aUcsShRefRefer' is a table of all (constant) short refs of the Ref CS.     |
+----------------------------------------------------------------------------*/
static UnicodeString const aUcsDelimRefer[] = {
   dAND  , dCOM  , dCRO  , dDSC  , dDSO  , dDTGC , dDTGO , dERO  ,
   dETAGO, dGRPC , dGRPO , dHCRO , dLIT  , dLITA , dMDC  , dMDO  ,
   dMINUS, dMSC  , dNESTC, dNET  , dOPT  , dOR   , dPERO , dPIC  ,
   dPIO  , dPLUS , dREFC , dREP  , dRNI  , dSEQ  , dSTAGO, dTAGC ,
   dVI
};
#define NBR_SDELIM (sizeof aUcsDelimRefer / sizeof aUcsDelimRefer[0])

static UnicodeString const aUcsShRefRefer[] = {
   (char*)0,(char*)0,(char*)0,(char*)0,(char*)0,(char*)0,(char*)0,(char*)0,
   "BB",    '\"',    '#',     '%',     '\'',    '(',     ')',     '*',
   '+',     ',',     '-',     "--",    ':',     ';',     '=',     '@',
   '[',     ']',     '^',     '_',     '{',     '|',     '}',     '~'
};
#define NBR_SHREFR (sizeof aUcsShRefRefer / sizeof aUcsShRefRefer[0])

/*----------------------------------------------------------------------------+
| `rcgnTable' describes in what mode a given delimiter is recognized          |
+----------------------------------------------------------------------------*/
static int const rcgnTable[] = {
   GRP_MO                                                     ,  // AND
   MD_MO | COMMENT_MO | MDCOM_MO                              ,  // COM
   CONBB_MO | LITC_MO | LITP_MO | DCRCD_MO | MSRCD_MO |INI_MO ,  // CRO   &#
   DS_MO   | ASL_MO                                           ,  // DSC
   MD_MO | BYDOD_MO                                           ,  // DSO
   GRP_MO                                                     ,  // DTGC
   GRP_MO                                                     ,  // DTGO
   CONBB_MO | LITC_MO | DCRCD_MO  | MSRCD_MO                  ,  // ERO   &
   CONBB_MO | TAG_MO  | DCRCD_MO  | DCFCD_MO                  ,  // ETAGO </
   GRP_MO  | BYGRP_MO                                         ,  // GRPC
   GRP_MO  | MD_MO   | BYGRP_MO                               ,  // GRPO
   CONBB_MO | LITC_MO | LITP_MO | DCRCD_MO | MSRCD_MO |INI_MO ,  // HCRO  &#x
   GRP_MO  | LITC_MO | LITP_MO | MD_MO | TAG_MO | ASL_MO      ,  // LIT
   GRP_MO  | LITC_MO | LITP_MO | MD_MO | TAG_MO | ASL_MO      ,  // LITA
   MD_MO   | BYMD_MO | BYGRP_MO | MDCOM_MO | BYDOD_MO         ,  // MDC
   CONBB_MO | DS_MO  | XDS_MO | INI_MO | BYDOD_MO             ,  // MDO   <!
   0                                                          ,  // MINUS
   0                                                          ,  // MSC
   TAG_MO                                                     ,  // NESTC /
   CONB1_MO | DCRCD_MO  | DCFCD_MO                            ,  // NET / or >
   GRP_MO                                                     ,  // OPT
   GRP_MO                                                     ,  // OR
   DS_MO   | XDS_MO | GRP_MO  | MD_MO   | LITP_MO             ,  // PERO
   PI_MO                                                      ,  // PIC
   CONBB_MO | DS_MO | XDS_MO  | INI_MO | BYDOD_MO             ,  // PIO   <?
   GRP_MO                                                     ,  // PLUS
   0                                                          ,  // REFC
   GRP_MO                                                     ,  // REP
   GRP_MO  | MD_MO                                            ,  // RNI
   GRP_MO                                                     ,  // SEQ
   CONBB_MO | TAG_MO  | INI_MO | BYDOD_MO                     ,  // STAGO <
   TAG_MO                                                     ,  // TAGC
   TAG_MO  | ASL_MO                                              // VI
};

/*----------------------------------------------------------------------------+
| `ddlmdef' defines the double-delimiters, as well as their recognition mode  |
+----------------------------------------------------------------------------*/
static const struct {  // same order as Delimiter::e_Ix!
   Delimiter::e_Ix ixdl1;
   Delimiter::e_Ix ixdl2;
   unsigned int iMode;
} ddlmdef[] = {
   {
      Delimiter::IX_ERO  ,
      Delimiter::IX_GRPO , CONBB_MO | LITC_MO | DCRCD_MO | MSRCD_MO
   }, {
      Delimiter::IX_ETAGO,
      Delimiter::IX_GRPO , CONBB_MO | TAG_MO | DCRCD_MO | DCFCD_MO
   }, {
      Delimiter::IX_ETAGO,
      Delimiter::IX_TAGC , CONBB_MO | TAG_MO | DCRCD_MO | DCFCD_MO
   }, {
      Delimiter::IX_MDO  ,
      Delimiter::IX_COM  , CONBB_MO | DS_MO | XDS_MO | INI_MO | BYDOD_MO
   }, {
      Delimiter::IX_MDO  ,
      Delimiter::IX_DSO  , CONBB_MO | DS_MO | XDS_MO | INI_MO | MSIGN_MO |
                           BYDOD_MO
   }, {
      Delimiter::IX_MDO  ,
      Delimiter::IX_MDC  , CONBB_MO | DS_MO  | XDS_MO | INI_MO | BYDOD_MO
   }, {
      Delimiter::IX_MINUS,
      Delimiter::IX_GRPO , MD_MO
   }, {
      Delimiter::IX_MSC  ,
      Delimiter::IX_MDC  , CONB2_MO | DS_MO | XDS_MO | BYDOD_MO |
                           MSRCD_MO | MSFCD_MO | MSIGN_MO
   }, {
      Delimiter::IX_PERO ,
      Delimiter::IX_GRPO , DS_MO | XDS_MO | GRP_MO | MD_MO | LITP_MO
   }, {
      Delimiter::IX_PLUS ,
      Delimiter::IX_GRPO , MD_MO
   }, {
      Delimiter::IX_STAGO,
      Delimiter::IX_GRPO , CONBB_MO | TAG_MO | BYDOD_MO  | INI_MO
   }, {
      Delimiter::IX_STAGO,
      Delimiter::IX_TAGC , CONBB_MO | TAG_MO | BYDOD_MO  | INI_MO
   }, {
      Delimiter::IX_COM,
      Delimiter::IX_MDC  , MDHCOM_MO
   }
};
#define NBR_DDELIM (sizeof ddlmdef / sizeof ddlmdef[0])

/*--------------------------------------------------------DelimList::rcsValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString const & DelimList::rcsValue(Delimiter::e_Ix ixdl)
{
   if (ixdl <= Delimiter::IX_VI) {
      return aUcsDelimRefer[ixdl];
   }else {
      return UnicodeString::Nil;
   }
}

/*-----------------------------------------------------------DelimList::clear-+
| Make an empty DelimList                                                     |
+----------------------------------------------------------------------------*/
inline void DelimList::clear()
{
   delete [] aUcsValue;
   aUcsValue = 0;
   iCountShortref = 0;
   iCountDelim = 0;
}

/*-------------------------------------------------------DelimList::DelimList-+
| Regular constructor                                                         |
+----------------------------------------------------------------------------*/
DelimList::DelimList(Charset const& chsArg) : chs(chsArg)
{
   iCountShortref = NBR_SHREFR;
   iCountDelim = NBR_SDELIM +  NBR_DDELIM + NBR_SHREFR;
   aUcsValue = new UnicodeString[iCountDelim];
   if (!aUcsValue) {
      clear();
      return;
   }

   unsigned int ix = 0;
   while (ix < NBR_SDELIM) {
      aUcsValue[ix] = aUcsDelimRefer[ix];
      ++ix;
   }
   while (ix < (NBR_SDELIM + NBR_DDELIM)) {
      if (!replace_double_delim(ix++)) {
         clear();
         return;
      }
   }
   /*
   | Warning: if SHORTREF SGMLREF is used,
   | then TAB *must* be a function character
   | this error needs to be caught later
   | (as we don't know what shortrefs are used so far)
   */
   aUcsValue[ix++] = UnicodeString(chs.inqUcTab());  // &#TAB;
   aUcsValue[ix++] = UnicodeString(chs.RE());        // &#RE;
   aUcsValue[ix++] = UnicodeString(chs.RS());        // &#RS;
   aUcsValue[ix++] =                                 // &#RS;B
      UnicodeString(chs.RS()) +
      UnicodeString(Charset::ucBSEQ);
   aUcsValue[ix++] =                                 // &#RS;&#RE;
      UnicodeString(chs.RS()) +
      UnicodeString(chs.RE());
   aUcsValue[ix++] =                                 // &#RS;B&#RE;
      UnicodeString(chs.RS()) +
      UnicodeString(Charset::ucBSEQ) +
      UnicodeString(chs.RE());
   aUcsValue[ix++] =                                 // B&#RE;
      UnicodeString(Charset::ucBSEQ) +
      UnicodeString(chs.RE());
   aUcsValue[ix++] = UnicodeString(chs.inqUcSpace()); // &#SPACE;

   while (ix < iCountDelim) {
      aUcsValue[ix] = aUcsShRefRefer[ix - (NBR_SDELIM + NBR_DDELIM)];
      ++ix;
   }
}

/*------------------------------------------------------DelimList::~DelimList-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
DelimList::~DelimList()
{
   clear();
}

/*----------------------------------------------DelimList::DelimList::replace-+
| Replace single delimiters, shortrefs excluded                               |
+----------------------------------------------------------------------------*/
bool DelimList::replace(Delimiter::e_Ix ixdl, UnicodeString const & ucsValue)
{
   if ((unsigned int)ixdl >= NBR_SDELIM) {
      return false;
   }
   aUcsValue[(unsigned int)ixdl] = ucsValue;
   for (unsigned int ix = NBR_SDELIM; ix < (NBR_SDELIM + NBR_DDELIM); ++ix) {
      if (
         (ddlmdef[ix-NBR_SDELIM].ixdl1 == ixdl) ||
         (ddlmdef[ix-NBR_SDELIM].ixdl2 == ixdl)
      ) {
         replace_double_delim(ix);
      }
   }
   return true;
}

/*PRIVATE-------------------------------------DelimList::replace_double_delim-+
| Replace double delimiters when a single delimiter changes                   |
+----------------------------------------------------------------------------*/
bool DelimList::replace_double_delim(int const iDlmcode)
{
   aUcsValue[iDlmcode] =
      aUcsValue[ddlmdef[iDlmcode - NBR_SDELIM].ixdl1] +
      aUcsValue[ddlmdef[iDlmcode - NBR_SDELIM].ixdl2]
   ;
   if (aUcsValue[iDlmcode].good()) {
      return true;
   }else {
      return false;
   }
}

/*------------------------------------------------------DelimList::reallocate-+
| Reallocate the array of Unicode Strings                                     |
+----------------------------------------------------------------------------*/
bool DelimList::reallocate(int iCount)
{
   UnicodeString * aNewUcsValue = new UnicodeString[iCount];
   if (!aNewUcsValue) {
      return false;
   }
   int iCountMax;
   if (iCount > (int)iCountDelim) {     // extending
      iCountMax = iCountDelim;
   }else {
      iCountMax = iCount;               // shrinking
   }
   for (int i=0; i < iCountMax; ++i) {
      aNewUcsValue[i] = aUcsValue[i];
   }
   delete [] aUcsValue;
   aUcsValue = aNewUcsValue;
   iCountDelim = iCount;
   return true;
}

/*----------------------------------------------DelimList::suppress_shortrefs-+
| Suppress all shortrefs (SHORTREF NONE)                                      |
+----------------------------------------------------------------------------*/
bool DelimList::suppress_shortrefs()
{
   if (!reallocate(iCountDelim - iCountShortref)) {
      return false;
   }
   iCountShortref = 0;
   return true;
}

/*----------------------------------------------------DelimList::add_shortref-+
| Add a new shortref                                                          |
+----------------------------------------------------------------------------*/
bool DelimList::add_shortref(UnicodeString const & ucsValue)
{
   if (!reallocate(iCountDelim+1)) {
      return false;
   }
   aUcsValue[iCountDelim-1] = ucsValue;
   ++iCountShortref;
   return true;
}

/*----------------------------------------------------------------DlmDscTable-+
| Constructor for a DlmDscTable                                               |
| This table is for temporary use by DelimBuilder (thru the Row cursor class) |
+----------------------------------------------------------------------------*/
DlmDscTable::DlmDscTable(
   DelimList const & dlmlist,
   Features const & ft,
   bool& isOk
) {
   int const iCountDelim = dlmlist.count();
   if (
      (!iCountDelim) ||
      (aDlmdsc = new Item[1+iCountDelim], !aDlmdsc)
   ) {
      iCountDlmDsc = 0;
      isOk = false;
      return;                      // memory shortage!
   }

   Item *pDlmdsc = aDlmdsc;
   new(pDlmdsc) Item((UCS_2)0, Delimiter::IX_NOT_FOUND, (unsigned int)~0);
   unsigned int ix = 0;

   while (ix < NBR_SDELIM) {
      if ((ix != Delimiter::IX_HCRO) || dlmlist[(Delimiter::e_Ix)ix].good()) {
         new(++pDlmdsc) Item(
            dlmlist[(Delimiter::e_Ix)ix], (Delimiter::e_Ix)ix, rcgnTable[ix]
         );
      }
      ++ix;
   }
   while (ix < (NBR_SDELIM + NBR_DDELIM)) {
      switch(ix) {
      case Delimiter::IX_STAGO_TAGC:
      case Delimiter::IX_ETAGO_TAGC:
         if (!ft.isShorttag()) {
            ++ix;
            continue;
         }
         break;
      case Delimiter::IX_STAGO_GRPO:
      case Delimiter::IX_ETAGO_GRPO:
         if (!ft.isConcur()) {
            ++ix;
            continue;
         }
         break;
      case Delimiter::IX_ERO_GRPO:
      case Delimiter::IX_PERO_GRPO:
         if (
            (!ft.isConcur()) && (!ft.isSimple())  &&
            (!ft.isExplicit()) && (!ft.isImplicit())
         ) {
            ++ix;
            continue;
         }
         break;
      default:
         break;
      }
      new(++pDlmdsc) Item(
         dlmlist[(Delimiter::e_Ix)ix],
         (Delimiter::e_Ix)ix,
         ddlmdef[ix-NBR_SDELIM].iMode
      );
      ++ix;
   }
   while (ix < (unsigned int)iCountDelim) {
      new(++pDlmdsc) Item(
         dlmlist[(Delimiter::e_Ix)ix], (Delimiter::e_Ix)ix, CON_MO
      );
      ++ix;
   }
   iCountDlmDsc =  1 + (pDlmdsc - aDlmdsc);
   sort();

   isOk = true;
}

/*---------------------------------------------------------------~DlmDscTable-+
| Destructor for a DlmDscTable                                                |
+----------------------------------------------------------------------------*/
DlmDscTable::~DlmDscTable()
{
   delete [] aDlmdsc;
}

/*-------------------------------------------------DlmDscTable::Item::compare-+
| Compare two delimiters descriptors.                                         |
| The B-sequence (for ShortRef's) will be greater than any other character.   |
|                                                                             |
| Note: Duplicates are caught in later by check_duplicate_delimiter()         |
+----------------------------------------------------------------------------*/
int DlmDscTable::Item::compare(DlmDscTable::Item const& itemAgainst)
{
   UCS_2 const* pUc1 = ucsDelim;
   UCS_2 const* pUc2 = itemAgainst.ucsDelim;

   while ((*pUc1 == *pUc2) && (*pUc1)) {       // until it differs or EOS
      ++pUc1, ++pUc2;
   }
   if ((ixdl >= (NBR_SDELIM + NBR_DDELIM)) && (*pUc1 == Charset::ucBSEQ)) {
      return 1;
   }else if (
      (itemAgainst.ixdl >= (NBR_SDELIM + NBR_DDELIM)) &&
      (*pUc2 == Charset::ucBSEQ)
   ) {
      return -1;
   }else {
      return *pUc1 - *pUc2;
   }
}

/*------------------------------------------------DlmDscTable::Item::exchange-+
| Exchange two delimiters descriptors.                                        |
+----------------------------------------------------------------------------*/
void DlmDscTable::Item::exchange(DlmDscTable::Item & itemAgainst)
{
   char tempo[sizeof itemAgainst];
   memcpy(tempo, &itemAgainst, sizeof tempo);
   memcpy(&itemAgainst, this, sizeof tempo);
   memcpy(this, tempo, sizeof tempo);
}

/*---------------------------------------------operator<<(ostream, DelimList)-+
| Output the delimiter list (debug only)                                      |
+----------------------------------------------------------------------------*/
#if defined DEBUG_DELIM || defined __DEBUG_ALLOC__
ostream & operator<<(ostream & out, DelimList const & dlmlst)
{
   unsigned int ix = 0;

   out << "Single delims" << endl;
   while (ix < NBR_SDELIM) {
      out << "  \"" << dlmlst[(Delimiter::e_Ix)ix] << "\" ("
          << ix << ')' << endl;
      ++ix;
   }
   out << "Double delims" << endl;
   while (ix < (NBR_SDELIM + NBR_DDELIM)) {
      out << "  \"" << dlmlst[(Delimiter::e_Ix)ix] << "\" ("
          << ix << ')' << endl;
      ++ix;
   }
   out << "Shortrefs" << endl;
   int iShortref = 0;
   while (iShortref < dlmlst.iCountShortref) {
      out << "  \"" << dlmlst[(Delimiter::e_Ix)ix] << "\" ("
          << iShortref << ')' << endl;
      ++ix, ++iShortref;
   }
   return out;
}
#endif /* DEBUG_DELIM || __DEBUG_ALLOC__ */

/*===========================================================================*/
