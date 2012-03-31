/* $Id: MsgTemplate.cpp,v 1.28 2002-09-01 17:40:46 jlatone Exp $ */

#include "MsgTemplate.h"

#include "assert.h"

#if defined __MWERKS__   // MWERKS has its own message resources
extern char const * getRexxMessage(int id);
extern char const * getYasp3Message(int id);

#else
static char const * msgTable[] = {
/* _XXX__UNKNOWN      */ "",

/* _YSP__LOCATION1WF  */ "Location:  line &3;, col &4; in entity '&1;' (&2;):",
/* _YSP__LOCATION2WF  */ " -used at: line &3;, col &4; in entity '&1;' (&2;)",
/* _YSP__LOCATION1    */ "Location:  line &2;, col &3; in entity '&1;':",
/* _YSP__LOCATION2    */ " -used at: line &2;, col &3; in entity '&1;' ",
/* _YSP__CONTEXT1     */ "Context:   |&1;&2;",
/* _YSP__CONTEXT2     */ "           |&1;",
/* _YSP__FQGI         */ "FQGI:      &1;",
/* _YSP__BAD_BUFSIZE  */ "Buffers are too small",
/* _YSP__NOTAGC       */ "Missing TAGC delimiter",
/* _YSP__INVSTGNAME   */ "Element '&1;' was declared nowhere in the DTD",
/* _YSP__INVSTGMINI   */ "Start-tag for '&1;' is omitted while not declared omissible",
/* _YSP__INVETGNAME   */ "Cannot end unknown element '&1;'",
/* _YSP__INVETGMINI   */ "End-tag for '&1;' is omitted while not declared omissible",
/* _YSP__INVSTGEMPTY  */ "Empty start-tag out of context",
/* _YSP__INVETGEMPTY  */ "Empty end-tag out of context",
/* _YSP__INVETGOOC    */ "End-tag for '&1;' is out of context",
/* _YSP__INVETGPREMA  */ "Element '&1;' was prematurely ended",
/* _YSP__NOATTVAL     */ "Attribute value '&1;' was not declared for any attribute",
/* _YSP__DUPATTSPEC   */ "Attribute '&1;' has already been specified",
/* _YSP__INVFIXED     */ "Value for #FIXED attribute '&1;' must be '&2;'",
/* _YSP__NOREQUIRED   */ "Value for #REQUIRED attribute '&1;' is missing",
/* _YSP__NOTAGOMIT    */ "Start or end tags were omitted, but OMITTAG is off",
/* _YSP__NOTAGCMIN    */ "TAGC delimiter omitted, but SHORTTAG is off",
/* _YSP__INVNETMIN    */ "NET delimiter found in tag, but SHORTTAG is off",
/* _YSP__UBLNLIT      */ "Missing delimiter '&1;'.  The literal opened at line &2; has been closed because the entity ends.",
/* _YSP__UBLNPI       */ "Missing delimiter '&1;'.  The processing instruction opened at line &2; has been closed because the entity ends.",
/* _YSP__INVDLM       */ "Invalid delimiter found",
/* _YSP__BIGTKLIST    */ "Too many tokens found in value specification",
/* _YSP__BIGNAME      */ "Name is too long, current limit is &1;",
/* _YSP__BIGMLIT      */ "Minimum literal (line &1;-&2;) is too long (max &3;)",
/* _YSP__INVATTSPEC   */ "Invalid attribute specification",
/* _YSP__BIGAVL       */ "Attribute value literal (line &1;-&2;) is too long (max &3;)",
/* _YSP__BIGPLIT      */ "Parameter literal (line &1;-&2;) is too long (max &3;)",
/* _YSP__INVEMPTYLIT  */ "Empty literal not allowed",
/* _YSP__BADMODEL     */ "Bad content model or declared content for element '&1;'",
/* _YSP__DUPGI        */ "Element '&1;' has already been defined",
/* _YSP__DUPATTL      */ "Duplicate ATTLIST for element '&1;'",
/* _YSP__USPDECL      */ "Unsupported markup declaration type '&1;'",
/* _YSP__INVDECL      */ "Invalid markup declaration type '&1;'",
/* _YSP__NOMDC        */ "Missing MDC in declaration '&1;'",
/* _YSP__INVNAME      */ "Invalid name",
/* _YSP__INVTOKEN     */ "Invalid token",
/* _YSP__INVTAGOMIT   */ "Tag were inferred while tag omission is disabled",
/* _YSP__INVMINOMIT   */ "Omitted tag minimization is invalid",
/* _YSP__DUPDCLV      */ "Declared value '&1;' already found for this attribute list",
/* _YSP__BIGATTRIB    */ "Too many attributes defined",
/* _YSP__INVADCLV     */ "Declared value is invalid",
/* _YSP__INVADFTV     */ "Default value is invalid",
/* _YSP__DUPNOTDEF    */ "More than one notation attribute found",
/* _YSP__DUPIDDEF     */ "More than one ID attribute found",
/* _YSP__INVATTDCL    */ "Invalid syntax in attribute declaration",
/* _YSP__DUPATT       */ "Attribute '&1;' was already defined",
/* _YSP__BIGPI        */ "Processing instruction (line &1;-&2;) is too long (max &3;)",
/* _YSP__INVKEYWD1    */ "Keyword '&1;' was expected",
/* _YSP__INVPENTK     */ "Invalid type for a parameter entity",
/* _YSP__NODSC        */ "Declaration of notation attribute does not end with DSC",
/* _YSP__INVNOTDCL    */ "Invalid syntax in notation declaration",
/* _YSP__DUPDOCTYPE   */ "Duplicate DOCTYPE declaration",
/* _YSP__INVDOCTYPE   */ "Missing DSO or MDC delimiter in DOCTYPE declaration",
/* _YSP__INVMSSTART   */ "Missing DSO after status keyword in marked section",
/* _YSP__INVMSEND     */ "Marked section end outside marked section",
/* _YSP__BIGSKIP      */ "Many lines skipped, lines &1;-&2;",
/* _YSP__NOPS         */ "Expecting parameter separator",
/* _YSP__INVGROUP     */ "Group syntax is invalid",
/* _YSP__BIGGROUP     */ "Group too long",
/* _YSP__INVKEYWD     */ "A keyword was expected, one of: &1;",
/* _YSP__NOLIT        */ "A LIT or LITA delimiter was expected",
/* _YSP__UBLNPIC      */ "End of entity found before closing PIC delimiter",
/* _YSP__UBLNMS       */ "Unterminated marked section, started at line &1;",
/* _YSP__INVNUMCREF   */ "Numeric character entity (d&1;) out of defined range",
/* _YSP__INVNAMCREF   */ "Unknown named character reference '&1;'",
/* _YSP__INVMLCHAR    */ "Character not allowed in a minimum literal",
/* _YSP__NOINSTANCE   */ "The document instance is empty",
/* _YSP__SUPFDOCTYPE  */ "A DOCTYPE markup declaration is not allowed within a DTD",
/* _YSP__NODTD        */ "DOCTYPE declaration is missing or invalid.",
/* _YSP__BADDATAINDS  */ "Extraneous text was found in the declaration subset; missing DSC delimiter? Text is '&1;'",
/* _YSP__BADDATAINMD  */ "Extraneous text was found in the DOCTYPE declaration; text is '&1;'",
/* _YSP__BADCOMMENT   */ "Text was found outside comment delimiters within a comment declaration",
/* _YSP__INVOPTION    */ "Unknown parsing option",
/* _YSP__INVSAVEDIR   */ "",
/* _YSP__BADDECL      */ "Unrecognizable markup declaration",
/* _YSP__INVDELIM     */ "Improper delimiter: '&1;'",
/* _YSP__UBLNCOM      */ "Missing delimiter '&1;'.  Comment opened at line &2; has been closed.",
/* _YSP__WILDDECL     */ "Declaration found outside a DTD.",
/* _YSP__NOXMLVER     */ "Version information is missing from the XML declaration.",
/* _YSP__BADXMLPI     */ "Bad syntax in XML declaration.",

/* _SDCL__DUPDIC      */ "Delimiter-in-context '&1;' duplicates '&2;'",
/* _SDCL__DUPDLM      */ "Two identical delimiters found: '&1;'",

/* _ELM__INVSTGEMPTY  */ "Start tag for '&2;' out of context: infers EMPTY element '&1;'",
/* _ELM__INVDATAEMPTY */ "Data out of context: infers EMPTY element '&1;'",
/* _ELM__INVSTGCDATA  */ "Start tag for '&2;' out of context: infers CDATA element '&1;'",
/* _ELM__NOERROR      */ "",
/* _ELM__INVSTGCLOSE  */ "Start tag for '&2;' out of context: element '&1;' cannot be closed.",
/* _ELM__INVDATACLOSE */ "Data out of context: element '&1;' cannot be closed.",
/* _ELM__INVSTGOOC    */ "Start tag for '&1;' out of context: not found in any opened elements",
/* _ELM__INVDATAOOC   */ "Data out of context: not found in any opened elements",
/* _ELM__INVSTGNOCNT  */ "Start tag for '&2;' out of context: infers element '&1;' with empty content",
/* _ELM__INVDATANOCNT */ "Data out of context: infers element '&1;' with empty content",
/* _ELM__STGNOTSYNCH  */ "Start tag for '&2;' infers the end of element '&1;' which did not start within the current entity",
/* _ELM__DATANOTSYNCH */ "Data infers the end of element '&1;' which did not start within the current entity",
/* _ELM__ENDNOTSYNCH  */ "Cannot end element '&1;' which does not belong to the current entity",

/* _ENT__DUPNOTNAME   */ "Notation '&1;' has already been defined",
/* _ENT__NOENTGENDEF  */ "General entity '&1;' has not been defined, and there is no #DEFAULT",
/* _ENT__NOENTPARDEF  */ "Parameter entity '&1;' has not been defined",
/* _ENT__INVRECURSE   */ "Entity '&1;' must not invoke itself even indirectly",
/* _ENT__BADOPENDOC   */ "Unable to open SGML document",
/* _ENT__BADOPENDTD   */ "Unable to open DTD for base document element '&1;'",
/* _ENT__BADOPEN      */ "Unable to open entity '&1;'",

/* _REX__0_0          */ "",
/* _REX__1_0          */ "",
/* _REX__2_0          */ "Failure during finalization",
/* _REX__3_0          */ "Failure during initialization",
/* _REX__4_0          */ "Program interrupted",
/* _REX__5_0          */ "System resources exhausted",
/* _REX__6_0          */ "Unmatched \"/*\" or quote",
/* _REX__7_0          */ "WHEN or OTHERWISE expected",
/* _REX__8_0          */ "Unexpected THEN or ELSE",
/* _REX__9_0          */ "Unexpected WHEN or OTHERWISE",
/* _REX__10_0         */ "Unexpected or unmatched END",
/* _REX__11_0         */ "",
/* _REX__12_0         */ "",
/* _REX__13_0         */ "Invalid character in program",
/* _REX__14_0         */ "Incomplete DO/SELECT/IF",
/* _REX__15_0         */ "Invalid hexadecimal or binary string",
/* _REX__16_0         */ "Label not found",
/* _REX__17_0         */ "Unexpected PROCEDURE",
/* _REX__18_0         */ "THEN expected",
/* _REX__19_0         */ "String or symbol expected",
/* _REX__20_0         */ "Name expected",
/* _REX__21_0         */ "Invalid data on end of clause",
/* _REX__22_0         */ "Invalid character string",
/* _REX__23_0         */ "Invalid data string",
/* _REX__24_0         */ "Invalid TRACE request",
/* _REX__25_0         */ "Invalid sub-keyword found",
/* _REX__26_0         */ "Invalid whole number",
/* _REX__27_0         */ "Invalid DO syntax",
/* _REX__28_0         */ "Invalid LEAVE or ITERATE",
/* _REX__29_0         */ "Environment name too long",
/* _REX__30_0         */ "Name or string too long",
/* _REX__31_0         */ "Name starts with number or \".\"",
/* _REX__32_0         */ "",
/* _REX__33_0         */ "Invalid expression result",
/* _REX__34_0         */ "Logical value not \"0\" or \"1\"",
/* _REX__35_0         */ "Invalid expression",
/* _REX__36_0         */ "Unmatched \"(\" in expression",
/* _REX__37_0         */ "Unexpected \",\" or \")\"",
/* _REX__38_0         */ "Invalid template or pattern",
/* _REX__39_0         */ "",
/* _REX__40_0         */ "Incorrect call to routine",
/* _REX__41_0         */ "Bad arithmetic conversion",
/* _REX__42_0         */ "Arithmetic overflow/underflow",
/* _REX__43_0         */ "Routine not found",
/* _REX__44_0         */ "Function did not return data",
/* _REX__45_0         */ "No data specified on function RETURN",
/* _REX__46_0         */ "Invalid variable reference",
/* _REX__47_0         */ "Unexpected label",
/* _REX__48_0         */ "Failure in system service",
/* _REX__49_0         */ "Interpretation Error",
/* _REX__50_0         */ "Unrecognized reserved symbol",
/* _REX__51_0         */ "Invalid function name",
/* _REX__52_0         */ "Result returned by \"&1;\" is longer than &2; characters",
/* _REX__53_0         */ "Invalid option",
/* _REX__54_0         */ "Invalid STEM value",
/* _REX__0_1          */ "Error &1; running &2;, line &3;:",
/* _REX__0_2          */ "Error &1; in interactive trace:",
/* _REX__0_3          */ "&1;Interactive trace. \"Trace Off\" to end debug. ENTER to continue.",
/* _REX__0_4          */ "&1;Interactive trace.  Error &2;: &3;",
/* _REX__0_5          */ "",
/* _REX__0_6          */ "",
/* _REX__0_7          */ "",
/* _REX__0_8          */ "",
/* _REX__2_1          */ "Failure during finalization: &1;",
/* _REX__2_2          */ "Unknown Genitor",
/* _REX__3_1          */ "Failure during initialization: &1;",
/* _REX__4_1          */ "Program interrupted with HALT condition: &1;",
/* _REX__5_1          */ "System resources exhausted: &1;",
/* _REX__6_1          */ "Unmatched comment delimiter (\"/*\")",
/* _REX__6_2          */ "Unmatched single quote (\')",
/* _REX__6_3          */ "Unmatched double quote (\")",
/* _REX__7_1          */ "SELECT on line &1; requires WHEN; found \"&2;\"",
/* _REX__7_2          */ "SELECT on line &1; requires WHEN, OTHERWISE, or END; found \"&2;\"",
/* _REX__7_3          */ "All WHEN expressions of SELECT on line &1; are false; OTHERWISE expected",
/* _REX__8_1          */ "THEN has no corresponding IF or WHEN clause",
/* _REX__8_2          */ "ELSE has no corresponding THEN clause",
/* _REX__9_1          */ "WHEN has no corresponding SELECT",
/* _REX__9_2          */ "OTHERWISE has no corresponding SELECT",
/* _REX__10_1         */ "END has no corresponding DO or SELECT",
/* _REX__10_2         */ "END corresponding to DO on line &1; must have a symbol following that matches the control variable (or no symbol); found \"&2;\"",
/* _REX__10_3         */ "END corresponding to DO on line &1; must not have a symbol following it because there is no control variable; found \"&2;\"",
/* _REX__10_4         */ "END corresponding to SELECT on line &1; must not have a symbol following; found \"&2;\"",
/* _REX__10_5         */ "END must not immediately follow THEN",
/* _REX__10_6         */ "END must not immediately follow ELSE",
/* _REX__13_1         */ "Invalid character in program \"&1;\" (&2;X)",
/* _REX__14_1         */ "DO instruction requires a matching END",
/* _REX__14_2         */ "SELECT instruction requires a matching END",
/* _REX__14_3         */ "THEN requires a following instruction",
/* _REX__14_4         */ "ELSE requires a following instruction",
/* _REX__15_1         */ "Invalid location of blank in position &1; in hexadecimal string",
/* _REX__15_2         */ "Invalid location of blank in position &1; in binary string",
/* _REX__15_3         */ "Only 0-9, a-f, A-F, and blank are valid in a hexadecimal string; found \"&1;\"",
/* _REX__15_4         */ "Only 0, 1, and blank are valid in a binary string; found \"&1;\"",
/* _REX__16_1         */ "Label \"&1;\" not found",
/* _REX__16_2         */ "Cannot SIGNAL to label \"&1;\" because it is inside an IF, SELECT or DO group",
/* _REX__16_3         */ "Cannot invoke label \"&1;\" because it is inside an IF, SELECT or DO group",
/* _REX__17_1         */ "PROCEDURE is valid only when it is the first instruction executed after an internal CALL or function invocation",
// _REX__18_1            "IF keyword on line &1; requires matching THEN clause; found \"&2;\"",
/* _REX__18_1         */ "IF keyword requires matching THEN clause; found \"&1;\"",
/* _REX__18_2         */ "WHEN keyword on line &1; requires matching THEN clause; found \"&2;\"",
/* _REX__19_1         */ "String or symbol expected after ADDRESS keyword; found \"&1;\"",
/* _REX__19_2         */ "String or symbol expected after CALL keyword; found \"&1;\"",
/* _REX__19_3         */ "String or symbol expected after NAME keyword; found \"&1;\"",
/* _REX__19_4         */ "String or symbol expected after SIGNAL keyword; found \"&1;\"",
/* _REX__19_6         */ "String or symbol expected after TRACE keyword; found \"&1;\"",
/* _REX__19_7         */ "Symbol expected in parsing pattern; found \"&1;\"",
/* _REX__20_1         */ "Name required; found \"&1;\"",
/* _REX__20_2         */ "Found \"&1;\" where only a name is valid",
/* _REX__21_1         */ "The clause ended at an unexpected token; found \"&1;\"",
/* _REX__22_1         */ "Invalid character string |&1;|X",
/* _REX__23_1         */ "Invalid data string |&1;|X",
/* _REX__24_1         */ "TRACE request letter must be one of \"ACEFILNOR\"; found \"&1;\"",
/* _REX__25_1         */ "CALL ON must be followed by one of the keywords ERROR, FAILURE, HALT, or NOTREADY; found \"&1;\"",
/* _REX__25_2         */ "CALL OFF must be followed by one of the keywords ERROR, FAILURE, HALT, or NOTREADY; found \"&1;\"",
/* _REX__25_3         */ "SIGNAL ON must be followed by one of the keywords ERROR, FAILURE, HALT, LOSTDIGITS, NOTREADY, NOVALUE, or SYNTAX; found \"&1;\"",
/* _REX__25_4         */ "SIGNAL OFF must be followed by one of the keywords ERROR, FAILURE, HALT, LOSTDIGITS, NOTREADY, NOVALUE, or SYNTAX; found \"&1;\"",
/* _REX__25_5         */ "ADDRESS WITH must be followed by one of the keywords &1;; found \"&2;\"",
/* _REX__25_6         */ "INPUT must be followed by one of the keywords &1;; found \"&2;\"",
/* _REX__25_7         */ "OUTPUT must be followed by one of the keywords &1;; found \"&2;\"",
/* _REX__25_8         */ "APPEND must be followed by one of the keywords &1;; found \"&2;\"",
/* _REX__25_9         */ "REPLACE must be followed by one of the keywords &1;; found \"&2;\"",
/* _REX__25_11        */ "NUMERIC FORM must be followed by one of the keywords &1;; found \"&2;\"",
/* _REX__25_12        */ "PARSE must be followed by one of the keywords UPPER, ARG, LINEIN, PULL, SOURCE, VALUE, VAR or VERSION; found \"&1;\"",
/* _REX__25_13        */ "UPPER must be followed by one of the keywords ARG, LINEIN, PULL, SOURCE, VALUE, VAR or VERSION; found \"&1;\"",
/* _REX__25_14        */ "ERROR must be followed by one of the keywords &1;; found \"&2;\"",
/* _REX__25_15        */ "NUMERIC must be followed by one of the keywords DIGITS, FORM, VALUE, or FUZZ; found \"&1;\"",
/* _REX__25_16        */ "FOREVER must be followed by one of the keywords &1; or nothing; found \"&2;\"",
/* _REX__25_17        */ "PROCEDURE must be followed by the keyword EXPOSE or nothing; found \"&1;\"",
/* _REX__26_1         */ "Whole numbers must fit within current DIGITS setting(&1;); found \"&2;\"",
/* _REX__26_2         */ "Value of repetition count expression in DO instruction must be zero or a positive whole number; found \"&1;\"",
/* _REX__26_3         */ "Value of FOR expression in DO instruction must be zero or a positive whole number; found \"&1;\"",
/* _REX__26_4         */ "Positional pattern of parsing template must be a whole number; found \"&1;\"",
/* _REX__26_5         */ "NUMERIC DIGITS value must be a positive whole number; found \"&1;\"",
/* _REX__26_6         */ "NUMERIC FUZZ value must be zero or a positive whole number; found \"&1;\"",
/* _REX__26_7         */ "Number used in TRACE setting must be a whole number; found \"&1;\"",
/* _REX__26_8         */ "Operand to right of the power operator (\"**\") must be a whole number; found \"&1;\"",
/* _REX__26_11        */ "Result of &1; % &2; operation would need exponential notation at current NUMERIC DIGITS &3;",
/* _REX__26_12        */ "Result of % operation used for &1; // &2; operation would need exponential notation at current NUMERIC DIGITS &3;",
/* _REX__27_1         */ "Invalid use of keyword \"&1;\" in DO clause",
/* _REX__28_1         */ "LEAVE is valid only within a repetitive DO loop",
/* _REX__28_2         */ "ITERATE is valid only within a repetitive DO loop",
/* _REX__28_3         */ "Symbol following LEAVE (\"&1;\") must either match control variable of a current DO loop or be omitted",
/* _REX__28_4         */ "Symbol following ITERATE (\"&1;\") must either match control variable of a current DO loop or be omitted",
/* _REX__29_1         */ "Environment name exceeds &1; characters; found \"&2;\"",
/* _REX__30_1         */ "Name exceeds &1; characters",
/* _REX__30_2         */ "Literal string exceeds &1; characters",
/* _REX__31_1         */ "A value cannot be assigned to a number; found \"&1;\"",
/* _REX__31_2         */ "Variable symbol must not start with a number; found \"&1;\"",
/* _REX__31_3         */ "Variable symbol must not start with a \".\"; found \"&1;\"",
/* _REX__33_1         */ "Value of NUMERIC DIGITS (\"&1;\") must exceed value of NUMERIC FUZZ (\"&2;\")",
/* _REX__33_2         */ "Value of NUMERIC DIGITS (\"&1;\") must not exceed \"&2;\"",
/* _REX__33_3         */ "Result of expression following NUMERIC FORM must start with \"E\" or \"S\"; found \"&1;\"",
/* _REX__34_1         */ "Value of expression following IF keyword must be exactly \"0\" or \"1\"; found \"&1;\"",
/* _REX__34_2         */ "Value of expression following WHEN keyword must be exactly \"0\" or \"1\"; found \"&1;\"",
/* _REX__34_3         */ "Value of expression following WHILE keyword must be exactly \"0\" or \"1\"; found \"&1;\"",
/* _REX__34_4         */ "Value of expression following UNTIL keyword must be exactly \"0\" or \"1\"; found \"&1;\"",
/* _REX__34_5         */ "Value of expression to left of logical operator \"&1;\" must be exactly \"0\" or \"1\"; found \"&2;\"",
/* _REX__34_6         */ "Value of expression to right of logical operator \"&1;\" must be exactly \"0\" or \"1\"; found \"&2;\"",
/* _REX__35_1         */ "Invalid expression detected at \"&1;\"",
/* _REX__37_1         */ "Unexpected \",\"",
/* _REX__37_2         */ "Unmatched \")\" in expression",
/* _REX__38_1         */ "Invalid parsing template detected at \"&1;\"",
/* _REX__38_2         */ "Invalid parsing position detected at \"&1;\"",
/* _REX__38_3         */ "PARSE VALUE instruction requires WITH keyword",
/* _REX__40_1         */ "External routine \"&1;\" failed",
/* _REX__40_3         */ "Not enough arguments in invocation of &1;; minimum expected is &2;",
/* _REX__40_4         */ "Too many arguments in invocation of &1;; maximum expected is &2;",
/* _REX__40_5         */ "Missing argument in invocation of &1;; argument &2; is required",
/* _REX__40_9         */ "&1; argument &2; exponent exceeds &3; digits; found \"&4;\"",
/* _REX__40_11        */ "&1; argument &2; must be a number; found \"&3;\"",
/* _REX__40_12        */ "&1; argument &2; must be a whole number; found \"&3;\"",
/* _REX__40_13        */ "&1; argument &2; must be zero or positive; found \"&3;\"",
/* _REX__40_14        */ "&1; argument &2; must be positive; found \"&3;\"",
/* _REX__40_15        */ "&1; argument &2; must fit in &3; digits; found \"&4;\"",
/* _REX__40_16        */ "&1; argument 1 requires a whole number fitting within DIGITS(&2;); found \"&3;\"",
/* _REX__40_17        */ "&1; argument 1 must have an integer part in the range 0:90 and a decimal part no larger than .9; found \"&2;\"",
/* _REX__40_18        */ "&1; conversion must have a year in the range 0001 to 9999",
/* _REX__40_19        */ "&1; argument 2, \"&2;\", is not in the format described by argument 3, \"&3;\"",
/* _REX__40_21        */ "&1; argument &2; must not be null",
/* _REX__40_23        */ "&1; argument &2; must be a single character; found \"&3;\"",
/* _REX__40_24        */ "&1; argument 1 must be a binary string; found \"&2;\"",
/* _REX__40_25        */ "&1; argument 1 must be a hexadecimal string; found \"&2;\"",
/* _REX__40_26        */ "&1; argument 1 must be a valid symbol; found \"&2;\"",
/* _REX__40_27        */ "&1; argument 1 must be a valid stream name; found \"&2;\"",
/* _REX__40_28        */ "&1; argument &2;, option must start with one of \"&3;\"; found \"&4;\"",
/* _REX__40_29        */ "&1; conversion to format \"&2;\" is not allowed",
/* _REX__40_31        */ "&1; argument 1 (\"&2;\") must not exceed 100000",
/* _REX__40_32        */ "&1; the difference between argument 1 (\"&2;\") and argument 2 (\"&3;\") must not exceed 100000",
/* _REX__40_33        */ "&1; argument 1 (\"&2;\") must be less than or equal to argument 2 (\"&3;\")",
/* _REX__40_34        */ "&1; argument 1 (\"&2;\") must be less than or equal to the number of lines in the program (&3;)",
/* _REX__40_35        */ "&1; argument 1 cannot be expressed as a whole number; found \"&2;\"",
// _REX__40_36        */ "&1; argument 1 must be the name of a variable in the pool; found \"&2;\"",
/* _REX__40_36        */ "argument 1 must be the name of a variable in the pool; found \"&1;\"",
// _REX__40_37        */ "&1; argument 3 must be the name of a pool; found \"&2;\"",
/* _REX__40_37        */ "argument 3 must be the name of a pool; found \"&1;\"",
/* _REX__40_38        */ "&1; argument &2; is not large enough to format \"&3;\"",
/* _REX__40_39        */ "&1; argument 3 is not zero or one; found \"&2;\"",
/* _REX__40_41        */ "&1; argument &2; must be within the bounds of the stream; found \"&3;\"",
/* _REX__40_42        */ "&1; argument 1; cannot position on this stream; found \"&2;\"",
/* _REX__40_43        */ "",
/* _REX__40_44        */ "",
/* _REX__40_45        */ "&1; argument &2; must be a single non-alphanumeric character or the null string; found \"&3;\"",
/* _REX__40_46        */ "&1; argument &2;, \"&3;\", is a format incompatible with separator specified in argument \"&4;\"",
/* _REX__41_1         */ "Non-numeric value (\"&1;\") to left of arithmetic operation \"&2;\"",
/* _REX__41_2         */ "Non-numeric value (\"&1;\") to right of arithmetic operation \"&2;\"",
/* _REX__41_3         */ "Non-numeric value (\"&1;\") used with prefix operator \"&2;\"",
/* _REX__41_4         */ "Value of TO expression in DO instruction must be numeric; found \"&1;\"",
/* _REX__41_5         */ "Value of BY expression in DO instruction must be numeric; found \"&1;\"",
/* _REX__41_6         */ "Value of control variable expression of DO instruction must be numeric; found \"&1;\"",
/* _REX__41_7         */ "Exponent exceeds &1; found \"&2;\"",
/* _REX__42_1         */ "Arithmetic overflow detected at \"&1; &2; &3;\"; exponent of result requires more than &4; digits",
/* _REX__42_2         */ "Arithmetic underflow detected at \"&1; &2; &3;\"; exponent of result requires more than &4; digits",
/* _REX__42_3         */ "Arithmetic overflow; divisor must not be zero",
/* _REX__43_1         */ "Could not find routine \"&1;\"",
/* _REX__44_1         */ "No data returned from function \"&1;\"",
/* _REX__45_1         */ "Data expected on RETURN instruction because routine \"&1;\" was called as a function",
/* _REX__46_1         */ "Extra token (\"&1;\") found in variable reference; \")\" expected",
/* _REX__47_1         */ "INTERPRET data must not contain labels; found \"&1;\"",
/* _REX__48_1         */ "Failure in system service: &1;",
/* _REX__49_1         */ "Interpretation Error: &1;",
/* _REX__50_1         */ "Unrecognized reserved symbol \"&1;\"",
/* _REX__51_1         */ "Unquoted function names must not end with a period; found \"&1;\"",
/* _REX__53_1         */ "Variable reference expected after STREAM keyword; found \"&1;\"",
/* _REX__53_2         */ "Variable reference expected after STEM keyword; found \"&1;\"",
/* _REX__53_3         */ "Argument to STEM must have one period, as its last character; found \"&1;\"",
/* _REX__54_1         */ "For this STEM APPEND, the value of \"&1;\" must be a count of lines; found: \"&2;\"",

/* _SMP__USAGE00      */ "usage: smp01 [options]",
/* _SMP__USAGE01      */ "  options:",
/* _SMP__USAGE02      */ "    -dtd                file path of the DTD",
/* _SMP__USAGE03      */ "    -e (tag tag...)     list of EMPTY tags",
/* _SMP__USAGE04      */ "    -f (input_file)     path to the file to parse",
/* _SMP__USAGE05      */ "    -g                  make fully qualified GI's",
/* _SMP__USAGE06      */ "    -h                  input file is HTML",
/* _SMP__USAGE07      */ "    -i (encoding)       input file encoding",
/* _SMP__USAGE08      */ "    -k (encoding)       output file encoding",
/* _SMP__USAGE09      */ "    -log (log_file)     path to log file",
/* _SMP__USAGE10      */ "    -n                  narrow the text output",
/* _SMP__USAGE11      */ "    -p                  do not print numbers",
/* _SMP__USAGE12      */ "    -q                  quiet",
/* _SMP__USAGE13      */ "    -r (event_count)    report at each event_count",
/* _SMP__USAGE14      */ "    -t                  trace all SGML events",
/* _SMP__USAGE15      */ "    -tl (line_num)      start trace from line_num in book",
/* _SMP__USAGE16      */ "    -te (element_id)    start trace from element_id",
/* _SMP__USAGE17      */ "    -v                  perform SGML validation",
/* _SMP__USAGE18      */ "    -x                  input file is XML",
/* _SMP__USAGE19      */ "    -y                  include ignored Marked Section",
/* _SMP__USAGE20      */ "    -~                  dont't expand PI or SDATA",
/* _SMP__USAGE21      */ "    -#                  compress blanks",
/* _SMP__USAGE22      */ "    -]                  report entities",
/* _SMP__OPTUNKNOWN   */ "Unknown option: '&1;'.",
/* _SMP__OPTNOVAL     */ "Missing value following '&1;' option.",
/* _SMP__OPTNOVALNUM  */ "Missing numeric value following '&1;' option.",
/* _SMP__OPTMISPLACED */ "'&1;' misplaced; the book name should be the last argument.",
/* _SMP__OPTWAITLINE  */ "Suppressing output until line &1;.",
/* _SMP__OPTCHKPTS    */ "Checkpoints every &1; elements.",
/* _SMP__OPTWAITELT   */ "Suppressing output until element &1;.",
/* _SMP__BADLOGFILE   */ "Log file '&1;' cannot be opened.",
/* _SMP__NOPARSER     */ "Unable to set up SGML parser.",
/* _SMP__CANTPARSE    */ "Unable to parse document: '&1;'.",
/* _SMP__SETEMPTIES   */ "Empty tag list set to:",
/* _SMP__BLANKCOMPRESS*/ "Stripping various whitespace characters in text.",
/* _SMP__ENDREVIEW    */ "Finished '&1;', &2; elements.",
/* _SMP__CHKPT        */ "At element &1;, line &2; in entity '&3;'",
/* _SMP__BIGLVL       */ "Nesting level extremely high (&1;) -- exiting",
/* _SMP__CANTOPENENT  */ "Can't resolve entity '&1;'.",
/* _SMP__CANTOPENDTD  */ "Unable to open DTD.",
/* _SMP__OPENDTD      */ "Reading DTD from file '&1;'.",
/* _SMP__BIGSEPA      */ "---------------------------------------------------------",

/*  XXX__END          */ ""
};

