/*
* $Id: yspenum.h,v 1.4 2002-02-28 10:17:17 pgr Exp $
*
*  enums for YSP:
*  SdclKeys         SGML reserved names
*  YaspType          enum of types of events returned by YSPnext...().
*  YaspOptName       enum of possible options passed to YSPsetOption.
*  YaspEntType       enum of types of entities, so caller can set which expand.
*  YaspEntClass      general, parameter, or special (dtd & document) entity.
*
*  grouped in a separate file:
*  an enum can not be "forward referenced" (in C++, its size varies)
*/

#ifndef TOKENUM_HEADER
#define TOKENUM_HEADER

/*--------------------------------------------------------------------SdclKeys-+
|  An enumeration of the SGML reserved names defined in clause 13.4.1.         |
|  Their numeric values are used whenever classes of SGML things are           |
|  needed, such as entity and declaration types, etc.                          |
|                                                                              |
|  (mapping of keywords to/from enums is managed by sdcl.c).                   |
+-----------------------------------------------------------------------------*/
typedef enum SdclKeys_ {
   KEY_error,                   /* Error value */
   KEY_ANY,      KEY_ATTLIST,   KEY_CONREF,    KEY_CURRENT,   KEY_DEFAULT,
   KEY_DOCTYPE,  KEY_ELEMENT,   KEY_EMPTY,     KEY_ENDTAG,    KEY_FIXED,
   KEY_IDLINK,   KEY_IGNORE,    KEY_IMPLIED,   KEY_INCLUDE,   KEY_INITIAL,
   KEY_LINK,     KEY_LINKTYPE,  KEY_MD,        KEY_MS,        KEY_NDATA,
   KEY_O,        KEY_PCDATA,    KEY_PI,        KEY_POSTLINK,  KEY_PUBLIC,
   KEY_RCDATA,   KEY_RE,        KEY_REQUIRED,  KEY_RESTORE,   KEY_RS,
   KEY_SDATA,    KEY_SHORTREF,  KEY_SIMPLE,    KEY_SPACE,     KEY_STARTTAG,
   KEY_SUBDOC,   KEY_SYSTEM,    KEY_TEMP,      KEY_USELINK,   KEY_USEMAP,

   /* entity declared value */
   KEY_CDATA,    KEY_ENTITIES,  KEY_ENTITY,    KEY_ID,        KEY_IDREF,
   KEY_IDREFS,   KEY_NOTATION,  KEY_NAME,      KEY_NAMES,     KEY_NMTOKEN,
   KEY_NMTOKENS, KEY_NUMBER,    KEY_NUMBERS,   KEY_NUTOKEN,   KEY_NUTOKENS,

   /* Anything after here isn't a real SGML reserved word, but is
   |  included in the enum so we can distinguish special cases in the
   |  code using just one big enum, instead of lots of special ones.
   |  For example, there's no keyword for plain entities, but we
   |  have an entry here under (note lower-case) 'KEY_literal'.
   */
   KEY_enumerated,      /* special:  enumerated attr value     */
   KEY_literal,         /* special:  untyped entity            */
   KEY_model,           /* element has model, not dcld content.*/
   KEY_number_of_keys   /* list terminator */
}SdclKeys;


/*---------------------------------------------------------------------YaspType-+
| The kinds of events which can be returned by YSPnext().  The caller          |
| should switch on these and do its application-dependent processing.          |
+-----------------------------------------------------------------------------*/
typedef enum YaspType_ {
   YSP__noType,
   YSP__TAG_START,
   YSP__TAG_END,
   YSP__TAG_EMPTY,

   YSP__TEXT,

   YSP__ENTITY_REF,
   YSP__ENTITY_START,
   YSP__ENTITY_END,

   YSP__ENTITY_CANT,
   YSP__ERROR,
   YSP__PARSE_START,
   YSP__PARSE_END,

/* Following should only occur in the DTD, and define
*  objects needed to parse the following document.
*/
   YSP__DCL_PENTITY,
   YSP__DCL_ENTITY,
   YSP__DCL_ELEMENT,
   YSP__DCL_ATTRIBUTE,
   YSP__DCL_NOTATION,
   YSP__DCL_NULL,

   YSP__COMMENT_START,
   YSP__COMMENT_TEXT,
   YSP__COMMENT_END,

   YSP__MARKED_START,
   YSP__MARKED_END,

   YSP__DCL_START,
   YSP__DCL_END,

   YSP__PI_START,
   YSP__PI_END,

   YSP__NDATA_START,
   YSP__NDATA_END,

   YSP__SDATA_START,
   YSP__SDATA_END,
   YSP__SUBDOC_START,
   YSP__SUBDOC_END,
   YSP__DCL_ELEMENT_IMPLIED,  /* implied by attlist or content model */
   YSP__ERROR_FATAL,
   YSP__lastType,

   YSP__RECORD_END,           /* unused */
   YSP__SDATA_TEXT,
   YSP__CDATA_START,
   YSP__CDATA_TEXT,
   YSP__CDATA_END,
   YSP__PI_TEXT,
   YSP__PENTITY_START,
   YSP__DCL_NATTRIBUTE,
   YSP__ENT_PI_START,
   YSP__ENT_PI_END,
   YSP__XML_PI,
   YSP__numTypes
}YaspType;

