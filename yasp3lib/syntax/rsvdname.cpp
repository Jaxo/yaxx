/*
* $Id: rsvdname.cpp,v 1.6 2002-04-24 15:22:03 jlatone Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include "../../toolslib/Writer.h"
#include "rsvdname.h"

enum e_ReservedNameType {
   RSNTYPE_INVL,            // (Invalid)
   RSNTYPE_DLIM,            // Delimiter role (SGML declaration)
   RSNTYPE_FCLS,            // Function classes (SGML declaration)
   RSNTYPE_CAPA,            // Capacity name (SGML declaration)
   RSNTYPE_QNTY,            // Quantity name (SGML declaration)
   RSNTYPE_SGML,            // Others belonging to the SGML Declaration
   RSNTYPE_SYST,            // Belonging to the System declaration
   RSNTYPE_REPL             // All others (replaceable)
};

static UnicodeString const rAND(       "AND"         ); // RSNTYPE_DLIM
static UnicodeString const rANY(       "ANY"         ); // RSNTYPE_REPL
static UnicodeString const rAPPINFO(   "APPINFO"     ); // RSNTYPE_SGML
static UnicodeString const rASN1(      "ASN1"        ); // RSNTYPE_SYST
static UnicodeString const rATTCAP(    "ATTCAP"      ); // RSNTYPE_CAPA
static UnicodeString const rATTCHCAP(  "ATTCHCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rATTCNT(    "ATTCNT"      ); // RSNTYPE_QNTY
static UnicodeString const rATTLIST(   "ATTLIST"     ); // RSNTYPE_REPL
static UnicodeString const rATTSPLEN(  "ATTSPLEN"    ); // RSNTYPE_QNTY
static UnicodeString const rAVGRPCAP(  "AVGRPCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rBASESET(   "BASESET"     ); // RSNTYPE_SGML
static UnicodeString const rBSEQLEN(   "BSEQLEN"     ); // RSNTYPE_QNTY
static UnicodeString const rCAPACITY(  "CAPACITY"    ); // RSNTYPE_REPL
static UnicodeString const rCDATA(     "CDATA"       ); // RSNTYPE_REPL
static UnicodeString const rCHANGES(   "CHANGES"     ); // RSNTYPE_SYST
static UnicodeString const rCHARSET(   "CHARSET"     ); // RSNTYPE_REPL
static UnicodeString const rCOM(       "COM"         ); // RSNTYPE_DLIM
static UnicodeString const rCONCUR(    "CONCUR"      ); // RSNTYPE_SGML
static UnicodeString const rCONREF(    "CONREF"      ); // RSNTYPE_REPL
static UnicodeString const rCONTROLS(  "CONTROLS"    ); // RSNTYPE_SGML
static UnicodeString const rCRO(       "CRO"         ); // RSNTYPE_DLIM
static UnicodeString const rCURRENT(   "CURRENT"     ); // RSNTYPE_REPL
static UnicodeString const rDATATAG(   "DATATAG"     ); // RSNTYPE_SGML
static UnicodeString const rDEFAULT(   "DEFAULT"     ); // RSNTYPE_REPL
static UnicodeString const rDELIM(     "DELIM"       ); // RSNTYPE_SGML
static UnicodeString const rDELIMLEN(  "DELIMLEN"    ); // RSNTYPE_SYST
static UnicodeString const rDESCSET(   "DESCSET"     ); // RSNTYPE_SGML
static UnicodeString const rDOCTYPE(   "DOCTYPE"     ); // RSNTYPE_REPL
static UnicodeString const rDOCUMENT(  "DOCUMENT"    ); // RSNTYPE_REPL
static UnicodeString const rDSC(       "DSC"         ); // RSNTYPE_DLIM
static UnicodeString const rDSO(       "DSO"         ); // RSNTYPE_DLIM
static UnicodeString const rDTAGLEN(   "DTAGLEN"     ); // RSNTYPE_QNTY
static UnicodeString const rDTD(       "DTD"         ); // RSNTYPE_REPL
static UnicodeString const rDTEMPLEN(  "DTEMPLEN"    ); // RSNTYPE_QNTY
static UnicodeString const rDTGC(      "DTGC"        ); // RSNTYPE_DLIM
static UnicodeString const rDTGO(      "DTGO"        ); // RSNTYPE_DLIM
static UnicodeString const rELEMCAP(   "ELEMCAP"     ); // RSNTYPE_CAPA
static UnicodeString const rELEMENT(   "ELEMENT"     ); // RSNTYPE_REPL
static UnicodeString const rELEMENTS(  "ELEMENTS"    ); // RSNTYPE_REPL
static UnicodeString const rEMPTY(     "EMPTY"       ); // RSNTYPE_REPL
static UnicodeString const rEMPTYNRM(  "EMPTYNRM"    ); // RSNTYPE_SGML -- WEB
static UnicodeString const rENDTAG(    "ENDTAG"      ); // RSNTYPE_REPL
static UnicodeString const rENTCAP(    "ENTCAP"      ); // RSNTYPE_CAPA
static UnicodeString const rENTCHCAP(  "ENTCHCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rENTITIES(  "ENTITIES"    ); // RSNTYPE_REPL
static UnicodeString const rENTITY(    "ENTITY"      ); // RSNTYPE_REPL
static UnicodeString const rENTLVL(    "ENTLVL"      ); // RSNTYPE_QNTY
static UnicodeString const rERO(       "ERO"         ); // RSNTYPE_DLIM
static UnicodeString const rETAGO(     "ETAGO"       ); // RSNTYPE_DLIM
static UnicodeString const rEXCLUDE(   "EXCLUDE"     ); // RSNTYPE_SYST
static UnicodeString const rEXGRPCAP(  "EXGRPCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rEXNMCAP(   "EXNMCAP"     ); // RSNTYPE_CAPA
static UnicodeString const rEXPLICIT(  "EXPLICIT"    ); // RSNTYPE_SGML
static UnicodeString const rFEATURES(  "FEATURES"    ); // RSNTYPE_SGML
static UnicodeString const rFIXED(     "FIXED"       ); // RSNTYPE_REPL
static UnicodeString const rFORMAL(    "FORMAL"      ); // RSNTYPE_SGML
static UnicodeString const rFUNCHAR(   "FUNCHAR"     ); // RSNTYPE_FCLS
static UnicodeString const rFUNCTION(  "FUNCTION"    ); // RSNTYPE_SGML
static UnicodeString const rGENERAL(   "GENERAL"     ); // RSNTYPE_SGML
static UnicodeString const rGRPC(      "GRPC"        ); // RSNTYPE_DLIM
static UnicodeString const rGRPCAP(    "GRPCAP"      ); // RSNTYPE_CAPA
static UnicodeString const rGRPCNT(    "GRPCNT"      ); // RSNTYPE_QNTY
static UnicodeString const rGRPGTCNT(  "GRPGTCNT"    ); // RSNTYPE_QNTY
static UnicodeString const rGRPLVL(    "GRPLVL"      ); // RSNTYPE_QNTY
static UnicodeString const rGRPO(      "GRPO"        ); // RSNTYPE_DLIM
static UnicodeString const rHCRO(      "HCRO"        ); // RSNTYPE_DLIM -- WEB
static UnicodeString const rID(        "ID"          ); // RSNTYPE_REPL
static UnicodeString const rIDCAP(     "IDCAP"       ); // RSNTYPE_CAPA
static UnicodeString const rIDLINK(    "IDLINK"      ); // RSNTYPE_REPL
static UnicodeString const rIDREF(     "IDREF"       ); // RSNTYPE_REPL
static UnicodeString const rIDREFCAP(  "IDREFCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rIDREFS(    "IDREFS"      ); // RSNTYPE_REPL
static UnicodeString const rIGNORE(    "IGNORE"      ); // RSNTYPE_REPL
static UnicodeString const rIMPLICIT(  "IMPLICIT"    ); // RSNTYPE_SGML
static UnicodeString const rIMPLIED(   "IMPLIED"     ); // RSNTYPE_REPL
static UnicodeString const rINCLUDE(   "INCLUDE"     ); // RSNTYPE_REPL
static UnicodeString const rINITIAL(   "INITIAL"     ); // RSNTYPE_REPL
static UnicodeString const rINSTANCE(  "INSTANCE"    ); // RSNTYPE_SGML
static UnicodeString const rKEEPRSRE(  "KEEPRSE"     ); // RSNTYPE_SGML -- WEB
static UnicodeString const rLCNMCHAR(  "LCNMCHAR"    ); // RSNTYPE_SGML
static UnicodeString const rLCNMSTRT(  "LCNMSTRT"    ); // RSNTYPE_SGML
static UnicodeString const rLINK(      "LINK"        ); // RSNTYPE_REPL
static UnicodeString const rLINKTYPE(  "LINKTYPE"    ); // RSNTYPE_REPL
static UnicodeString const rLIT(       "LIT"         ); // RSNTYPE_DLIM
static UnicodeString const rLITA(      "LITA"        ); // RSNTYPE_DLIM
static UnicodeString const rLITLEN(    "LITLEN"      ); // RSNTYPE_QNTY
static UnicodeString const rLKNMCAP(   "LKNMCAP"     ); // RSNTYPE_CAPA
static UnicodeString const rLKSETCAP(  "LKSETCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rLPD(       "LPD"         ); // RSNTYPE_REPL
static UnicodeString const rMAPCAP(    "MAPCAP"      ); // RSNTYPE_CAPA
static UnicodeString const rMD(        "MD"          ); // RSNTYPE_REPL
static UnicodeString const rMDC(       "MDC"         ); // RSNTYPE_DLIM
static UnicodeString const rMDO(       "MDO"         ); // RSNTYPE_DLIM
static UnicodeString const rMINIMIZE(  "MINIMIZE"    ); // RSNTYPE_SGML
static UnicodeString const rMINUS(     "MINUS"       ); // RSNTYPE_DLIM
static UnicodeString const rMODEL(     "MODEL"       ); // RSNTYPE_SYST
static UnicodeString const rMS(        "MS"          ); // RSNTYPE_REPL
static UnicodeString const rMSC(       "MSC"         ); // RSNTYPE_DLIM
static UnicodeString const rMSICHAR(   "MSICHAR"     ); // RSNTYPE_FCLS
static UnicodeString const rMSOCHAR(   "MSOCHAR"     ); // RSNTYPE_FCLS
static UnicodeString const rMSSCHAR(   "MSSCHAR"     ); // RSNTYPE_FCLS
static UnicodeString const rNAME(      "NAME"        ); // RSNTYPE_REPL
static UnicodeString const rNAMECASE(  "NAMECASE"    ); // RSNTYPE_SGML
static UnicodeString const rNAMELEN(   "NAMELEN"     ); // RSNTYPE_QNTY
static UnicodeString const rNAMES(     "NAMES"       ); // RSNTYPE_REPL
static UnicodeString const rNAMING(    "NAMING"      ); // RSNTYPE_SGML
static UnicodeString const rNDATA(     "NDATA"       ); // RSNTYPE_REPL
static UnicodeString const rNESTC(     "NESTC"       ); // RSNTYPE_DLIM -- WEB
static UnicodeString const rNET(       "NET"         ); // RSNTYPE_DLIM
static UnicodeString const rNMTOKEN(   "NMTOKEN"     ); // RSNTYPE_REPL
static UnicodeString const rNMTOKENS(  "NMTOKENS"    ); // RSNTYPE_REPL
static UnicodeString const rNO(        "NO"          ); // RSNTYPE_SGML
static UnicodeString const rNONE(      "NONE"        ); // RSNTYPE_SGML
static UnicodeString const rNONSGML(   "NONSGML"     ); // RSNTYPE_REPL
static UnicodeString const rNORMSEP(   "NORMSEP"     ); // RSNTYPE_QNTY
static UnicodeString const rNOTATION(  "NOTATION"    ); // RSNTYPE_REPL
static UnicodeString const rNOTCAP(    "NOTCAP"      ); // RSNTYPE_CAPA
static UnicodeString const rNOTCHCAP(  "NOTCHCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rNUMBER(    "NUMBER"      ); // RSNTYPE_REPL
static UnicodeString const rNUMBERS(   "NUMBERS"     ); // RSNTYPE_REPL
static UnicodeString const rNUTOKEN(   "NUTOKEN"     ); // RSNTYPE_REPL
static UnicodeString const rNUTOKENS(  "NUTOKENS"    ); // RSNTYPE_REPL
static UnicodeString const rO(         "O"           ); // RSNTYPE_REPL
static UnicodeString const rOMITTAG(   "OMITTAG"     ); // RSNTYPE_SGML
static UnicodeString const rOPT(       "OPT"         ); // RSNTYPE_DLIM
static UnicodeString const rOR(        "OR"          ); // RSNTYPE_DLIM
static UnicodeString const rOTHER(     "OTHER"       ); // RSNTYPE_SGML
static UnicodeString const rPACK(      "PACK"        ); // RSNTYPE_SYST
static UnicodeString const rPCDATA(    "PCDATA"      ); // RSNTYPE_REPL
static UnicodeString const rPERO(      "PERO"        ); // RSNTYPE_DLIM
static UnicodeString const rPI(        "PI"          ); // RSNTYPE_REPL
static UnicodeString const rPIC(       "PIC"         ); // RSNTYPE_DLIM
static UnicodeString const rPILEN(     "PILEN"       ); // RSNTYPE_QNTY
static UnicodeString const rPIO(       "PIO"         ); // RSNTYPE_DLIM
static UnicodeString const rPLUS(      "PLUS"        ); // RSNTYPE_DLIM
static UnicodeString const rPOSTLINK(  "POSTLINK"    ); // RSNTYPE_REPL
static UnicodeString const rPUBLIC(    "PUBLIC"      ); // RSNTYPE_REPL
static UnicodeString const rQUANTITY(  "QUANTITY"    ); // RSNTYPE_SGML
static UnicodeString const rRANK(      "RANK"        ); // RSNTYPE_SGML
static UnicodeString const rRCDATA(    "RCDATA"      ); // RSNTYPE_REPL
static UnicodeString const rRE(        "RE"          ); // RSNTYPE_REPL
static UnicodeString const rREFC(      "REFC"        ); // RSNTYPE_DLIM
static UnicodeString const rREP(       "REP"         ); // RSNTYPE_DLIM
static UnicodeString const rREQUIRED(  "REQUIRED"    ); // RSNTYPE_REPL
static UnicodeString const rRESTORE(   "RESTORE"     ); // RSNTYPE_REPL
static UnicodeString const rRNI(       "RNI"         ); // RSNTYPE_DLIM
static UnicodeString const rRS(        "RS"          ); // RSNTYPE_REPL
static UnicodeString const rSCOPE(     "SCOPE"       ); // RSNTYPE_SGML
static UnicodeString const rSDATA(     "SDATA"       ); // RSNTYPE_REPL
static UnicodeString const rSDIF(      "SDIF"        ); // RSNTYPE_SYST
static UnicodeString const rSEPCHAR(   "SEPCHAR"     ); // RSNTYPE_FCLS
static UnicodeString const rSEQ(       "SEQ"         ); // RSNTYPE_DLIM
static UnicodeString const rSEQUENCE(  "SEQUENCE"    ); // RSNTYPE_SYST
static UnicodeString const rSGML(      "SGML"        ); // RSNTYPE_SGML
static UnicodeString const rSGMLREF(   "SGMLREF"     ); // RSNTYPE_SGML
static UnicodeString const rSHORTREF(  "SHORTREF"    ); // RSNTYPE_REPL
static UnicodeString const rSHORTTAG(  "SHORTTAG"    ); // RSNTYPE_SGML
static UnicodeString const rSHUNCHAR(  "SHUNCHAR"    ); // RSNTYPE_SGML
static UnicodeString const rSIMPLE(    "SIMPLE"      ); // RSNTYPE_REPL
static UnicodeString const rSPACE(     "SPACE"       ); // RSNTYPE_REPL
static UnicodeString const rSRCNT(     "SRCNT"       ); // RSNTYPE_SYST
static UnicodeString const rSRLEN(     "SRLEN"       ); // RSNTYPE_SYST
static UnicodeString const rSTAGO(     "STAGO"       ); // RSNTYPE_DLIM
static UnicodeString const rSTARTTAG(  "STARTTAG"    ); // RSNTYPE_REPL
static UnicodeString const rSUBDOC(    "SUBDOC"      ); // RSNTYPE_REPL
static UnicodeString const rSWITCHES(  "SWITCHES"    ); // RSNTYPE_SGML
static UnicodeString const rSYNTAX(    "SYNTAX"      ); // RSNTYPE_REPL
static UnicodeString const rSYSTEM(    "SYSTEM"      ); // RSNTYPE_REPL
static UnicodeString const rTAB(       "TAB"         ); // RSNTYPE_SGML
static UnicodeString const rTAGC(      "TAGC"        ); // RSNTYPE_DLIM
static UnicodeString const rTAGLEN(    "TAGLEN"      ); // RSNTYPE_QNTY
static UnicodeString const rTAGLVL(    "TAGLVL"      ); // RSNTYPE_QNTY
static UnicodeString const rTEMP(      "TEMP"        ); // RSNTYPE_REPL
static UnicodeString const rTEXT(      "TEXT"        ); // RSNTYPE_REPL
static UnicodeString const rTOTALCAP(  "TOTALCAP"    ); // RSNTYPE_CAPA
static UnicodeString const rUCNMCHAR(  "UCNMCHAR"    ); // RSNTYPE_SGML
static UnicodeString const rUCNMSTRT(  "UCNMSTRT"    ); // RSNTYPE_SGML
static UnicodeString const rUNPACK(    "UNPACK"      ); // RSNTYPE_SYST
static UnicodeString const rUNUSED(    "UNUSED"      ); // RSNTYPE_SGML
static UnicodeString const rUSELINK(   "USELINK"     ); // RSNTYPE_REPL
static UnicodeString const rUSEMAP(    "USEMAP"      ); // RSNTYPE_REPL
static UnicodeString const rVALIDATE(  "VALIDATE"    ); // RSNTYPE_SYST
static UnicodeString const rVI(        "VI"          ); // RSNTYPE_DLIM
static UnicodeString const rYES(       "YES"         ); // RSNTYPE_SGML

/*----------------------------------------------------------------------------+
| `aUcsRefer' is a table of all (constant) reserved names of the Reference,   |
| including the ones found in the System Declaration.                         |
| Indeces to `aUcsRefer' are found in the `ReservedName::Ix' enum             |
+----------------------------------------------------------------------------*/
#ifdef __MWERKS_NEEDS_STATICINIT_FIX1__
#pragma stack_cleanup on
#endif
static UnicodeString aUcsRefer[ReservedName::MAX_SIZE+1]

