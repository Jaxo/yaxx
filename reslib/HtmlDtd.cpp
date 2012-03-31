/* $Id: HtmlDtd.cpp,v 1.5 2002-08-31 13:40:47 pgr Exp $
*
* This is the HTML dtd, from:
*
* <!DOCTYPE HTML
*           PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN"
*           "http://www.w3.org/TR/REC-html40/loose.dtd">
*
*
* Currently (01/24/02), YASP3 doesn't know how to reuse a "compiled DTD."
* So YASP3 needs to recompile it each time.
*
* This is one of the reasons we have the HTML DTD "online", so to avoid
* too much IO.  But this is not the only reason.
*
* Assuming I enhance YASP3 so to reuse a "compiled DTD", we will
* have, anyway, to access at least once to this DTD.  And experience
* has prooved that "external data files" is a bad idea.  Most of YASP
* users pest against having to have a local OCS, messages, etc...
* The resources is the place where to have this kind of data.
* Resources are DLL (or SO), and everyone know those should be on
* the PATH.  Which justifies this file should be kept in the resources,
*
* I foresee 2 ways to have YASP3 reuse a compiled DTD:
*
* 1) The first time the method "openDocument" of a YASP3 instance is called,
*    YASP3 generates an event to request a DTD.  It does it before to parse
*    the very first tag of the document.
*    All subsequent "Yasp3::openDocument" on the SAME instance
*    will parse each new document reusing the same DTD that was
*    used for the first document, so the DTD will not be reparsed.
*    => Solution.  In XmlIOMgr, rather than to have each transform session
*       construct and delete a Yasp3 parser, have a few Yasp3 parsers
*       available and don't delete them!  (a transform session is an
*       instance of XmlIoMgr::YaspEntry)
*    => But... This solution has a few flaws: I'm not 100% certain that the
*       image of the DTD is constant.  In fact, I'm quite sure that the
*       parse of a document may add "unknown tag" to the DTD tag list.
*       Therefore, a bad document could pollute the DTD image.
*
* 2) A DTD is just two lists: the ElementList and the EntityList.
*    These lists are TpLists, which have shallow copies and copy on write.
*    It should then be easy to make a copy of the lists after the DTD
*    has been parsed, and to create a YASP3 constructor taking these
*    lists as arguments.
*    This may require a few days to test and I currently don't have
*    this time.
*/
#if !defined COM_JAXO_YAXX_DENY_XML
#include "HtmlDtd.h"

#if defined __MWERKS__
#pragma pcrelconstdata on
static char const htmlDtd[] = "<!ELEMENT HTML O O ANY>";  // easy!
#pragma pcrelconstdata reset

