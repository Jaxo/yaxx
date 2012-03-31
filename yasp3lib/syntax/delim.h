/*
* $Id: delim.h,v 1.1 2002-01-17 06:57:44 pgr Exp $
*
* Define the enum associated to the SGML delimiters
* as well as the recognition modes.
*/

#if !defined DELIM_HEADER && defined __cplusplus
#define DELIM_HEADER

/*----------------+
| Delimiter Codes |
+----------------*/
class Delimiter {
public:
   enum e_Ix {
      // ------ Single delimiters occur first
      IX_AND,             IX_COM,             IX_CRO,
      IX_DSC,             IX_DSO,             IX_DTGC,
      IX_DTGO,            IX_ERO,             IX_ETAGO,
      IX_GRPC,            IX_GRPO,            IX_HCRO,  /*Web*/
      IX_LIT,             IX_LITA,            IX_MDC,
      IX_MDO,             IX_MINUS,           IX_MSC,
      IX_NESTC, /*Web*/   IX_NET,             IX_OPT,
      IX_OR,              IX_PERO,            IX_PIC,
      IX_PIO,             IX_PLUS,            IX_REFC,
      IX_REP,             IX_RNI,             IX_SEQ,
      IX_STAGO,           IX_TAGC,            IX_VI,
      // ------ Double delimiters (not part of ISO 8879) follow
      IX_ERO_GRPO,        IX_ETAGO_GRPO,      IX_ETAGO_TAGC,
      IX_MDO_COM,         IX_MDO_DSO,         IX_MDO_MDC,
      IX_MINUS_GRPO,      IX_MSC_MDC,         IX_PERO_GRPO,
      IX_PLUS_GRPO,       IX_STAGO_GRPO,      IX_STAGO_TAGC,
      IX_COM_MDC,     // special (HTML)
      // ------ Shortref delimiters end the list
      IX_SHORTREF,              // 1st ShortRef delimiter
      /*...*/                   // (and so on...)
      // ------
      IX_NOT_FOUND = 5000       // Value needed to reserve enough shortrefs
   };
};

/*------------------+
| Recognition Modes |
+------------------*/
/*
| With history, this naming scheme becomes bad and confusing.
| Do not assume that XML means "not used in SGML".
|
| Mode          should be:   recognizes
|
| CON           CON_ALL      everything said in ISO, including shortrefs.
| CON_NOSR      CON          no shortrefs
| CON_XML       CON_LTAMP    '<' and '&' recognized.  Mostly in this mode.
| CON_XML_WNET  CON_LTAMPNET '<', '&' and '/' recognized.
| MS_INCLUDE    (ok)         '<', '&', ']]>'  recognized.
|
| Also, note that the external world only knows about
| CON (unused), CON_XML, CON_RCDATA, CON_CDATA.
| The others CON mode are "private"
|
*/
enum e_RecognitionMode {
   MODE_CON,        // As indicated in ISO 8879, 9.6.1
   MODE_CON_NOSR,   // CON mode without shortrefs
   MODE_CON_XML,    // CON mode without NET, nor shortrefs, nor "]]>"
   MODE_CON_XML_WNET, // CON mode with NET, bit no shortrefs, nor "]]>"
   MODE_CON_RCDATA, // replaceable character data (RCDATA) in declared content
   MODE_CON_CDATA,  // fixed character data (CDATA) in declared content
   MODE_MS_RCDATA,  // replaceable character data in Marked Section (RCDATA MS)
   MODE_MS_CDATA,   // fixed character data in Marked Section (CDATA MS)
   MODE_MS_IGNORE,  // Ignored Marked Section (IGNORE MS)
   MODE_MS_INCLUDE, // As CON_XML, + MSC_DSC ("]]>") recognized
   MODE_INI,        // when the parse of the document starts
   MODE_COMMENT,    // where only COM is recognized
   MODE_MD_COM,     // where only COM and MDC are recognized
   MODE_MD_HCOM,    // where only COM-MDC is recognized
   MODE_VOID,       // where no markup will be recognized
   MODE_DS,         // Declaration Subset (9.6.1)
   MODE_XDS,        // External Declaration Subset
   MODE_GRP,        // As indicated in ISO 8879, 9.6.1
   MODE_LITC,       // literal with replaceable character data
   MODE_MD,         // As indicated in ISO 8879, 9.6.1
   MODE_TAG,        // As indicated in ISO 8879, 9.6.1
   MODE_PI,         // As indicated in ISO 8879, 9.6.1
   MODE_ASL,        // attribute specification list started with DSO
   MODE_LITP,       // literal with replaceable parameter data
   MODE_BYP_MD,     // Bypass a Markup Declaration
   MODE_BYP_GRP,    // Bypass a Group
   MODE_BYP_DOCMD,  // Bypass a <!DOCTYPE (complex recovery)
   NBR_OF_MODES // ** This must be the last entry **
};

#endif /* DELIM_HEADER =======================================================*/