#ifdef __MWERKS_NEEDS_STATICINIT_FIX2__

// Hard-line work-around for CW static array init code 
// running out of stack space.

;  // end of declaration of aUcsRefer

void ReservedName::aUcsRefer__static_init(void) {
   aUcsRefer[IX_AND		] = rAND     ;
   aUcsRefer[IX_ANY		] = rANY     ;
   aUcsRefer[IX_APPINFO	] = rAPPINFO ;
   aUcsRefer[IX_ASN1	] = rASN1    ;
   aUcsRefer[IX_ATTCAP	] = rATTCAP  ;
   aUcsRefer[IX_ATTCHCAP] = rATTCHCAP;
   aUcsRefer[IX_ATTCNT	] = rATTCNT  ;
   aUcsRefer[IX_ATTLIST	] = rATTLIST ;
   aUcsRefer[IX_ATTSPLEN] = rATTSPLEN;
   aUcsRefer[IX_AVGRPCAP] = rAVGRPCAP;
   aUcsRefer[IX_BASESET	] = rBASESET ;
   aUcsRefer[IX_BSEQLEN	] = rBSEQLEN ;
   aUcsRefer[IX_CAPACITY] = rCAPACITY;
   aUcsRefer[IX_CDATA	] = rCDATA   ;
   aUcsRefer[IX_CHANGES	] = rCHANGES ;
   aUcsRefer[IX_CHARSET	] = rCHARSET ;
   aUcsRefer[IX_COM		] = rCOM     ;
   aUcsRefer[IX_CONCUR	] = rCONCUR  ;
   aUcsRefer[IX_CONREF	] = rCONREF  ;
   aUcsRefer[IX_CONTROLS] = rCONTROLS;
   aUcsRefer[IX_CRO		] = rCRO     ;
   aUcsRefer[IX_CURRENT	] = rCURRENT ;
   aUcsRefer[IX_DATATAG	] = rDATATAG ;
   aUcsRefer[IX_DEFAULT	] = rDEFAULT ;
   aUcsRefer[IX_DELIM	] = rDELIM   ;
   aUcsRefer[IX_DELIMLEN] = rDELIMLEN;
   aUcsRefer[IX_DESCSET	] = rDESCSET ;
   aUcsRefer[IX_DOCTYPE	] = rDOCTYPE ;
   aUcsRefer[IX_DOCUMENT] = rDOCUMENT;
   aUcsRefer[IX_DSC		] = rDSC     ;
   aUcsRefer[IX_DSO		] = rDSO     ;
   aUcsRefer[IX_DTAGLEN	] = rDTAGLEN ;
   aUcsRefer[IX_DTD		] = rDTD     ;
   aUcsRefer[IX_DTEMPLEN] = rDTEMPLEN;
   aUcsRefer[IX_DTGC	] = rDTGC    ;
   aUcsRefer[IX_DTGO	] = rDTGO    ;
   aUcsRefer[IX_ELEMCAP	] = rELEMCAP ;
   aUcsRefer[IX_ELEMENT	] = rELEMENT ;
   aUcsRefer[IX_ELEMENTS] = rELEMENTS;
   aUcsRefer[IX_EMPTY	] = rEMPTY   ;
   aUcsRefer[IX_EMPTYNRM] = rEMPTYNRM;
   aUcsRefer[IX_ENDTAG	] = rENDTAG  ;
   aUcsRefer[IX_ENTCAP	] = rENTCAP  ;
   aUcsRefer[IX_ENTCHCAP] = rENTCHCAP;
   aUcsRefer[IX_ENTITIES] = rENTITIES;
   aUcsRefer[IX_ENTITY	] = rENTITY  ;
   aUcsRefer[IX_ENTLVL	] = rENTLVL  ;
   aUcsRefer[IX_ERO		] = rERO     ;
   aUcsRefer[IX_ETAGO	] = rETAGO   ;
   aUcsRefer[IX_EXCLUDE	] = rEXCLUDE ;
   aUcsRefer[IX_EXGRPCAP] = rEXGRPCAP;
   aUcsRefer[IX_EXNMCAP	] = rEXNMCAP ;
   aUcsRefer[IX_EXPLICIT] = rEXPLICIT;
   aUcsRefer[IX_FEATURES] = rFEATURES;
   aUcsRefer[IX_FIXED	] = rFIXED   ;
   aUcsRefer[IX_FORMAL	] = rFORMAL  ;
   aUcsRefer[IX_FUNCHAR	] = rFUNCHAR ;
   aUcsRefer[IX_FUNCTION] = rFUNCTION;
   aUcsRefer[IX_GENERAL	] = rGENERAL ;
   aUcsRefer[IX_GRPC	] = rGRPC    ;
   aUcsRefer[IX_GRPCAP	] = rGRPCAP  ;
   aUcsRefer[IX_GRPCNT	] = rGRPCNT  ;
   aUcsRefer[IX_GRPGTCNT] = rGRPGTCNT;
   aUcsRefer[IX_GRPLVL	] = rGRPLVL  ;
   aUcsRefer[IX_GRPO	] = rGRPO    ;
   aUcsRefer[IX_HCRO	] = rHCRO    ;
   aUcsRefer[IX_ID		] = rID      ;
   aUcsRefer[IX_IDCAP	] = rIDCAP   ;
   aUcsRefer[IX_IDLINK	] = rIDLINK  ;
   aUcsRefer[IX_IDREF	] = rIDREF   ;
   aUcsRefer[IX_IDREFCAP] = rIDREFCAP;
   aUcsRefer[IX_IDREFS	] = rIDREFS  ;
   aUcsRefer[IX_IGNORE	] = rIGNORE  ;
   aUcsRefer[IX_IMPLICIT] = rIMPLICIT;
   aUcsRefer[IX_IMPLIED	] = rIMPLIED ;
   aUcsRefer[IX_INCLUDE	] = rINCLUDE ;
   aUcsRefer[IX_INITIAL	] = rINITIAL ;
   aUcsRefer[IX_INSTANCE] = rINSTANCE;
   aUcsRefer[IX_KEEPRSRE] = rKEEPRSRE;
   aUcsRefer[IX_LCNMCHAR] = rLCNMCHAR;
   aUcsRefer[IX_LCNMSTRT] = rLCNMSTRT;
   aUcsRefer[IX_LINK	] = rLINK    ;
   aUcsRefer[IX_LINKTYPE] = rLINKTYPE;
   aUcsRefer[IX_LIT		] = rLIT     ;
   aUcsRefer[IX_LITA	] = rLITA    ;
   aUcsRefer[IX_LITLEN	] = rLITLEN  ;
   aUcsRefer[IX_LKNMCAP	] = rLKNMCAP ;
   aUcsRefer[IX_LKSETCAP] = rLKSETCAP;
   aUcsRefer[IX_LPD		] = rLPD     ;
   aUcsRefer[IX_MAPCAP	] = rMAPCAP  ;
   aUcsRefer[IX_MD		] = rMD      ;
   aUcsRefer[IX_MDC		] = rMDC     ;
   aUcsRefer[IX_MDO		] = rMDO     ;
   aUcsRefer[IX_MINIMIZE] = rMINIMIZE;
   aUcsRefer[IX_MINUS	] = rMINUS   ;
   aUcsRefer[IX_MODEL	] = rMODEL   ;
   aUcsRefer[IX_MS		] = rMS      ;
   aUcsRefer[IX_MSC		] = rMSC     ;
   aUcsRefer[IX_MSICHAR	] = rMSICHAR ;
   aUcsRefer[IX_MSOCHAR	] = rMSOCHAR ;
   aUcsRefer[IX_MSSCHAR	] = rMSSCHAR ;
   aUcsRefer[IX_NAME	] = rNAME    ;
   aUcsRefer[IX_NAMECASE] = rNAMECASE;
   aUcsRefer[IX_NAMELEN	] = rNAMELEN ;
   aUcsRefer[IX_NAMES	] = rNAMES   ;
   aUcsRefer[IX_NAMING	] = rNAMING  ;
   aUcsRefer[IX_NDATA	] = rNDATA   ;
   aUcsRefer[IX_NESTC	] = rNESTC   ;
   aUcsRefer[IX_NET		] = rNET     ;
   aUcsRefer[IX_NMTOKEN	] = rNMTOKEN ;
   aUcsRefer[IX_NMTOKENS] = rNMTOKENS;
   aUcsRefer[IX_NO		] = rNO      ;
   aUcsRefer[IX_NONE	] = rNONE    ;
   aUcsRefer[IX_NONSGML	] = rNONSGML ;
   aUcsRefer[IX_NORMSEP	] = rNORMSEP ;
   aUcsRefer[IX_NOTATION] = rNOTATION;
   aUcsRefer[IX_NOTCAP	] = rNOTCAP  ;
   aUcsRefer[IX_NOTCHCAP] = rNOTCHCAP;
   aUcsRefer[IX_NUMBER	] = rNUMBER  ;
   aUcsRefer[IX_NUMBERS	] = rNUMBERS ;
   aUcsRefer[IX_NUTOKEN	] = rNUTOKEN ;
   aUcsRefer[IX_NUTOKENS] = rNUTOKENS;
   aUcsRefer[IX_O		] = rO       ;
   aUcsRefer[IX_OMITTAG	] = rOMITTAG ;
   aUcsRefer[IX_OPT		] = rOPT     ;
   aUcsRefer[IX_OR		] = rOR      ;
   aUcsRefer[IX_OTHER	] = rOTHER   ;
   aUcsRefer[IX_PACK	] = rPACK    ;
   aUcsRefer[IX_PCDATA	] = rPCDATA  ;
   aUcsRefer[IX_PERO	] = rPERO    ;
   aUcsRefer[IX_PI		] = rPI      ;
   aUcsRefer[IX_PIC		] = rPIC     ;
   aUcsRefer[IX_PILEN	] = rPILEN   ;
   aUcsRefer[IX_PIO		] = rPIO     ;
   aUcsRefer[IX_PLUS	] = rPLUS    ;
   aUcsRefer[IX_POSTLINK] = rPOSTLINK;
   aUcsRefer[IX_PUBLIC	] = rPUBLIC  ;
   aUcsRefer[IX_QUANTITY] = rQUANTITY;
   aUcsRefer[IX_RANK	] = rRANK    ;
   aUcsRefer[IX_RCDATA	] = rRCDATA  ;
   aUcsRefer[IX_RE		] = rRE      ;
   aUcsRefer[IX_REFC	] = rREFC    ;
   aUcsRefer[IX_REP		] = rREP     ;
   aUcsRefer[IX_REQUIRED] = rREQUIRED;
   aUcsRefer[IX_RESTORE	] = rRESTORE ;
   aUcsRefer[IX_RNI		] = rRNI     ;
   aUcsRefer[IX_RS		] = rRS      ;
   aUcsRefer[IX_SCOPE	] = rSCOPE   ;
   aUcsRefer[IX_SDATA	] = rSDATA   ;
   aUcsRefer[IX_SDIF	] = rSDIF    ;
   aUcsRefer[IX_SEPCHAR	] = rSEPCHAR ;
   aUcsRefer[IX_SEQ		] = rSEQ     ;
   aUcsRefer[IX_SEQUENCE] = rSEQUENCE;
   aUcsRefer[IX_SGML	] = rSGML    ;
   aUcsRefer[IX_SGMLREF	] = rSGMLREF ;
   aUcsRefer[IX_SHORTREF] = rSHORTREF;
   aUcsRefer[IX_SHORTTAG] = rSHORTTAG;
   aUcsRefer[IX_SHUNCHAR] = rSHUNCHAR;
   aUcsRefer[IX_SIMPLE	] = rSIMPLE  ;
   aUcsRefer[IX_SPACE	] = rSPACE   ;
   aUcsRefer[IX_SRCNT	] = rSRCNT   ;
   aUcsRefer[IX_SRLEN	] = rSRLEN   ;
   aUcsRefer[IX_STAGO	] = rSTAGO   ;
   aUcsRefer[IX_STARTTAG] = rSTARTTAG;
   aUcsRefer[IX_SUBDOC	] = rSUBDOC  ;
   aUcsRefer[IX_SWITCHES] = rSWITCHES;
   aUcsRefer[IX_SYNTAX	] = rSYNTAX  ;
   aUcsRefer[IX_SYSTEM	] = rSYSTEM  ;
   aUcsRefer[IX_TAB		] = rTAB     ;
   aUcsRefer[IX_TAGC	] = rTAGC    ;
   aUcsRefer[IX_TAGLEN	] = rTAGLEN  ;
   aUcsRefer[IX_TAGLVL	] = rTAGLVL  ;
   aUcsRefer[IX_TEMP	] = rTEMP    ;
   aUcsRefer[IX_TEXT	] = rTEXT    ;
   aUcsRefer[IX_TOTALCAP] = rTOTALCAP;
   aUcsRefer[IX_UCNMCHAR] = rUCNMCHAR;
   aUcsRefer[IX_UCNMSTRT] = rUCNMSTRT;
   aUcsRefer[IX_UNPACK	] = rUNPACK  ;
   aUcsRefer[IX_UNUSED	] = rUNUSED  ;
   aUcsRefer[IX_USELINK	] = rUSELINK ;
   aUcsRefer[IX_USEMAP	] = rUSEMAP  ;
   aUcsRefer[IX_VALIDATE] = rVALIDATE;
   aUcsRefer[IX_VI		] = rVI      ;
   aUcsRefer[IX_YES		] = rYES     ; 
   aUcsRefer[IX_NOT_FOUND] = UnicodeString();
   return;
}
#else
= {
   rAND     , rANY     , rAPPINFO , rASN1    , rATTCAP  ,
   rATTCHCAP, rATTCNT  , rATTLIST , rATTSPLEN, rAVGRPCAP,
   rBASESET , rBSEQLEN , rCAPACITY, rCDATA   , rCHANGES ,
   rCHARSET , rCOM     , rCONCUR  , rCONREF  , rCONTROLS,
   rCRO     , rCURRENT , rDATATAG , rDEFAULT , rDELIM   ,
   rDELIMLEN, rDESCSET , rDOCTYPE , rDOCUMENT, rDSC     ,
   rDSO     , rDTAGLEN , rDTD     , rDTEMPLEN, rDTGC    ,
   rDTGO    , rELEMCAP , rELEMENT , rELEMENTS, rEMPTY   ,
   rEMPTYNRM, rENDTAG  , rENTCAP  , rENTCHCAP, rENTITIES,
   rENTITY  , rENTLVL  , rERO     , rETAGO   , rEXCLUDE ,
   rEXGRPCAP, rEXNMCAP , rEXPLICIT, rFEATURES, rFIXED   ,
   rFORMAL  , rFUNCHAR , rFUNCTION, rGENERAL , rGRPC    ,
   rGRPCAP  , rGRPCNT  , rGRPGTCNT, rGRPLVL  , rGRPO    ,
   rHCRO    , rID      , rIDCAP   , rIDLINK  , rIDREF   ,
   rIDREFCAP, rIDREFS  , rIGNORE  , rIMPLICIT, rIMPLIED ,
   rINCLUDE , rINITIAL , rINSTANCE, rKEEPRSRE, rLCNMCHAR,
   rLCNMSTRT, rLINK    , rLINKTYPE, rLIT     , rLITA    ,
   rLITLEN  , rLKNMCAP , rLKSETCAP, rLPD     , rMAPCAP  ,
   rMD      , rMDC     , rMDO     , rMINIMIZE, rMINUS   ,
   rMODEL   , rMS      , rMSC     , rMSICHAR , rMSOCHAR ,
   rMSSCHAR , rNAME    , rNAMECASE, rNAMELEN , rNAMES   ,
   rNAMING  , rNDATA   , rNESTC   , rNET     , rNMTOKEN ,
   rNMTOKENS, rNO      , rNONE    , rNONSGML , rNORMSEP ,
   rNOTATION, rNOTCAP  , rNOTCHCAP, rNUMBER  , rNUMBERS ,
   rNUTOKEN , rNUTOKENS, rO       , rOMITTAG , rOPT     ,
   rOR      , rOTHER   , rPACK    , rPCDATA  , rPERO    ,
   rPI      , rPIC     , rPILEN   , rPIO     , rPLUS    ,
   rPOSTLINK, rPUBLIC  , rQUANTITY, rRANK    , rRCDATA  ,
   rRE      , rREFC    , rREP     , rREQUIRED, rRESTORE ,
   rRNI     , rRS      , rSCOPE   , rSDATA   , rSDIF    ,
   rSEPCHAR , rSEQ     , rSEQUENCE, rSGML    , rSGMLREF ,
   rSHORTREF, rSHORTTAG, rSHUNCHAR, rSIMPLE  , rSPACE   ,
   rSRCNT   , rSRLEN   , rSTAGO   , rSTARTTAG, rSUBDOC  ,
   rSWITCHES, rSYNTAX  , rSYSTEM  , rTAB     , rTAGC    ,
   rTAGLEN  , rTAGLVL  , rTEMP    , rTEXT    , rTOTALCAP,
   rUCNMCHAR, rUCNMSTRT, rUNPACK  , rUNUSED  , rUSELINK ,
   rUSEMAP  , rVALIDATE, rVI      , rYES     ,
   UnicodeString()
};
#endif