/*----------------------------------------------------------------YaspOptName-+
|  The following enumerated type provides names which may be passed           |
|  (among other places) to YSPsetOption() to change SGML parsing settings.    |
+----------------------------------------------------------------------------*/
typedef enum YaspOptName_ {
   /* Application-specific options.  (values 0-23) */
   YSP__noOption     = 0,
   YSP__longTagNames = 1,        /* return tag context? */
   YSP__killRERS     = 2,        /* newlines to spaces? */
   YSP__killTAB      = 3,        /* tabs to spaces? */
   YSP__killLEAD     = 4,        /* kill lead spaces? */
   YSP__killTRAIL    = 5,        /* kill trail spaces? */
   /* __specialPI    = 6,           -- UNUSED */
   YSP__allowWildDecl      =  7, /* allow decls to occur in CON mode */
   YSP__reportOMITS        =  8, /* -- UNUSED (use YSP__validate) */
   YSP__HTML               =  9, /* allow for HTML syntax errs */
   YSP__reportEntityEvents = 10, /* inform about ent end and start */
   YSP__reportImpliedElems = 11, /* -- UNUSED */
   YSP__includeIgnoredMS   = 12, /* so ignored MS (from %ent) got included */
   YSP__noEndElementAtEof  = 13, /* do not clean the element stack at EOF */
   YSP__validate           = 14, /* report all validation errors */
   YSP__keepLineBreak      = 15, /* keep line breaks: do not apply 7.6.1 */
   /* __unused             = 16,    -- UNUSED */
   YSP__keepUcCharRef      = 17, /* &u-1234; is a regular (SDATA) entity */
   YSP__synchroElmWithEnt  = 18, /* elements start/end within same entity */

   /* Case options.  (combined with next group, values 24-31)  */
   YSP__nameCase_E   = 24,       /* (SGML features) case ignored - entities */
   YSP__nameCase_G   = 25,       /* (SGML features) case ignored - general */

   /* System declaration information */
   /* 15.6.1.1: Concrete syntax changes */
   YSP__S_SWITCHES   = 26,
   YSP__S_SEQUENCE   = 27,

   /* SGML feature settings, per section 13.5 (values 32-47) */
   YSP__F_DATATAG    = 32,       /* (SGML features) */
   YSP__F_OMITTAG    = 33,       /* (SGML features) */
   YSP__F_RANK       = 34,       /* (SGML features) */
   YSP__F_SHORTTAG   = 35,       /* (SGML features) */
   YSP__F_SIMPLE     = 36,       /* (SGML features) */
   YSP__F_IMPLICIT   = 37,       /* (SGML features) */
   YSP__F_EXPLICIT   = 38,       /* (SGML features) */
   YSP__F_CONCUR     = 39,       /* (SGML features) */
   YSP__F_SUBDOC     = 40,       /* (SGML features) */
   YSP__F_FORMAL     = 41,       /* (SGML features) */

   /* 15.6.2: Validation services (values 48-63) */
   YSP__V_GENERAL    = 48,
   YSP__V_MODEL      = 49,
   YSP__V_EXCLUDE    = 50,
   YSP__V_CAPACITY   = 51,
   YSP__V_NONSGML    = 52,
   YSP__V_SGML       = 53,
   YSP__V_FORMAL     = 54
}YaspOptName;

/*-------------------------------------------------------------------e_EntKind-+
| The kinds of entities known to the entity manager (input.c), such as         |
| CDATA, SDATA, NDATA, and PI.                                                 |
+-----------------------------------------------------------------------------*/
typedef enum e_EntKind_ {
   ENTK_INVALID = 0,    /* Invalid */
   ENTK_CDATA,          /* not_rcdata | Ext/Int | notation iff Ext */
   ENTK_SDATA,          /* not_rcdata | Ext/Int | notation iff Ext */
   ENTK_MD,             /*   rcdata   |   Int   |    bracketed     */
   ENTK_PI,             /* not_rcdata |   Int   |    bracketed     */
   ENTK_STARTTAG,       /*   rcdata   |   Int   |    bracketed     */
   ENTK_ENDTAG,         /*   rcdata   |   Int   |    bracketed     */
   ENTK_MS,             /*   rcdata   |   Int   |    bracketed     */
   ENTK_NDATA,          /* not_rcdata |   Ext   |    notation      */
   ENTK_TEXT,           /*   rcdata   |   Int   |                  */
   ENTK_SGML,           /*   rcdata   |   Ext   |                  */
   ENTK_SUBDOC,         /* not_rcdata |   Ext   |                  */

   ENTK_last            /* (not a kind, should be last entry) */
}e_EntKind;