#elif defined __MWERKS__SOLUTION_2
#pragma pcrelconstdata on
static char const htmlDtd[] =
"<!ENTITY % HTML.Version \"-//W3C//DTD HTML 4.0 Transitional//EN\">"
"<!ENTITY % head.misc \"SCRIPT|STYLE|META|LINK|OBJECT\">"
"<!ENTITY % heading \"H1|H2|H3|H4|H5|H6\">"
"<!ENTITY % phrase \"EM | STRONG | DFN | CODE | SAMP | KBD | VAR | CITE | ABBR | ACRONYM | TT | I | B | U | S | STRIKE | BIG | SMALL\">"
"<!ENTITY % special \"A | IMG | APPLET | OBJECT | FONT | BASEFONT | BR | SCRIPT | MAP | Q | SUB | SUP | SPAN | BDO | IFRAME\">"
"<!ENTITY % formctrl \"INPUT | SELECT | TEXTAREA | LABEL | BUTTON\">"
"<!ENTITY % inline \"#PCDATA | %phrase; | %special; | %formctrl;\">"
"<!ENTITY % block \"P | %heading; | UL | OL | DIR | MENU | PRE | DL | DIV | CENTER | NOSCRIPT | NOFRAMES | BLOCKQUOTE | FORM | ISINDEX | HR | TABLE | FIELDSET | ADDRESS\">"
"<!ENTITY % flow \"%block; | %inline;\">"
"<!ENTITY % pre.exclusion \"IMG|OBJECT|APPLET|BIG|SMALL|SUB|SUP|FONT|BASEFONT\">"
"<!ELEMENT (%phrase;|SUB|SUP|SPAN|BDO|FONT) - - (%inline;)*>"
"<!ELEMENT BASEFONT - O EMPTY>"
"<!ELEMENT BR - O EMPTY>"
"<!ELEMENT BODY O O (%flow;)* +(INS|DEL)>"
"<!ELEMENT ADDRESS - - ((%inline;)|P)*>"
"<!ELEMENT DIV - - (%flow;)*>"
"<!ELEMENT CENTER - - (%flow;)*>"
"<!ELEMENT A - - (%inline;)* -(A)>"
"<!ELEMENT MAP - - ((%block;)+ | AREA+)>"
"<!ELEMENT AREA - O EMPTY>"
"<!ELEMENT LINK - O EMPTY>"
"<!ELEMENT IMG - O EMPTY>"
"<!ELEMENT OBJECT - - (PARAM | %flow;)*>"
"<!ELEMENT PARAM - O EMPTY>"
"<!ELEMENT APPLET - - (PARAM | %flow;)*>"
"<!ELEMENT HR - O EMPTY>"
"<!ELEMENT P - O (%inline;)*>"
"<!ELEMENT (%heading;) - - (%inline;)*>"
"<!ELEMENT PRE - - (%inline;)* -(%pre.exclusion;)>"
"<!ELEMENT Q - - (%inline;)*>"
"<!ELEMENT BLOCKQUOTE - - (%flow;)*>"
"<!ELEMENT (INS|DEL) - - (%flow;)*>"
"<!ELEMENT DL - - (DT|DD)+>"
"<!ELEMENT DT - O (%inline;)*>"
"<!ELEMENT DD - O (%flow;)*>"
"<!ELEMENT OL - - (LI)+>"
"<!ELEMENT UL - - (LI)+>"
"<!ELEMENT (DIR|MENU) - - (LI)+ -(%block;)>"
"<!ELEMENT LI - O (%flow;)*>"
"<!ELEMENT FORM - - (%flow;)* -(FORM)>"
"<!ELEMENT LABEL - - (%inline;)* -(LABEL)>"
"<!ELEMENT INPUT - O EMPTY>"
"<!ELEMENT SELECT - - (OPTGROUP|OPTION)+>"
"<!ELEMENT OPTGROUP - - (OPTION)+>"
"<!ELEMENT OPTION - O (#PCDATA)>"
"<!ELEMENT TEXTAREA - - (#PCDATA)>"
"<!ELEMENT FIELDSET - - (#PCDATA,LEGEND,(%flow;)*)>"
"<!ELEMENT LEGEND - - (%inline;)*>"
"<!ELEMENT BUTTON - - (%flow;)* -(A|%formctrl;|FORM|ISINDEX|FIELDSET|IFRAME)>"
"<!ELEMENT TABLE - - (CAPTION?, (COL*|COLGROUP*), THEAD?, TFOOT?, TBODY+)>"
"<!ELEMENT CAPTION - - (%inline;)*>"
"<!ELEMENT THEAD - O (TR)+>"
"<!ELEMENT TFOOT - O (TR)+>"
"<!ELEMENT TBODY O O (TR)+>"
"<!ELEMENT COLGROUP - O (col)*>"
"<!ELEMENT COL - O EMPTY>"
"<!ELEMENT TR - O (TH|TD)+>"
"<!ELEMENT (TH|TD|) - O (%flow;)*>"
"<!ELEMENT (IFRAME|NOFRAMES) - - (%flow;)*>"
"<!ELEMENT HEAD O O (TITLE & ISINDEX? & BASE?) +(%head.misc;)>"
"<!ELEMENT TITLE - - (#PCDATA) -(%head.misc;)>"
"<!ELEMENT ISINDEX - O EMPTY>"
"<!ELEMENT BASE - O EMPTY>"
"<!ELEMENT META - O EMPTY>"
"<!ELEMENT STYLE - - CDATA>"
"<!ELEMENT SCRIPT - - CDATA>"
"<!ELEMENT NOSCRIPT - - (%flow;)*>"
"<!ELEMENT HTML O O (HEAD, BODY)>";
#pragma pcrelconstdata reset