/*----------------------------------------------------------------------------+
| `typeTable' is a table for the types of all reserved names                  |
| Indeces to `typeTable' are found in the `ReservedName::Ix' enum             |
+----------------------------------------------------------------------------*/
static e_ReservedNameType const typeTable[ReservedName::MAX_SIZE+1]  = {
   RSNTYPE_DLIM, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_SYST, RSNTYPE_CAPA,
   RSNTYPE_CAPA, RSNTYPE_QNTY, RSNTYPE_REPL, RSNTYPE_QNTY, RSNTYPE_CAPA,
   RSNTYPE_SGML, RSNTYPE_QNTY, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_SYST,
   RSNTYPE_REPL, RSNTYPE_DLIM, RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_SGML,
   RSNTYPE_DLIM, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_SGML,
   RSNTYPE_SYST, RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_DLIM,
   RSNTYPE_DLIM, RSNTYPE_QNTY, RSNTYPE_REPL, RSNTYPE_QNTY, RSNTYPE_DLIM,
   RSNTYPE_DLIM, RSNTYPE_CAPA, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_REPL,
   RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_CAPA, RSNTYPE_CAPA, RSNTYPE_REPL,
   RSNTYPE_REPL, RSNTYPE_QNTY, RSNTYPE_DLIM, RSNTYPE_DLIM, RSNTYPE_SYST,
   RSNTYPE_CAPA, RSNTYPE_CAPA, RSNTYPE_SGML, RSNTYPE_SGML, RSNTYPE_REPL,
   RSNTYPE_SGML, RSNTYPE_FCLS, RSNTYPE_SGML, RSNTYPE_SGML, RSNTYPE_DLIM,
   RSNTYPE_CAPA, RSNTYPE_QNTY, RSNTYPE_QNTY, RSNTYPE_QNTY, RSNTYPE_DLIM,
   RSNTYPE_DLIM, RSNTYPE_REPL, RSNTYPE_CAPA, RSNTYPE_REPL, RSNTYPE_REPL,
   RSNTYPE_CAPA, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_REPL,
   RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_SGML, RSNTYPE_SGML,
   RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_DLIM, RSNTYPE_DLIM,
   RSNTYPE_QNTY, RSNTYPE_CAPA, RSNTYPE_CAPA, RSNTYPE_REPL, RSNTYPE_CAPA,
   RSNTYPE_REPL, RSNTYPE_DLIM, RSNTYPE_DLIM, RSNTYPE_SGML, RSNTYPE_DLIM,
   RSNTYPE_SYST, RSNTYPE_REPL, RSNTYPE_DLIM, RSNTYPE_FCLS, RSNTYPE_FCLS,
   RSNTYPE_FCLS, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_QNTY, RSNTYPE_REPL,
   RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_DLIM, RSNTYPE_DLIM, RSNTYPE_REPL,
   RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_QNTY,
   RSNTYPE_REPL, RSNTYPE_CAPA, RSNTYPE_CAPA, RSNTYPE_REPL, RSNTYPE_REPL,
   RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_DLIM,
   RSNTYPE_DLIM, RSNTYPE_SGML, RSNTYPE_SYST, RSNTYPE_REPL, RSNTYPE_DLIM,
   RSNTYPE_REPL, RSNTYPE_DLIM, RSNTYPE_QNTY, RSNTYPE_DLIM, RSNTYPE_DLIM,
   RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_SGML, RSNTYPE_REPL,
   RSNTYPE_REPL, RSNTYPE_DLIM, RSNTYPE_DLIM, RSNTYPE_REPL, RSNTYPE_REPL,
   RSNTYPE_DLIM, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_SYST,
   RSNTYPE_FCLS, RSNTYPE_DLIM, RSNTYPE_SYST, RSNTYPE_SGML, RSNTYPE_SGML,
   RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_REPL,
   RSNTYPE_SYST, RSNTYPE_SYST, RSNTYPE_DLIM, RSNTYPE_REPL, RSNTYPE_REPL,
   RSNTYPE_SGML, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_SGML, RSNTYPE_DLIM,
   RSNTYPE_QNTY, RSNTYPE_QNTY, RSNTYPE_REPL, RSNTYPE_REPL, RSNTYPE_CAPA,
   RSNTYPE_SGML, RSNTYPE_SGML, RSNTYPE_SYST, RSNTYPE_SGML, RSNTYPE_REPL,
   RSNTYPE_REPL, RSNTYPE_SYST, RSNTYPE_DLIM, RSNTYPE_SGML,
   RSNTYPE_INVL
};