#endif

/*------------------------------------------------------------ struct RxCode -+
| This struct establishes the correspondence between a rex message id (subid) |
| and: the main id, the main no and the main subno.                           |
+----------------------------------------------------------------------------*/
#ifdef __MWERKS__
#pragma pcrelconstdata on
#endif
static const struct RxCode {
   unsigned char mainNo;
   unsigned char subNo;
} rxCodes [] = {
   {  0,  0 },    //  _REX__0_0
   {  1,  0 },    //  _REX__1_0
   {  2,  0 },    //  _REX__2_0
   {  3,  0 },    //  _REX__3_0
   {  4,  0 },    //  _REX__4_0
   {  5,  0 },    //  _REX__5_0
   {  6,  0 },    //  _REX__6_0
   {  7,  0 },    //  _REX__7_0
   {  8,  0 },    //  _REX__8_0
   {  9,  0 },    //  _REX__9_0
   { 10,  0 },    //  _REX__10_0
   { 11,  0 },    //  _REX__11_0
   { 12,  0 },    //  _REX__12_0
   { 13,  0 },    //  _REX__13_0
   { 14,  0 },    //  _REX__14_0
   { 15,  0 },    //  _REX__15_0
   { 16,  0 },    //  _REX__16_0
   { 17,  0 },    //  _REX__17_0
   { 18,  0 },    //  _REX__18_0
   { 19,  0 },    //  _REX__19_0
   { 20,  0 },    //  _REX__20_0
   { 21,  0 },    //  _REX__21_0
   { 22,  0 },    //  _REX__22_0
   { 23,  0 },    //  _REX__23_0
   { 24,  0 },    //  _REX__24_0
   { 25,  0 },    //  _REX__25_0
   { 26,  0 },    //  _REX__26_0
   { 27,  0 },    //  _REX__27_0
   { 28,  0 },    //  _REX__28_0
   { 29,  0 },    //  _REX__29_0
   { 30,  0 },    //  _REX__30_0
   { 31,  0 },    //  _REX__31_0
   { 32,  0 },    //  _REX__32_0
   { 33,  0 },    //  _REX__33_0
   { 34,  0 },    //  _REX__34_0
   { 35,  0 },    //  _REX__35_0
   { 36,  0 },    //  _REX__36_0
   { 37,  0 },    //  _REX__37_0
   { 38,  0 },    //  _REX__38_0
   { 39,  0 },    //  _REX__39_0
   { 40,  0 },    //  _REX__40_0
   { 41,  0 },    //  _REX__41_0
   { 42,  0 },    //  _REX__42_0
   { 43,  0 },    //  _REX__43_0
   { 44,  0 },    //  _REX__44_0
   { 45,  0 },    //  _REX__45_0
   { 46,  0 },    //  _REX__46_0
   { 47,  0 },    //  _REX__47_0
   { 48,  0 },    //  _REX__48_0
   { 49,  0 },    //  _REX__49_0
   { 50,  0 },    //  _REX__50_0
   { 51,  0 },    //  _REX__51_0
   { 52,  0 },    //  _REX__52_0
   { 53,  0 },    //  _REX__53_0
   { 54,  0 },    //  _REX__54_0
   {  0,  1 },    //  _REX__0_1
   {  0,  2 },    //  _REX__0_2
   {  0,  3 },    //  _REX__0_3
   {  0,  4 },    //  _REX__0_4
   {  0,  5 },    //  _REX__0_5
   {  0,  6 },    //  _REX__0_6
   {  0,  7 },    //  _REX__0_7
   {  0,  8 },    //  _REX__0_8
   {  2,  1 },    //  _REX__2_1
   {  2,  2 },    //  _REX__2_2
   {  3,  1 },    //  _REX__3_1
   {  4,  1 },    //  _REX__4_1
   {  5,  1 },    //  _REX__5_1
   {  6,  1 },    //  _REX__6_1
   {  6,  2 },    //  _REX__6_2
   {  6,  3 },    //  _REX__6_3
   {  7,  1 },    //  _REX__7_1
   {  7,  2 },    //  _REX__7_2
   {  7,  3 },    //  _REX__7_3
   {  8,  1 },    //  _REX__8_1
   {  8,  2 },    //  _REX__8_2
   {  9,  1 },    //  _REX__9_1
   {  9,  2 },    //  _REX__9_2
   { 10,  1 },    //  _REX__10_1
   { 10,  2 },    //  _REX__10_2
   { 10,  3 },    //  _REX__10_3
   { 10,  4 },    //  _REX__10_4
   { 10,  5 },    //  _REX__10_5
   { 10,  6 },    //  _REX__10_6
   { 13,  1 },    //  _REX__13_1
   { 14,  1 },    //  _REX__14_1
   { 14,  2 },    //  _REX__14_2
   { 14,  3 },    //  _REX__14_3
   { 14,  4 },    //  _REX__14_4
   { 15,  1 },    //  _REX__15_1
   { 15,  2 },    //  _REX__15_2
   { 15,  3 },    //  _REX__15_3
   { 15,  4 },    //  _REX__15_4
   { 16,  1 },    //  _REX__16_1
   { 16,  2 },    //  _REX__16_2
   { 16,  3 },    //  _REX__16_3
   { 17,  1 },    //  _REX__17_1
   { 18,  1 },    //  _REX__18_1
   { 18,  2 },    //  _REX__18_2
   { 19,  1 },    //  _REX__19_1
   { 19,  2 },    //  _REX__19_2
   { 19,  3 },    //  _REX__19_3
   { 19,  4 },    //  _REX__19_4
   { 19,  6 },    //  _REX__19_6
   { 19,  7 },    //  _REX__19_7
   { 20,  1 },    //  _REX__20_1
   { 20,  2 },    //  _REX__20_2
   { 21,  1 },    //  _REX__21_1
   { 22,  1 },    //  _REX__22_1
   { 23,  1 },    //  _REX__23_1
   { 24,  1 },    //  _REX__24_1
   { 25,  1 },    //  _REX__25_1
   { 25,  2 },    //  _REX__25_2
   { 25,  3 },    //  _REX__25_3
   { 25,  4 },    //  _REX__25_4
   { 25,  5 },    //  _REX__25_5
   { 25,  6 },    //  _REX__25_6
   { 25,  7 },    //  _REX__25_7
   { 25,  8 },    //  _REX__25_8
   { 25,  9 },    //  _REX__25_9
   { 25, 11 },    //  _REX__25_11
   { 25, 12 },    //  _REX__25_12
   { 25, 13 },    //  _REX__25_13
   { 25, 14 },    //  _REX__25_14
   { 25, 15 },    //  _REX__25_15
   { 25, 16 },    //  _REX__25_16
   { 25, 17 },    //  _REX__25_17
   { 26,  1 },    //  _REX__26_1
   { 26,  2 },    //  _REX__26_2
   { 26,  3 },    //  _REX__26_3
   { 26,  4 },    //  _REX__26_4
   { 26,  5 },    //  _REX__26_5
   { 26,  6 },    //  _REX__26_6
   { 26,  7 },    //  _REX__26_7
   { 26,  8 },    //  _REX__26_8
   { 26, 11 },    //  _REX__26_11
   { 26, 12 },    //  _REX__26_12
   { 27,  1 },    //  _REX__27_1
   { 28,  1 },    //  _REX__28_1
   { 28,  2 },    //  _REX__28_2
   { 28,  3 },    //  _REX__28_3
   { 28,  4 },    //  _REX__28_4
   { 29,  1 },    //  _REX__29_1
   { 30,  1 },    //  _REX__30_1
   { 30,  2 },    //  _REX__30_2
   { 31,  1 },    //  _REX__31_1
   { 31,  2 },    //  _REX__31_2
   { 31,  3 },    //  _REX__31_3
   { 33,  1 },    //  _REX__33_1
   { 33,  2 },    //  _REX__33_2
   { 33,  3 },    //  _REX__33_3
   { 34,  1 },    //  _REX__34_1
   { 34,  2 },    //  _REX__34_2
   { 34,  3 },    //  _REX__34_3
   { 34,  4 },    //  _REX__34_4
   { 34,  5 },    //  _REX__34_5
   { 34,  6 },    //  _REX__34_6
   { 35,  1 },    //  _REX__35_1
   { 37,  1 },    //  _REX__37_1
   { 37,  2 },    //  _REX__37_2
   { 38,  1 },    //  _REX__38_1
   { 38,  2 },    //  _REX__38_2
   { 38,  3 },    //  _REX__38_3
   { 40,  1 },    //  _REX__40_1
   { 40,  3 },    //  _REX__40_3
   { 40,  4 },    //  _REX__40_4
   { 40,  5 },    //  _REX__40_5
   { 40,  9 },    //  _REX__40_9
   { 40, 11 },    //  _REX__40_11
   { 40, 12 },    //  _REX__40_12
   { 40, 13 },    //  _REX__40_13
   { 40, 14 },    //  _REX__40_14
   { 40, 15 },    //  _REX__40_15
   { 40, 16 },    //  _REX__40_16
   { 40, 17 },    //  _REX__40_17
   { 40, 18 },    //  _REX__40_18
   { 40, 19 },    //  _REX__40_19
   { 40, 21 },    //  _REX__40_21
   { 40, 23 },    //  _REX__40_23
   { 40, 24 },    //  _REX__40_24
   { 40, 25 },    //  _REX__40_25
   { 40, 26 },    //  _REX__40_26
   { 40, 27 },    //  _REX__40_27
   { 40, 28 },    //  _REX__40_28
   { 40, 29 },    //  _REX__40_29
   { 40, 31 },    //  _REX__40_31
   { 40, 32 },    //  _REX__40_32
   { 40, 33 },    //  _REX__40_33
   { 40, 34 },    //  _REX__40_34
   { 40, 35 },    //  _REX__40_35
   { 40, 36 },    //  _REX__40_36
   { 40, 37 },    //  _REX__40_37
   { 40, 38 },    //  _REX__40_38
   { 40, 39 },    //  _REX__40_39
   { 40, 41 },    //  _REX__40_41
   { 40, 42 },    //  _REX__40_42
   { 40, 43 },    //  _REX__40_43
   { 40, 44 },    //  _REX__40_44
   { 40, 45 },    //  _REX__40_45
   { 40, 46 },    //  _REX__40_46
   { 41,  1 },    //  _REX__41_1
   { 41,  2 },    //  _REX__41_2
   { 41,  3 },    //  _REX__41_3
   { 41,  4 },    //  _REX__41_4
   { 41,  5 },    //  _REX__41_5
   { 41,  6 },    //  _REX__41_6
   { 41,  7 },    //  _REX__41_7
   { 42,  1 },    //  _REX__42_1
   { 42,  2 },    //  _REX__42_2
   { 42,  3 },    //  _REX__42_3
   { 43,  1 },    //  _REX__43_1
   { 44,  1 },    //  _REX__44_1
   { 45,  1 },    //  _REX__45_1
   { 46,  1 },    //  _REX__46_1
   { 47,  1 },    //  _REX__47_1
   { 48,  1 },    //  _REX__48_1
   { 49,  1 },    //  _REX__49_1
   { 50,  1 },    //  _REX__50_1
   { 51,  1 },    //  _REX__51_1
   { 53,  1 },    //  _REX__53_1
   { 53,  2 },    //  _REX__53_2
   { 53,  3 },    //  _REX__53_3
   { 54,  1 }     //  _REX__54_1
};
#ifdef __MWERKS__
#pragma pcrelconstdata reset
#endif