#else
static char const htmlDtd[] =
"<!ENTITY % HTML.Version \"-//W3C//DTD HTML 4.0 Transitional//EN\">"
"<!ENTITY % head.misc \"SCRIPT|STYLE|META|LINK|OBJECT\">"
"<!ENTITY % heading \"H1|H2|H3|H4|H5|H6\">"
"<!ENTITY % list \"UL | OL | DIR | MENU\">"
"<!ENTITY % preformatted \"PRE\">"
"<!ENTITY % Color \"CDATA\">"
"<!ENTITY % bodycolors \"bgcolor CDATA #IMPLIED text CDATA #IMPLIED link CDATA #IMPLIED vlink CDATA #IMPLIED alink CDATA #IMPLIED\">"
"<!ENTITY % coreattrs \"id ID #IMPLIED class CDATA #IMPLIED style CDATA #IMPLIED title CDATA #IMPLIED\">"
"<!ENTITY % i18n \"lang NAME #IMPLIED dir (ltr|rtl) #IMPLIED \">"
"<!ENTITY % events \"onclick CDATA #IMPLIED ondblclick CDATA #IMPLIED onmousedown CDATA #IMPLIED onmouseup CDATA #IMPLIED onmouseover CDATA #IMPLIED onmousemove CDATA #IMPLIED onmouseout CDATA #IMPLIED onkeypress CDATA #IMPLIED onkeydown CDATA #IMPLIED onkeyup CDATA #IMPLIED \">"
"<!ENTITY % reserved \"\">"
"<!ENTITY % attrs \"%coreattrs; %i18n; %events;\">"
"<!ENTITY % align \"align (left|center|right|justify) #IMPLIED\">"
"<!ENTITY % fontstyle \"TT | I | B | U | S | STRIKE | BIG | SMALL\">"
"<!ENTITY % phrase \"EM | STRONG | DFN | CODE | SAMP | KBD | VAR | CITE | ABBR | ACRONYM\">"
"<!ENTITY % special \"A | IMG | APPLET | OBJECT | FONT | BASEFONT | BR | SCRIPT | MAP | Q | SUB | SUP | SPAN | BDO | IFRAME\">"
"<!ENTITY % formctrl \"INPUT | SELECT | TEXTAREA | LABEL | BUTTON\">"
"<!ENTITY % inline \"#PCDATA | %fontstyle; | %phrase; | %special; | %formctrl;\">"
"<!ELEMENT (%fontstyle;|%phrase;) - - (%inline;)*>"
"<!ATTLIST (%fontstyle;|%phrase;) %attrs;>"
"<!ELEMENT (SUB|SUP) - - (%inline;)*>"
"<!ATTLIST (SUB|SUP) %attrs;>"
"<!ELEMENT SPAN - - (%inline;)*>"
"<!ATTLIST SPAN %attrs; %reserved;>"
"<!ELEMENT BDO - - (%inline;)*>"
"<!ATTLIST BDO %coreattrs; lang NAME #IMPLIED dir (ltr|rtl) #REQUIRED>"
"<!ELEMENT BASEFONT - O EMPTY>"
"<!ATTLIST BASEFONT id ID #IMPLIED size CDATA #REQUIRED color CDATA #IMPLIED face CDATA #IMPLIED>"
"<!ELEMENT FONT - - (%inline;)*>"
"<!ATTLIST FONT %coreattrs; %i18n; size CDATA #IMPLIED color CDATA #IMPLIED face CDATA #IMPLIED>"
"<!ELEMENT BR - O EMPTY>"
"<!ATTLIST BR %coreattrs; clear (left|all|right|none) none>"
"<!ENTITY % block \"P | %heading; | %list; | %preformatted; | DL | DIV | CENTER | NOSCRIPT | NOFRAMES | BLOCKQUOTE | FORM | ISINDEX | HR | TABLE | FIELDSET | ADDRESS\">"
"<!ENTITY % flow \"%block; | %inline;\">"
"<!ELEMENT BODY O O (%flow;)* +(INS|DEL)>"
"<!ATTLIST BODY %attrs; onload CDATA #IMPLIED onunload CDATA #IMPLIED background CDATA #IMPLIED %bodycolors;>"
"<!ELEMENT ADDRESS - - ((%inline;)|P)*>"
"<!ATTLIST ADDRESS %attrs;>"
"<!ELEMENT DIV - - (%flow;)*>"
"<!ATTLIST DIV %attrs; %align; %reserved;>"
"<!ELEMENT CENTER - - (%flow;)*>"
"<!ATTLIST CENTER %attrs;>"
"<!ENTITY % Shape \"(rect|circle|poly|default)\">"
"<!ENTITY % Coords \"CDATA\">"
"<!ELEMENT A - - (%inline;)* -(A)>"
"<!ATTLIST A  %attrs; charset CDATA #IMPLIED type CDATA #IMPLIED name CDATA #IMPLIED href CDATA #IMPLIED hreflang NAME #IMPLIED target CDATA #IMPLIED rel CDATA #IMPLIED rev CDATA #IMPLIED accesskey CDATA #IMPLIED shape %Shape; rect coords %Coords; #IMPLIED tabindex NUMBER #IMPLIED onfocus CDATA #IMPLIED onblur CDATA #IMPLIED>"
"<!ELEMENT MAP - - ((%block;)+ | AREA+)>"
"<!ATTLIST MAP %attrs; name CDATA #REQUIRED>"
"<!ELEMENT AREA - O EMPTY>"
"<!ATTLIST AREA %attrs; shape %Shape; rect coords %Coords; #IMPLIED href CDATA #IMPLIED target CDATA #IMPLIED nohref (nohref) #IMPLIED alt CDATA #REQUIRED tabindex NUMBER #IMPLIED accesskey CDATA #IMPLIED onfocus CDATA #IMPLIED onblur CDATA #IMPLIED>"
"<!ELEMENT LINK - O EMPTY>"
"<!ATTLIST LINK %attrs; charset CDATA #IMPLIED href CDATA #IMPLIED hreflang NAME #IMPLIED type CDATA #IMPLIED rel CDATA #IMPLIED rev CDATA #IMPLIED media CDATA #IMPLIED target CDATA #IMPLIED>"
"<!ENTITY % Length \"CDATA\">"
"<!ENTITY % MultiLength \"CDATA\">"
"<!ENTITY % MultiLengths \"CDATA\">"
"<!ENTITY % Pixels \"CDATA\">"
"<!ENTITY % IAlign \"(top|middle|bottom|left|right)\">"
"<!ELEMENT IMG - O EMPTY>"
"<!ATTLIST IMG %attrs; src CDATA #REQUIRED alt CDATA #REQUIRED longdesc CDATA #IMPLIED height %Length; #IMPLIED width %Length; #IMPLIED usemap CDATA #IMPLIED ismap (ismap) #IMPLIED align %IAlign; #IMPLIED border %Length; #IMPLIED hspace %Pixels; #IMPLIED vspace %Pixels; #IMPLIED>"
"<!ELEMENT OBJECT - - (PARAM | %flow;)*>"
"<!ATTLIST OBJECT %attrs; declare (declare) #IMPLIED classid CDATA #IMPLIED codebase CDATA #IMPLIED data CDATA #IMPLIED type CDATA #IMPLIED codetype CDATA #IMPLIED archive CDATA #IMPLIED standby CDATA #IMPLIED height %Length; #IMPLIED width %Length; #IMPLIED usemap CDATA #IMPLIED name CDATA #IMPLIED tabindex NUMBER #IMPLIED align %IAlign; #IMPLIED border %Length; #IMPLIED hspace %Pixels; #IMPLIED vspace %Pixels; #IMPLIED %reserved;>"
"<!ELEMENT PARAM - O EMPTY>"
"<!ATTLIST PARAM id ID #IMPLIED name CDATA #REQUIRED value CDATA #IMPLIED valuetype (DATA|REF|OBJECT) DATA type CDATA #IMPLIED>"
"<!ELEMENT APPLET - - (PARAM | %flow;)*>"
"<!ATTLIST APPLET %coreattrs; codebase CDATA #IMPLIED archive CDATA #IMPLIED code CDATA #IMPLIED object CDATA #IMPLIED alt CDATA #IMPLIED name CDATA #IMPLIED width %Length; #REQUIRED height %Length; #REQUIRED align %IAlign; #IMPLIED hspace %Pixels; #IMPLIED vspace %Pixels; #IMPLIED>"
"<!ELEMENT HR - O EMPTY>"
"<!ATTLIST HR %coreattrs; %events; align (left|center|right) #IMPLIED noshade (noshade) #IMPLIED size %Pixels; #IMPLIED width %Length; #IMPLIED>"
"<!ELEMENT P - O (%inline;)*>"
"<!ATTLIST P %attrs; %align;>"
"<!ELEMENT (%heading;) - - (%inline;)*>"
"<!ATTLIST (%heading;) %attrs; %align;>"
"<!ENTITY % pre.exclusion \"IMG|OBJECT|APPLET|BIG|SMALL|SUB|SUP|FONT|BASEFONT\">"
"<!ELEMENT PRE - - (%inline;)* -(%pre.exclusion;)>"
"<!ATTLIST PRE %attrs; width NUMBER #IMPLIED>"
"<!ELEMENT Q - - (%inline;)*>"
"<!ATTLIST Q %attrs; cite CDATA #IMPLIED>"
"<!ELEMENT BLOCKQUOTE - - (%flow;)*>"
"<!ATTLIST BLOCKQUOTE %attrs; cite CDATA #IMPLIED>"
"<!ELEMENT (INS|DEL) - - (%flow;)*>"
"<!ATTLIST (INS|DEL) %attrs; cite CDATA #IMPLIED datetime CDATA #IMPLIED>"
"<!ELEMENT DL - - (DT|DD)+>"
"<!ATTLIST DL %attrs; compact (compact) #IMPLIED>"
"<!ELEMENT DT - O (%inline;)*>"
"<!ELEMENT DD - O (%flow;)*>"
"<!ATTLIST (DT|DD) %attrs;>"
"<!ENTITY % OLStyle \"CDATA\">"
"<!ELEMENT OL - - (LI)+>"
"<!ATTLIST OL %attrs; type %OLStyle; #IMPLIED compact (compact) #IMPLIED start NUMBER #IMPLIED>"
"<!ENTITY % ULStyle \"(disc|square|circle)\">"
"<!ELEMENT UL - - (LI)+>"
"<!ATTLIST UL %attrs; type %ULStyle; #IMPLIED compact (compact) #IMPLIED>"
"<!ELEMENT (DIR|MENU) - - (LI)+ -(%block;)>"
"<!ATTLIST DIR %attrs; compact (compact) #IMPLIED>"
"<!ATTLIST MENU %attrs; compact (compact) #IMPLIED>"
"<!ENTITY % LIStyle \"CDATA\">"
"<!ELEMENT LI - O (%flow;)*>"
"<!ATTLIST LI %attrs; type %LIStyle; #IMPLIED value NUMBER #IMPLIED>"
"<!ELEMENT FORM - - (%flow;)* -(FORM)>"
"<!ATTLIST FORM %attrs; action CDATA #REQUIRED method (GET|POST) GET enctype CDATA \"application/x-www-form-urlencoded\" onsubmit CDATA #IMPLIED onreset CDATA #IMPLIED target CDATA #IMPLIED accept-charset CDATA #IMPLIED>"
"<!ELEMENT LABEL - - (%inline;)* -(LABEL)>"
"<!ATTLIST LABEL %attrs; for IDREF #IMPLIED accesskey CDATA #IMPLIED onfocus CDATA #IMPLIED onblur CDATA #IMPLIED>"
"<!ENTITY % InputType \"(TEXT | PASSWORD | CHECKBOX | RADIO | SUBMIT | RESET | FILE | HIDDEN | IMAGE | BUTTON)\">"
"<!ELEMENT INPUT - O EMPTY>"
"<!ATTLIST INPUT %attrs; type %InputType; TEXT name CDATA #IMPLIED value CDATA #IMPLIED checked (checked) #IMPLIED disabled (disabled) #IMPLIED readonly (readonly) #IMPLIED size CDATA #IMPLIED maxlength NUMBER #IMPLIED src CDATA #IMPLIED alt CDATA #IMPLIED usemap CDATA #IMPLIED tabindex NUMBER #IMPLIED accesskey CDATA #IMPLIED onfocus CDATA #IMPLIED onblur CDATA #IMPLIED onselect CDATA #IMPLIED onchange CDATA #IMPLIED accept CDATA #IMPLIED align %IAlign; #IMPLIED %reserved;>"
"<!ELEMENT SELECT - - (OPTGROUP|OPTION)+>"
"<!ATTLIST SELECT %attrs; name CDATA #IMPLIED size NUMBER #IMPLIED multiple (multiple) #IMPLIED disabled (disabled) #IMPLIED tabindex NUMBER #IMPLIED onfocus CDATA #IMPLIED onblur CDATA #IMPLIED onchange CDATA #IMPLIED %reserved;>"
"<!ELEMENT OPTGROUP - - (OPTION)+>"
"<!ATTLIST OPTGROUP %attrs; disabled (disabled) #IMPLIED label CDATA #REQUIRED>"
"<!ELEMENT OPTION - O (#PCDATA)>"
"<!ATTLIST OPTION %attrs; selected (selected) #IMPLIED disabled (disabled) #IMPLIED label CDATA #IMPLIED value CDATA #IMPLIED>"
"<!ELEMENT TEXTAREA - - (#PCDATA)>"
"<!ATTLIST TEXTAREA %attrs; name CDATA #IMPLIED rows NUMBER #REQUIRED cols NUMBER #REQUIRED disabled (disabled) #IMPLIED readonly (readonly) #IMPLIED tabindex NUMBER #IMPLIED accesskey CDATA #IMPLIED onfocus CDATA #IMPLIED onblur CDATA #IMPLIED onselect CDATA #IMPLIED onchange CDATA #IMPLIED %reserved;>"
"<!ELEMENT FIELDSET - - (#PCDATA,LEGEND,(%flow;)*)>"
"<!ATTLIST FIELDSET %attrs;>"
"<!ELEMENT LEGEND - - (%inline;)*>"
"<!ENTITY % LAlign \"(top|bottom|left|right)\">"
"<!ATTLIST LEGEND %attrs; accesskey CDATA #IMPLIED align %LAlign; #IMPLIED>"
"<!ELEMENT BUTTON - - (%flow;)* -(A|%formctrl;|FORM|ISINDEX|FIELDSET|IFRAME)>"
"<!ATTLIST BUTTON %attrs; name CDATA #IMPLIED value CDATA #IMPLIED type (button|submit|reset) submit disabled (disabled) #IMPLIED tabindex NUMBER #IMPLIED accesskey CDATA #IMPLIED onfocus CDATA #IMPLIED onblur CDATA #IMPLIED %reserved;>"
"<!ENTITY % TFrame \"(void|above|below|hsides|lhs|rhs|vsides|box|border)\">"
"<!ENTITY % TRules \"(none | groups | rows | cols | all)\">"
"<!ENTITY % TAlign \"(left|center|right)\">"
"<!ENTITY % cellhalign \"align (left|center|right|justify|char) #IMPLIED char CDATA #IMPLIED charoff %Length; #IMPLIED \">"
"<!ENTITY % cellvalign \"valign (top|middle|bottom|baseline) #IMPLIED\">"
"<!ELEMENT TABLE - - (CAPTION?, (COL*|COLGROUP*), THEAD?, TFOOT?, TBODY+)>"
"<!ELEMENT CAPTION - - (%inline;)*>"
"<!ELEMENT THEAD - O (TR)+>"
"<!ELEMENT TFOOT - O (TR)+>"
"<!ELEMENT TBODY O O (TR)+>"
"<!ELEMENT COLGROUP - O (col)*>"
"<!ELEMENT COL - O EMPTY>"
"<!ELEMENT TR - O (TH|TD)+>"
"<!ELEMENT (TH|TD) - O (%flow;)*>"
"<!ATTLIST TABLE %attrs; summary CDATA #IMPLIED width %Length; #IMPLIED border %Pixels; #IMPLIED frame %TFrame; #IMPLIED rules %TRules; #IMPLIED cellspacing %Length; #IMPLIED cellpadding %Length; #IMPLIED align %TAlign; #IMPLIED bgcolor CDATA #IMPLIED %reserved; datapagesize CDATA #IMPLIED>"
"<!ENTITY % CAlign \"(top|bottom|left|right)\">"
"<!ATTLIST CAPTION %attrs; align %CAlign; #IMPLIED>"
"<!ATTLIST COLGROUP %attrs; span NUMBER 1 width %MultiLength; #IMPLIED %cellhalign; %cellvalign;>"
"<!ATTLIST COL %attrs; span NUMBER 1 width %MultiLength; #IMPLIED %cellhalign; %cellvalign;>"
"<!ATTLIST (THEAD|TBODY|TFOOT) %attrs; %cellhalign; %cellvalign;>"
"<!ATTLIST TR %attrs; %cellhalign; %cellvalign; bgcolor CDATA #IMPLIED>"
"<!ENTITY % Scope \"(row|col|rowgroup|colgroup)\">"
"<!ATTLIST (TH|TD) %attrs; abbr CDATA #IMPLIED axis CDATA #IMPLIED headers IDREFS #IMPLIED scope %Scope; #IMPLIED rowspan NUMBER 1 colspan NUMBER 1 %cellhalign; %cellvalign; nowrap (nowrap) #IMPLIED bgcolor CDATA #IMPLIED width %Pixels; #IMPLIED height %Pixels; #IMPLIED>"
"<!ELEMENT IFRAME - - (%flow;)*>"
"<!ATTLIST IFRAME %coreattrs; longdesc CDATA #IMPLIED name CDATA #IMPLIED src CDATA #IMPLIED frameborder (1|0) 1 marginwidth %Pixels; #IMPLIED marginheight %Pixels; #IMPLIED scrolling (yes|no|auto) auto align %IAlign; #IMPLIED height %Length; #IMPLIED width %Length; #IMPLIED>"
"<!ENTITY % noframes.content \"(%flow;)*\">"
"<!ELEMENT NOFRAMES - - %noframes.content;>"
"<!ATTLIST NOFRAMES %attrs;>"
"<!ENTITY % head.content \"TITLE & ISINDEX? & BASE?\">"
"<!ELEMENT HEAD O O (%head.content;) +(%head.misc;)>"
"<!ATTLIST HEAD %i18n; profile CDATA #IMPLIED>"
"<!ELEMENT TITLE - - (#PCDATA) -(%head.misc;)>"
"<!ATTLIST TITLE %i18n>"
"<!ELEMENT ISINDEX - O EMPTY>"
"<!ATTLIST ISINDEX %coreattrs; %i18n; prompt CDATA #IMPLIED>"
"<!ELEMENT BASE - O EMPTY>"
"<!ATTLIST BASE href CDATA #IMPLIED target CDATA #IMPLIED>"
"<!ELEMENT META - O EMPTY>"
"<!ATTLIST META %i18n; http-equiv NAME #IMPLIED name NAME #IMPLIED content CDATA #REQUIRED scheme CDATA #IMPLIED>"
"<!ELEMENT STYLE - - CDATA>"
"<!ATTLIST STYLE %i18n; type CDATA #REQUIRED media CDATA #IMPLIED title CDATA #IMPLIED>"
"<!ELEMENT SCRIPT - - CDATA>"
"<!ATTLIST SCRIPT charset CDATA #IMPLIED type CDATA #REQUIRED language CDATA #IMPLIED src CDATA #IMPLIED defer (defer) #IMPLIED event CDATA #IMPLIED for CDATA #IMPLIED>"
"<!ELEMENT NOSCRIPT - - (%flow;)*>"
"<!ATTLIST NOSCRIPT %attrs;>"
"<!ENTITY % version \"version CDATA #FIXED '%HTML.Version;'\">"
"<!ENTITY % html.content \"HEAD, BODY\">"
"<!ELEMENT HTML O O (%html.content;)>"
"<!ATTLIST HTML %i18n; %version;>";
#endif

/*-----------------------------------------------------------------getHtmlDtd-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * getHtmlDtd() {
   return htmlDtd;
}

/*-----------------------------------------------------------getHtmlDtdLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int getHtmlDtdLength() {
   return sizeof htmlDtd - 1;
}

#endif  // COM_JAXO_YAXX_DENY_XML
/*===========================================================================*/