/*---------------------------------------------------------------ReservedName-+
| Constructor from scratch                                                    |
+----------------------------------------------------------------------------*/
ReservedName::ReservedName()
{
   #ifdef __MWERKS_NEEDS_STATICINIT_FIX2__
   aUcsRefer__static_init();
   #endif
   UnicodeString *pUcsVarnt = aUcsVarnt;
   const UnicodeString *pUcsRefer = aUcsRefer;
   int num = (sizeof aUcsRefer / sizeof aUcsRefer[0]);

   while (num--) {
       *pUcsVarnt++ = *pUcsRefer++;
   }
}

/*--------------------------------------------------------------------rcsName-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString const & ReservedName::rcsName(ReservedName::e_Ix ix) {
   return aUcsRefer[ix];
}
UnicodeString const & ReservedName::rcsName(e_MsKey msk) {
   return aUcsRefer[convert(msk)];
}

/*STATIC---------------------------------------------------------------search-+
| Performs a binary search in the aUcsRefer sorted array.                     |
| Returns the index or IX_NOT_FOUND if not found.                             |
+----------------------------------------------------------------------------*/
static ReservedName::e_Ix search(UnicodeString const & needle)
{
   UnicodeString const * haystack = aUcsRefer;
   int num = (sizeof aUcsRefer / sizeof aUcsRefer[0]);

   int low = 0;                    // low bound
   while (low <= num) {
      int mid = ((unsigned int)low + (unsigned int)num) / 2;
      int cond = needle.compare(haystack[mid]);
      if (cond < 0) {
         num = mid - 1;            // set low range
      }else if (cond > 0) {
         low = mid + 1;
      }else {                      // found
         return (ReservedName::e_Ix)mid;
      }
   }
   return ReservedName::IX_NOT_FOUND;
}