/*-------------------------------------------------------------getMsgTemplate-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * getMsgTemplate(MsgTemplateId id) {
#if defined __MWERKS__
   if (id >= _REX__0_0) {
      return getRexxMessage(id - _REX__0_0);
   }else {
      #if defined COM_JAXO_YAXX_DENY_XML
         assert (false);
         return "";
      #else
         return getYasp3Message(id - _YSP__FirstMessage);
      #endif
   }
#else
   return msgTable[(int)id];
#endif
}

/*----------------------------------------------------------------isRxMessage-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool isRxMessage(MsgTemplateId id) {
   return (id >= _REX__0_0) && (id <= _REX__54_1);
}

/*----------------------------------------------------------------getRxMainId-+
|                                                                             |
+----------------------------------------------------------------------------*/
MsgTemplateId getRxMainId(MsgTemplateId subid) {
   return (MsgTemplateId)(_REX__0_0 + rxCodes[subid-_REX__0_0].mainNo);
}

/*----------------------------------------------------------------getRxMainId-+
|                                                                             |
+----------------------------------------------------------------------------*/
MsgTemplateId getRxMainId(int mainNo) {
   return (MsgTemplateId)(_REX__0_0 + mainNo);
}

/*------------------------------------------------------------getRxMainCodeNo-+
|                                                                             |
+----------------------------------------------------------------------------*/
int getRxMainCodeNo(MsgTemplateId subid) {
   return rxCodes[subid-_REX__0_0].mainNo;
}

/*-------------------------------------------------------------getRxSubCodeNo-+
|                                                                             |
+----------------------------------------------------------------------------*/
int getRxSubCodeNo(MsgTemplateId subid) {
   return rxCodes[subid-_REX__0_0].subNo;
}
/*===========================================================================*/