/*-----------------------------------------------------------------YaspEntClass-+
| The basic classes of entities: general vs. parameter, plus special           |
| ones for document, dtd, and other reserved entities.                         |
+-----------------------------------------------------------------------------*/
typedef enum YaspEntClass_ {
   ENT_GENERAL    = 'G',
   ENT_PARAMETER  = 'P',
   ENT_SPECIAL    = 'S'
}YaspEntClass;

/*-------------------------------------------------------------------e_TxtType-+
| Values for grabSpecificText function                                         |
+-----------------------------------------------------------------------------*/
typedef enum e_TxtType_ {
   TXTTYPE_MINLIT,
   TXTTYPE_LIT,
   TXTTYPE_PI
}e_TxtType;

/*----------------------------------------------------------------ElementClass-+
| ElementClass                                                                 |
|   Represents what kind of content model or declared content an               |
|   element has.   Legitimate values: error, CDATA, RCDATA, ANY, model.        |
+-----------------------------------------------------------------------------*/
typedef SdclKeys ElementClass;

/*-----------------------------------------------------------------e_AttDclVal-+
| Declared values for an attribute (see 11.3.3).                               |
+-----------------------------------------------------------------------------*/
typedef enum e_AttDclVal_ {
   ADCLV_INVALID,               /* None = Invalid */
   ADCLV_CDATA,                 /* character data */
   ADCLV_ENTITY,                /* general entity name */
   ADCLV_ENTITIES,              /* general entity name list */
   ADCLV_ID,                    /* an ID */
   ADCLV_IDREF,                 /* an ID reference */
   ADCLV_IDREFS,                /* an ID reference list */
   ADCLV_NAME,                  /* a name */
   ADCLV_NAMES,                 /* a name list */
   ADCLV_NMTOKEN,               /* a name token */
   ADCLV_NMTOKENS,              /* a name token list */
   ADCLV_NOTATION,              /* notation  (followed by a name group) */
   ADCLV_NUMBER,                /* a number */
   ADCLV_NUMBERS,               /* a number list */
   ADCLV_NUTOKEN,               /* a number token */
   ADCLV_NUTOKENS,              /* a number token list */
   ADCLV_ENUM                   /* name group (one of a fixed enumerated set) */
}e_AttDclVal;

/*-----------------------------------------------------------------e_AttDftVal-+
| Values for an attribute's default.                                           |
+-----------------------------------------------------------------------------*/
typedef enum e_AttDftVal_ {
   ADFTV_INVALID = 0,           /* None = Invalid */
   ADFTV_IMPLIED,               /* Impliable attribute */
   ADFTV_REQUIRED,              /* This attribute is REQUIRED  <= 1 => ID */
   ADFTV_CURRENT,               /* Default attribute is CURRENT value */
   ADFTV_CONREF,                /* Content Reference Attribute */
   ADFTV_DEFLT,                 /* >= A default value has been specified */
   ADFTV_FIXED                  /* This default value is fixed */
}e_AttDftVal;

/*---------------------------------------------------------------------e_MsKey-+
| Values for a Marked Section key                                              |
+-----------------------------------------------------------------------------*/
typedef enum e_MsKey_ {
   MSKEY_INVALID = 0,
   MSKEY_TEMP    = 1,
   MSKEY_INCLUDE = 2,
   MSKEY_RCDATA  = 3,
   MSKEY_CDATA   = 4,
   MSKEY_IGNORE  = 5
}e_MsKey;

/*------------------------------------------------------------------e_FctClass-+
| Function Character classes                                                   |
+-----------------------------------------------------------------------------*/
typedef enum e_FctCls_ {
   FCTCLS_RE,
   FCTCLS_RS,
   FCTCLS_SPACE,
   FCTCLS_FUNCHAR,
   FCTCLS_MSICHAR,
   FCTCLS_MSOCHAR,
   FCTCLS_MSSCHAR,
   FCTCLS_SEPCHAR,
   FCTCLS_INVALID
}e_FctCls;

/*----------------------------------------------------------------------e_Ptxc-+
| Formal Identifier: Public Text Classes                                       |
+-----------------------------------------------------------------------------*/
typedef enum e_Ptxc_ {
   PTXC_INVALID,
   PTXC_CAPACITY,
   PTXC_CHARSET,
   PTXC_DOCUMENT,
   PTXC_DTD,
   PTXC_ELEMENTS,
   PTXC_ENTITIES,
   PTXC_LPD,
   PTXC_NONSGML,
   PTXC_NOTATION,
   PTXC_SHORTREF,
   PTXC_SUBDOC,
   PTXC_SYNTAX,
   PTXC_TEXT
}e_Ptxc;
#endif /* TOKENUM_HEADER =====================================================*/