/*--------------------------------------------------------------------replace-+
| Replace a reserved name or tell why it's not possible                       |
+----------------------------------------------------------------------------*/
ReservedName::e_ReplaceCode ReservedName::replace(
   UnicodeString const ucsReplaced,
   UnicodeString const ucsReplacement,
   e_Ix& ixReplaced
)
{
   /*
   |  Check that the replaced is replaceable
   */
   ixReplaced = search(ucsReplaced);
   if (ixReplaced == IX_NOT_FOUND) {       // not found
      return NOT_A_RESERVED_NAME;
   }else if (typeTable[ixReplaced] != RSNTYPE_REPL) {
      if (typeTable[ixReplaced] == RSNTYPE_SYST) {
         return SYST_DECL_NOT_REPLACEABLE;
      }else {
         return SGML_DECL_NOT_REPLACEABLE;
      }
   }else if (!(aUcsVarnt[ixReplaced].isIdentical(aUcsRefer[ixReplaced]))) {
      return ALREADY_REPLACED;
   }

   /*
   |  Check that the replacement is not an existing keyword
   */
   {
      e_Ix ixReplacement = search(ucsReplacement);
      if (ixReplacement != IX_NOT_FOUND) {      // already a keyword
         return REPLACEMENT_NAME_IS_KEYWORD;
      }
   }
   /*
   |  Check the replacement name is not already in use
   */
   {
      UnicodeString *pUcsVarnt = aUcsVarnt;
      const UnicodeString *pUcsRefer = aUcsRefer;
      int num = (sizeof aUcsRefer / sizeof aUcsRefer[0]);
      while (num--) {
         if (*pUcsVarnt == *pUcsRefer) {      // same
            ixReplaced = (e_Ix)(pUcsVarnt - aUcsVarnt);
            return REPLACEMENT_NAME_IS_IN_USE;
         }
         ++pUcsVarnt, ++pUcsRefer;
      }
   }
   /*
   |  It's OK: replace
   */
   aUcsVarnt[ixReplaced] = ucsReplacement;
   return OK;
}

/*--------------------------------------------------------------------convert-+
| Convert a e_msKey to a ReservedName::e_iX                                   |
+----------------------------------------------------------------------------*/
ReservedName::e_Ix ReservedName::convert(e_MsKey key)
{
   switch (key) {
   case MSKEY_TEMP:
      return ReservedName::IX_TEMP;
   case MSKEY_INCLUDE:
      return ReservedName::IX_INCLUDE;
   case MSKEY_RCDATA:
      return ReservedName::IX_RCDATA;
   case MSKEY_CDATA:
      return ReservedName::IX_CDATA;
   case MSKEY_IGNORE:
      return ReservedName::IX_IGNORE;
   default:
      break;
   }
   return ReservedName::IX_NOT_FOUND;
}

/*-----------------------------------------------------------------operator[]-+
| Get the reserved name associated to an index in the quantity table.         |
+----------------------------------------------------------------------------*/
UnicodeString const & ReservedName::operator[](Quantity::e_Ix ix) const
{
   switch (ix) {
   case Quantity::IX_ATTCNT:   return aUcsVarnt[IX_ATTCNT];
   case Quantity::IX_ATTSPLEN: return aUcsVarnt[IX_ATTSPLEN];
   case Quantity::IX_BSEQLEN:  return aUcsVarnt[IX_BSEQLEN];
   case Quantity::IX_DTAGLEN:  return aUcsVarnt[IX_DTAGLEN];
   case Quantity::IX_DTEMPLEN: return aUcsVarnt[IX_DTEMPLEN];
   case Quantity::IX_ENTLVL:   return aUcsVarnt[IX_ENTLVL];
   case Quantity::IX_GRPCNT:   return aUcsVarnt[IX_GRPCNT];
   case Quantity::IX_GRPGTCNT: return aUcsVarnt[IX_GRPGTCNT];
   case Quantity::IX_GRPLVL:   return aUcsVarnt[IX_GRPLVL];
   case Quantity::IX_LITLEN:   return aUcsVarnt[IX_LITLEN];
   case Quantity::IX_NAMELEN:  return aUcsVarnt[IX_NAMELEN];
   case Quantity::IX_NORMSEP:  return aUcsVarnt[IX_NORMSEP];
   case Quantity::IX_PILEN:    return aUcsVarnt[IX_PILEN];
   case Quantity::IX_TAGLEN:   return aUcsVarnt[IX_TAGLEN];
   case Quantity::IX_TAGLVL:   return aUcsVarnt[IX_TAGLVL];
   default:                    return UnicodeString::Nil;
   }
}

/*-----------------------------------------------------------------operator[]-+
| Get the reserved name associated to an index in the delimiter list          |
| Note: returns a UnicodeString copy, versus a reference because some of      |
|       these values can be created here as a temporay UnicodeString          |
+----------------------------------------------------------------------------*/
UnicodeString ReservedName::operator[](Delimiter::e_Ix ix) const
{
   switch (ix) {
   case Delimiter::IX_AND:        return aUcsVarnt[IX_AND];
   case Delimiter::IX_COM:        return aUcsVarnt[IX_COM];
   case Delimiter::IX_CRO:        return aUcsVarnt[IX_CRO];
   case Delimiter::IX_DSC:        return aUcsVarnt[IX_DSC];
   case Delimiter::IX_DSO:        return aUcsVarnt[IX_DSO];
   case Delimiter::IX_DTGC:       return aUcsVarnt[IX_DTGC];
   case Delimiter::IX_DTGO:       return aUcsVarnt[IX_DTGO];
   case Delimiter::IX_ERO:        return aUcsVarnt[IX_ERO];
   case Delimiter::IX_ETAGO:      return aUcsVarnt[IX_ETAGO];
   case Delimiter::IX_GRPC:       return aUcsVarnt[IX_GRPC];
   case Delimiter::IX_GRPO:       return aUcsVarnt[IX_GRPO];
   case Delimiter::IX_HCRO:       return aUcsVarnt[IX_HCRO];
   case Delimiter::IX_LIT:        return aUcsVarnt[IX_LIT];
   case Delimiter::IX_LITA:       return aUcsVarnt[IX_LITA];
   case Delimiter::IX_MDC:        return aUcsVarnt[IX_MDC];
   case Delimiter::IX_MDO:        return aUcsVarnt[IX_MDO];
   case Delimiter::IX_MINUS:      return aUcsVarnt[IX_MINUS];
   case Delimiter::IX_MSC:        return aUcsVarnt[IX_MSC];
   case Delimiter::IX_NESTC:      return aUcsVarnt[IX_NESTC];
   case Delimiter::IX_NET:        return aUcsVarnt[IX_NET];
   case Delimiter::IX_OPT:        return aUcsVarnt[IX_OPT];
   case Delimiter::IX_OR:         return aUcsVarnt[IX_OR];
   case Delimiter::IX_PERO:       return aUcsVarnt[IX_PERO];
   case Delimiter::IX_PIC:        return aUcsVarnt[IX_PIC];
   case Delimiter::IX_PIO:        return aUcsVarnt[IX_PIO];
   case Delimiter::IX_PLUS:       return aUcsVarnt[IX_PLUS];
   case Delimiter::IX_REFC:       return aUcsVarnt[IX_REFC];
   case Delimiter::IX_REP:        return aUcsVarnt[IX_REP];
   case Delimiter::IX_RNI:        return aUcsVarnt[IX_RNI];
   case Delimiter::IX_SEQ:        return aUcsVarnt[IX_SEQ];
   case Delimiter::IX_STAGO:      return aUcsVarnt[IX_STAGO];
   case Delimiter::IX_TAGC:       return aUcsVarnt[IX_TAGC];
   case Delimiter::IX_VI:         return aUcsVarnt[IX_VI];
   case Delimiter::IX_ERO_GRPO:
      return aUcsVarnt[IX_ERO] + aUcsVarnt[IX_GRPO];
   case Delimiter::IX_ETAGO_GRPO:
      return aUcsVarnt[IX_ETAGO] + aUcsVarnt[IX_GRPO];
   case Delimiter::IX_ETAGO_TAGC:
      return aUcsVarnt[IX_ETAGO] + aUcsVarnt[IX_TAGC];
   case Delimiter::IX_MDO_COM:
      return aUcsVarnt[IX_MDO] + aUcsVarnt[IX_COM];
   case Delimiter::IX_MDO_DSO:
      return aUcsVarnt[IX_MDO] + aUcsVarnt[IX_DSO];
   case Delimiter::IX_MDO_MDC:
      return aUcsVarnt[IX_MDO] + aUcsVarnt[IX_MDC];
   case Delimiter::IX_MINUS_GRPO:
      return aUcsVarnt[IX_MINUS] + aUcsVarnt[IX_GRPO];
   case Delimiter::IX_MSC_MDC:
      return aUcsVarnt[IX_MSC] + aUcsVarnt[IX_MDC];
   case Delimiter::IX_PERO_GRPO:
      return aUcsVarnt[IX_PERO] + aUcsVarnt[IX_GRPO];
   case Delimiter::IX_PLUS_GRPO:
      return aUcsVarnt[IX_PLUS] + aUcsVarnt[IX_GRPO];
   case Delimiter::IX_STAGO_GRPO:
      return aUcsVarnt[IX_STAGO] + aUcsVarnt[IX_GRPO];
   case Delimiter::IX_STAGO_TAGC:
      return aUcsVarnt[IX_STAGO] + aUcsVarnt[IX_TAGC];
   case Delimiter::IX_NOT_FOUND:
      return UnicodeString::Nil;
   default:
      return aUcsVarnt[IX_SHORTREF] +
             UnicodeString(" #") +
             UnicodeString(ix - Delimiter::IX_SHORTREF);
   }
}

/*----------------------------------------ReservedName::ChoiceList::stringize-+
| Get the list of reserved name out of a choice list.                         |
+----------------------------------------------------------------------------*/
UnicodeString ReservedName::ChoiceList::stringize(
   UnicodeString const & separator
) const {
   UnicodeMemWriter out;
   bool iFirst = true;
   for (ReservedName::Choice const * pChoice = list; ; ++pChoice) {
      UnicodeString const & ucsFound = rsvdname[pChoice->ix];
      if (!ucsFound) break;
      if (iFirst) {
         iFirst = false;
      }else {
         out << separator;
      }
      out << ucsFound;
   }
   return out;
}
/*===========================================================================*/
