/*
* $Id: yspparse.cpp,v 1.6 2002-02-23 17:27:47 pgr Exp $
*
* Parse large SGML constructs like markup declarations to the final MDC.
*/

/*---------+
| Includes |
+---------*/
#include "../yasp3.h"
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"

/*----------------------------------------------------------parseContentModel-+
| Parse a content model (ISO 8879 - p40)                                      |
|                                                  |                          |
| [127] model_group = GRPO, ts*,content_token,ts*, V , GRPC,occurence_indic?  |
|                           (connector, ts*,content_token,ts* )*              |
|                                                                             |
| At ENTRY: current char == GRPO;  (no check, start AFTER: at ts*)            |
|           recognitionMode = MODE_GROUP                                      |
| Return when an error occured.                                               |
+----------------------------------------------------------------------------*/
bool Yasp3::parseContentModel(ModelTree & model)
{
   if (model.tryInsert(Delimiter::IX_GRPO) == ModelTree::OK) {
      for (;;) {
         skip_ts();
         switch (dlmfnd.delimFound()) {
         case Delimiter::IX_NOT_FOUND:
            {
               UCS_2 giName[1+NAMELEN_MAX];
               Element const * pElm;
               if (
                  !grabNameGeneral(giName) ||
                  (pElm = elmMgr.defineElementIfNotThere(giName), !pElm) ||
                  (model.tryInsert(pElm->inqIx()) != ModelTree::OK)
               ) {
                  break;
               }
            }
            continue;
         case Delimiter::IX_RNI:
            if (!skip(ReservedName::IX_PCDATA) ||
               (model.tryInsert(-1) != ModelTree::OK)
            ) {
               break;
            }
            continue;
         case Delimiter::IX_GRPC:
            if (model.tryInsert(Delimiter::IX_GRPC) != ModelTree::OK) {
               break;
            }
            if (model.status() == ModelTree::OCC_OR_END_EXPECTED) {
               if ((dlmfnd.find(oec) != Delimiter::IX_NOT_FOUND) &&
                  (model.tryInsert(dlmfnd.delimFound()) != ModelTree::OK)
               ) {
                 break;
               }
               model.tryEnd();
               return true;
            }
            continue;
         default:
            //>>PGR: if the delimiter is an occurence indicator,
            //       we should check that skip_ts didn't skip any ts
            if (model.tryInsert(dlmfnd.delimFound()) != ModelTree::OK) {
               break;
            }
            continue;
         }
         oec.backup(dlmfnd.delimLength());  // backup: not for us!
         break;
      }
   }
   // if we land here, something turned wrong...
   model = ModelTree::Undefined;
   return false;
}

/*--------------------------------------------------------parseElementContent-+
| Effects:                                                                    |
|   Parse an element_content.                                                 |
|                                                                             |
|   element_content = (declared_content | content_model)                      |
|                                                                             |
|   [125] declared_content = "CDATA" | "RCDATA" | "EMPTY"                     |
|   [126] content_model = (model_group | "ANY"), (ps+, exceptions)?           |
|                                                                             |
| Requires:                                                                   |
|   Expects the stream to be pointing right where the element_content         |
|   should start.  Push and pop GRP mode here, not in caller.                 |
|                                                                             |
| NOTES:                                                                      |
|   Scan returns where the first character not part of an element_content     |
|   occurs, or where an error has been detected.                              |
+----------------------------------------------------------------------------*/
bool Yasp3::parseElementContent(ModelTree & model)
{
   static ReservedName::Choice const aChoicesElmContent[] = {
      ReservedName::Choice(ReservedName::IX_CDATA,     ModelTree::CDATA  ),
      ReservedName::Choice(ReservedName::IX_RCDATA,    ModelTree::RCDATA ),
      ReservedName::Choice(ReservedName::IX_EMPTY,     ModelTree::EMPTY  ),
      ReservedName::Choice(ReservedName::IX_ANY,       ModelTree::ANY    ),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND, ModelTree::INVALID)
   };

   switch (dlmfnd.delimFound()) {
   case Delimiter::IX_GRPO:
      {
         bool isOk;
         dlmfnd.pushMode(MODE_GRP);
         isOk = parseContentModel(model);
         dlmfnd.popMode();
         return isOk;
      }
   case Delimiter::IX_NOT_FOUND:         // declared content
      {
         ModelTree::e_DeclaredClass dcls(
             (ModelTree::e_DeclaredClass)grabChoice(aChoicesElmContent)
         );
         if (dcls !=  ModelTree::INVALID) {
            model << dcls << endm;
            return true;
         }
      }
      break;
   default:
      break;
   }
   return false;
}

/*------------------------------------------------------------NameGrabGroupCB-+
| Class used by parseNameGroup, parseExceptionGroup                           |
+----------------------------------------------------------------------------*/
class NameGrabGroupCB : public Yasp3::GrabGroupCB {
   friend bool Yasp3::parseNameGroup(TpIxList & ilist, bool isNotation);
   friend bool Yasp3::parseExceptionGroup(TpIxList & ilist);

private:
   NameGrabGroupCB(TpIxList & ilistArg, bool isNotationArg) :
      ilist(ilistArg), isNotation(isNotationArg) {
   }
   bool call(Yasp3 * pYasp);
   TpIxList & ilist;
   bool isNotation;
};

bool NameGrabGroupCB::call(Yasp3 * pYasp)
{
   UCS_2 pUcName[1+NAMELEN_MAX];
   if (!pYasp->grabNameGeneral(pUcName)) {
      return false;
   }
   if (!isNotation) {
      Element const * pElm = pYasp->elmMgr.defineElementIfNotThere(pUcName);
      if (pElm) {
         ilist += pElm->inqIx();
      }
   }else {
      Notation const * pNcb = pYasp->entMgr.defineNotationIfNotThere(pUcName);
      if (pNcb) {
         //>>>PGR: no index in a notation... but soon or later, PDT will need it
         ilist += pYasp->entMgr.inqListNotation().ix(pNcb->key());
      }
   }
   return true;
}

/*-------------------------------------------------------------parseNameGroup-+
| Parse an enumeration of names (name group)                                  |
+----------------------------------------------------------------------------*/
bool Yasp3::parseNameGroup(TpIxList & ilist, bool isNotation)
{
    NameGrabGroupCB groupCB(ilist, isNotation);
    bool isOk = false;

    switch (dlmfnd.delimFound()) {
    case Delimiter::IX_GRPO:
       grabGroup(groupCB);
       if (dlmfnd.delimFound() == Delimiter::IX_GRPC) {
          isOk = true;
       }
       break;
    case Delimiter::IX_NOT_FOUND:
       isOk = groupCB.call(this);
       break;
    default:
       break;
    }
    return isOk;
}


/*--------------------------------------------------------parseExceptionGroup-+
| Called by grabGroup during to parse an enumeration of elements exceptions.  |
| The MINUS_GRPO or PLUS_GRPO must have been recognized first.                |
+----------------------------------------------------------------------------*/
bool Yasp3::parseExceptionGroup(TpIxList & ilist)
{
    NameGrabGroupCB groupCB(ilist, false);   // isNotation is FALSE
    bool isOk = false;

    grabGroup(groupCB);
    if (dlmfnd.delimFound() == Delimiter::IX_GRPC) {
       isOk = true;
    }
    return isOk;
}


/*-----------------------------------------------------------parseElementDecl-+
| Parse an element declaration, starting just after "<!ELEMENT".              |
|                                                                             |
| [116] element declaration = mdo,"ELEMENT",                                  |
|       ps+,element_type,(ps+, omitted tag minimization)?,                    |
|       ps+,(declared_content | content_model), ps*                           |
|       mdc                                                                   |
+----------------------------------------------------------------------------*/
void Yasp3::parseElementDecl(YaspEvent & ev)
{
   ev = YSP__ERROR;
   bool omitStart = false;
   bool omitEnd = false;

   TpIxList elements;
   TpIxList inclusions;
   TpIxList exclusions;
   ModelTree model;

   /*---------------------------------------------------+
   | Grab the entire tag-name or name group; latter is  |
   | parsed at end, just before adding the definition   |
   | to the element library                             |
   |                                                    |
   |[117] element_type = generic_identifier |           |
   |     name_group | ranked_element | ranked_group     |
   +---------------------------------------------------*/
   skip_ps();
   if (!parseNameGroup(elements)) {
      bypassDecl();
      return;                             // should be at MDC now...
   }

   /*-----------------------+
   |Omitted tag minimization|
   +-----------------------*/
   skip_ps();
   {
      int iTimes = 0;
      while (dlmfnd.delimFound() == Delimiter::IX_NOT_FOUND) {
         if (skip(ReservedName::IX_O)) {             // "O" found
            if (iTimes == 0) {                       // first time:
               omitStart = true;                     // for start tag
               skip_psRequired();                    // skip'n check PS
            }else {
               omitEnd = true;                       // for end tag
               skip_ps();                            // skip PS if any
            }
         }else {                                     // no "O" found
            if (!skip(Delimiter::IX_MINUS)) {        // neither MINUS
               break;                                // break loop
            }
            skip_ps();                               // skip PS if any
         }
         if (++iTimes == 2) break;
      }
      if (
         ((!iTimes && sdcl.features().isOmittag()) || (iTimes == 1)) &&
         (options == YSP__validate)
      ) {
         erh << ECE__ERROR << _YSP__INVMINOMIT << endm;
      }
   }

   /*----------------------+
   | parse element content |
   +----------------------*/
   if (!parseElementContent(model)) {
      erh << ECE__ERROR << _YSP__BADMODEL
          << elmMgr.inqElement(elements.inqIx(0))->key()
          << endm;
      bypassDecl();          // try to recover?
   }else {
      skip_ps();
      /*------------------------------------------------------------+
      | Decode exceptions                                           |
      | [138] exceptions = (exclusions,(ps+,inclusions)?|inclusions |
      | [140] exclusions = MINUS, name group                        |
      | [139] inclusions = PLUS, name group                         |
      +------------------------------------------------------------*/
      for (int iTimes = 0; iTimes < 2; ++iTimes) {
         if (iTimes == 0) {         // Exclusions first if present
            if (dlmfnd.delimFound() != Delimiter::IX_MINUS_GRPO) continue;
            if (!parseExceptionGroup(exclusions)) {
               bypassDecl();
               break;                              // should be at MDC now...
            }
         }else {
            if (dlmfnd.delimFound() != Delimiter::IX_PLUS_GRPO) continue;
            if (!parseExceptionGroup(inclusions)) {
               bypassDecl();
               break;                              // should be at MDC now...
            }
         }
         skip_ps();
      }
   }

   /* Now we have:
   | - the element list
   | - the omission flags
   | - the model
   | - the exceptions (or NULL's).
   | Add the declaration(s) to the database, as much as possible
   */
   {
      ElmContent content(model, inclusions, exclusions);
      int const iLast = elements.count();
      for (int i=0; i < iLast; ++i) {
         Element * pElm = elmMgr.defineElement(
            elements.inqIx(i), content, omitStart, omitEnd
         );
         if (pElm) {
            if (ev.isPresent()) {
               evlst += YaspEvent(pElm, YSP__DCL_ELEMENT);
            }else {
               ev = YaspEvent(pElm, YSP__DCL_ELEMENT);
            }
         }else if (options == YSP__validate) {
            erh << ECE__ERROR << _YSP__DUPGI
                << elmMgr.inqElement(elements.inqIx(i))->key()
                << endm;
         }
      }
   }
}

/*---------------------------------------------------------AttDclvGrabGroupCB-+
| Called by grabGroup during the parse of an enumeration of declared          |
| values, for each new value.                                                 |
| Make a few checks.  If everything is OK, then enter the new value           |
| into the arglEnum of the current attribute.                                 |
| Return false to tell grabGroup to bypass the enumeration -- aka name group  |
+----------------------------------------------------------------------------*/
class AttDclvGrabGroupCB : public Yasp3::GrabGroupCB {
public:
   AttDclvGrabGroupCB(Attlist & attlstArg, Attribute * pAttArg) :
      attlst(attlstArg), pAtt(pAttArg) {
   }
   bool call(Yasp3 * pYasp);
   Attlist & attlst;
   Attribute * pAtt;
};
bool AttDclvGrabGroupCB::call(Yasp3 * pYasp)
{
   /* Get the token - rules vary dep. on the declared value
   */
   UCS_2 pUcValue[1+NAMELEN_MAX];
   int i;
   if (pAtt->inqTypeDeclaredValue() ==  ADCLV_NOTATION) {
      i = pYasp->grabNameGeneral(pUcValue);
   }else {
      i = pYasp->grabNameToken(pUcValue);
   }
   if (!i) {
      return false;
   }
   /* check if this name already belongs to an enum in the list
   */
   if (attlst.findToken(pUcValue) && (pYasp->options == YSP__validate)) {
      pYasp->erh << ECE__ERROR << _YSP__DUPDCLV << pUcValue << endm;
   }
   /* all right.  enter the value into the arglEnum of the attribute
   */
   pAtt->addToken(pUcValue);
   return true;
}

/*-----------------------------------------------------------parseAttlistDecl-+
| Process ATTLIST MD (Attribute Definition List Declaration)                  |
|                                                                             |
| Enter here when ATTLIST has been decoded,                                   |
| Returns with curr_char (supposed to ) == MDC                                |
|                                                                             |
| [141] attribute definition list declaration =                               |
|         mdo, "ATTLIST", ps+,                                                |
|         (associated element type | associated notation name), ps+           |
|         attribute definition list,                                          |
|         ps*, mdc)                                                           |
|                                                                             |
|                                                                             |
| [142] attribute definition list = attribute definition,                     |
|                 (ps+, attribute definition)*                                |
|                                                                             |
| [143] attribute definition =                                                |
|                 attribute name, ps+, declared value, ps+, default value     |
|                                                                             |
| [144] attribute name = name                                                 |
|                                                                             |
| [145] declared value = "CDATA" | "ENTITY" | "ENTITIES" |                    |
| [146]                  "ID" | "IDREF" | "IDREFS" |                          |
|                        "NAME" | "NAMES" | "NMTOKEN" | "NMTOKENS" |          |
|                        "NUMBER" | "NUMBERS" | "NUTOKEN" | "NUTOKENS" |      |
|                        ("NOTATION", ps+, name group) | name token group     |
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::parseAttlistDecl(YaspEvent & ev)
{
   static ReservedName::Choice const aChoicesAdftv[] = {
      ReservedName::Choice(ReservedName::IX_FIXED,     ADFTV_FIXED    ),
      ReservedName::Choice(ReservedName::IX_REQUIRED,  ADFTV_REQUIRED ),
      ReservedName::Choice(ReservedName::IX_CURRENT,   ADFTV_CURRENT  ),
      ReservedName::Choice(ReservedName::IX_CONREF,    ADFTV_CONREF   ),
      ReservedName::Choice(ReservedName::IX_IMPLIED,   ADFTV_IMPLIED  ),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND, ADFTV_INVALID  )
   };
   static ReservedName::Choice const aChoicesAdclv[] = {
      ReservedName::Choice(ReservedName::IX_CDATA,     ADCLV_CDATA    ),
      ReservedName::Choice(ReservedName::IX_ENTITY,    ADCLV_ENTITY   ),
      ReservedName::Choice(ReservedName::IX_ENTITIES,  ADCLV_ENTITIES ),
      ReservedName::Choice(ReservedName::IX_ID,        ADCLV_ID       ),
      ReservedName::Choice(ReservedName::IX_IDREF,     ADCLV_IDREF    ),
      ReservedName::Choice(ReservedName::IX_IDREFS,    ADCLV_IDREFS   ),
      ReservedName::Choice(ReservedName::IX_NAME,      ADCLV_NAME     ),
      ReservedName::Choice(ReservedName::IX_NAMES,     ADCLV_NAMES    ),
      ReservedName::Choice(ReservedName::IX_NMTOKEN,   ADCLV_NMTOKEN  ),
      ReservedName::Choice(ReservedName::IX_NMTOKENS,  ADCLV_NMTOKENS ),
      ReservedName::Choice(ReservedName::IX_NOTATION,  ADCLV_NOTATION ),
      ReservedName::Choice(ReservedName::IX_NUMBER,    ADCLV_NUMBER   ),
      ReservedName::Choice(ReservedName::IX_NUMBERS,   ADCLV_NUMBERS  ),
      ReservedName::Choice(ReservedName::IX_NUTOKEN,   ADCLV_NUTOKEN  ),
      ReservedName::Choice(ReservedName::IX_NUTOKENS,  ADCLV_NUTOKENS ),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND, ADCLV_INVALID  )
   };

   ev = YSP__ERROR;
   AttlistFlag flag;
   /*-----------------------------------------------------+
   | Decode associated element / notation:                |
   | [149.1] associated notation name =                   |
   |    rni, "NOTATION, ps+, (notation name | name group) |
   | [72] associated element type =                       |
   |    generic identifier | name group                   |
   +-----------------------------------------------------*/
   bool isDataAttlist;
   TpIxList associateds;          // elements,  or notations if isDataAttlist
   skip_ps();
   if (dlmfnd.delimFound() ==  Delimiter::IX_RNI) {
      if (!skip(ReservedName::IX_NOTATION)) {
         erh << ECE__ERROR << _YSP__INVKEYWD1
             << name(ReservedName::IX_NOTATION)
             << endm;
         bypassDecl();
         return;
      }
      isDataAttlist = true;
      skip_ps();
   }else {
      isDataAttlist = false;
   }
   if (!parseNameGroup(associateds, isDataAttlist)) {
      bypassDecl();
      return;
   }

   /*------------------------------------+
   | Enroll all attributes in an Attlist |
   +------------------------------------*/
   Attlist attlst;

   while (skip_ps(), dlmfnd.delimFound() == Delimiter::IX_NOT_FOUND) {
      /*---------------+
      | Attribute Name |
      +---------------*/
      UCS_2 pUcName[1+NAMELEN_MAX];
      if (!grabNameGeneral(pUcName)) {
         bypassDecl();
         return;
      }
      if (attlst.count() >= 1000) {
         erh << ECE__ERROR << _YSP__BIGATTRIB << endm;
         bypassDecl();
         return;
      }
      /*---------------+
      | Declared Value |
      +---------------*/
      e_AttDclVal dclVal;
      skip_ps();
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_GRPO:
         dclVal = ADCLV_ENUM;
         break;
      case Delimiter::IX_NOT_FOUND:
         dclVal = (e_AttDclVal)grabChoice(aChoicesAdclv);
         switch (dclVal) {
         case ADCLV_ENTITY:
         case ADCLV_ENTITIES:
         case ADCLV_IDREF:
         case ADCLV_IDREFS:
            if (isDataAttlist) {
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__INVADCLV << endm;
               }
               if ((dclVal == ADCLV_ENTITY) || (dclVal == ADCLV_IDREF)) {
                  dclVal = ADCLV_NAME;
               }else {
                  dclVal = ADCLV_NAMES;
               }
            }else  {
               flag.entity = true;
            }
            break;
         case ADCLV_NOTATION:
            if (flag.notation) {
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__DUPNOTDEF << endm;
               }
               dclVal = ADCLV_ENUM;               // Treat as a enum
            }else if (isDataAttlist) {            // See 11.4.1
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__INVADCLV << endm;
               }
               dclVal = ADCLV_ENUM;               // Treat as a enum
            }else {
               flag.notation = true;              // no more than 1!
            }
            skip_ps();
            if (dlmfnd.delimFound() != Delimiter::IX_GRPO) {
               erh << ECE__ERROR << _YSP__INVATTDCL << endm;
               bypassDecl();
               return;
            }
            break;
         case ADCLV_ID:
            if (flag.id) {
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__DUPIDDEF << endm;
               }
               dclVal = ADCLV_NAME;               // Treat as a name
            }else if (isDataAttlist) {            // See 11.4.1
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__INVADCLV << endm;
               }
               dclVal = ADCLV_NAME;               // Treat as a name
            }else {
               flag.id = true;                    // no more than 1!
            }
            break;
         case ADCLV_INVALID:
            bypassDecl();
            return;
         default:                                 // Others: keep type
            break;
         }
         break;
      default:
         erh << ECE__ERROR << _YSP__INVATTDCL << endm;
         bypassDecl();
         return;
      }
      /*------------------------------+
      | Time to create the attribute  |
      | pUcName has the name,         |
      | dclVal has the declared value |
      +------------------------------*/
      if (attlst[pUcName]) {
         if (options == YSP__validate) {
            erh << ECE__ERROR << _YSP__DUPATT << pUcName << endm;
         }
      }
      Attribute * pAttCurr = attlst.defineAttr(pUcName, dclVal);
      if (!pAttCurr) {
         bypassDecl();
         return;
      }
      /*----------------------------+
      | Grab the name group, if any |
      | This updates arglEnum       |
      +----------------------------*/
      if ((dclVal == ADCLV_ENUM) || (dclVal == ADCLV_NOTATION)) {
         AttDclvGrabGroupCB dclvCB(attlst, pAttCurr);
         grabGroup(dclvCB);
         if (dlmfnd.delimFound() != Delimiter::IX_GRPC) {
            erh << ECE__ERROR << _YSP__INVATTDCL << endm;
            bypassDecl();
            return;
         }
      }

      /*-----------------------------------------------------------+
      | [147] default value =                                      |
      |    ((rni, "FIXED", ps+)?, attribute value specification) | |
      |    (rni,("REQUIRED" | "CURRENT" | " CONREF" | "IMPLIED"))  |
      +-----------------------------------------------------------*/
      skip_ps();
      e_AttDftVal dftVal;
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_RNI:
         dftVal = (e_AttDftVal)grabChoice(aChoicesAdftv);
         switch (dftVal) {
         case ADFTV_FIXED:
            skip_ps();
            break;
         case ADFTV_REQUIRED:
            flag.required = true;
            break;
         case ADFTV_CURRENT:
            if (isDataAttlist) {                  // See 11.4.1
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__INVADFTV << endm;
               }
               dftVal = ADFTV_IMPLIED;            // assume IMPLIED
            }else {
               flag.current = true;
            }
            break;
         case ADFTV_CONREF:
            if (isDataAttlist) {                  // See 11.4.1
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__INVADFTV << endm;
               }
               dftVal = ADFTV_IMPLIED;            // assume IMPLIED
            }else {
               flag.conref = true;
            }
            break;
         case ADFTV_IMPLIED:
            break;
         default:                                 // ADFTV_INVALID
            dftVal = ADFTV_IMPLIED;
            break;
         }
         break;
      default:
         dftVal = ADFTV_DEFLT;
         break;
      }
      pAttCurr->setDefaultVal(dftVal);
      /*-----------------------+
      | Parse the default spec |
      | as any regular spec    |
      +-----------------------*/
      if (dftVal >=  ADFTV_DEFLT) {            // default or fixed
         Delimiter::e_Ix dlmLit = Delimiter::IX_NOT_FOUND;
         UnicodeString ucsDefault = grabValueSpec(pAttCurr, dlmLit);
         pAttCurr->setValueSpec(ucsDefault, dlmLit);
      }
   }
   attlst.setFlag(flag);

   /*---------------------------------------+
   | Now attach the complete attribute list |
   | to the element(s) / data-entity(ies)   |
   +---------------------------------------*/
   if (isDataAttlist) {
      int const iLast = associateds.count();
      for (int i=0; i < iLast; ++i) {
         int ix = associateds.inqIx(i);
         //>>>PGR: should be burried inside elmmgr!
         Notation * pNcb = entMgr.inqListNotation()[ix];
         if (!pNcb) {
            assert (false);
            continue;
         }
         if (pNcb->attachAttlist(attlst)) {
            if (ev.isPresent()) {
               evlst += YaspEvent(pNcb, YSP__DCL_NATTRIBUTE);
            }else {
               ev = YaspEvent(pNcb, YSP__DCL_NATTRIBUTE);
            }
         }else {
            if (options == YSP__validate) {
               erh << ECE__ERROR << _YSP__DUPATTL << pNcb->key() << endm;
            }
         }
      }
   }else {
      int const iLast = associateds.count();
      for (int i=0; i < iLast; ++i) {
         int ix = associateds.inqIx(i);
         //>>>PGR: should be burried inside elmmgr!
         Element * pElm = elmMgr.inqListElement()[ix];
         if (!pElm) {
            assert (false);
            continue;
         }
         if (pElm->attachAttlist(attlst)) {
            if (ev.isPresent()) {
               evlst += YaspEvent(pElm, YSP__DCL_ATTRIBUTE);
            }else {
               ev = YaspEvent(pElm, YSP__DCL_ATTRIBUTE);
            }
         }else {
            if (options == YSP__validate) {
               erh << ECE__ERROR << _YSP__DUPATTL << pElm->key() << endm;
            }
         }
      }
   }
}

/*--------------------------------------------------------------parseExternId-+
| Parse the External identifer portion of a markup declaration.               |
| Leaves the stream pointing just after the sysid (if any), plus any          |
| following spaces.                                                           |
|                                                                             |
| [73] external identifier = ("SYSTEM" | ("PUBLIC", ps+, public identifier)), |
|                      (ps+, system identifier)?                              |
|                                                                             |
| [74] public identifier = minimum literal                                    |
|                                                                             |
| [75] system identifier = ((lit, system data, lit)|(lita, system data, lita))|
|                                                                             |
| [45] system data = character data  (no markup recognized)                   |
|                                                                             |
| A System Identifier is optional for PUBLIC                                  |
| A Public identifier doesn't exist for SYSTEM                                |
+----------------------------------------------------------------------------*/
ExternId Yasp3::parseExternId(ReservedName::e_Ix ixKey)
{
   static ReservedName::Choice const aChoicesExternId[] = {
      ReservedName::Choice(ReservedName::IX_PUBLIC),
      ReservedName::Choice(ReservedName::IX_SYSTEM),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND)
   };
   UnicodeString ucsSid, ucsPid;
   Delimiter::e_Ix dlmSid, dlmPid;

   if (ixKey == ReservedName::IX_NOT_FOUND) {
      ixKey = (ReservedName::e_Ix)grabChoice(aChoicesExternId);
   }
   switch (ixKey) {
   case ReservedName::IX_PUBLIC:
      skip_ps();
      dlmPid = dlmfnd.delimFound();
      ucsPid = getMinimumLiteral();
      break;
   case ReservedName::IX_SYSTEM:
      dlmPid =  Delimiter::IX_NOT_FOUND;
      break;
   default:
      return ExternId::Nil;
   }
   skip_ps();
   dlmSid = dlmfnd.delimFound();
   if ((dlmSid == Delimiter::IX_LIT) || (dlmSid == Delimiter::IX_LITA)) {
      ucsSid = getMinimumLiteral();
      skip_ps();
   }else {
      if (ixKey == ReservedName::IX_SYSTEM) return ExternId(ExternId::TEXT);
      dlmSid = Delimiter::IX_NOT_FOUND;
   };
   return ExternId(ucsSid, value(dlmSid), ucsPid, value(dlmPid));
}

/*------------------------------------------------------------parseEntityDecl-+
| Parse an entity declaration, from just after "<!ENTITY", in MD mode.        |
|                                                                             |
|Enter here when ENTITY has been decoded,                                     |
|Returns with curr_char (supposed to ) == MDC                                 |
|                                                                             |
|[101] entity declaration = mdo, "ENTITY",                                    |
|          ps+, entity name, ps+, entity text, ps*, mdc                       |
|                                                                             |
|[102] entity name = general entity name | parameter entity name              |
|[103] general entity name = name | (rni,"DEFAULT")                           |
|[104] parameter entity name = pero, ps+, name                                |
|                                                                             |
|[105] entity text = parameter literal | data text | bracketed text |         |
|                    external entity specification                            |
|                                                                             |
|[106] data text = ("CDATA"|"SDATA"|"PI"), ps+, parameter literal             |
|[107] bracketed text = ("STARTAG"|"ENDTAG"|"MS"|"MD"), ps+, parameter literal|
|                                                                             |
|[108] external entity specification = external identifier,(ps+, entity type)?|
|[109] entity type = "SUBDOC" |                                               |
|                    ( ("NDATA" | "CDATA" | " SDATA"),                        |
|                       ps+, notation name, data attribute specification ?)   |
|[149.2] data attribute specification =                                       |
|                  ps+, dso, attribute specification list, ps*, dsc           |
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::parseEntityDecl(YaspEvent & ev)
{
   static ReservedName::Choice const aChoicesInternalEntityKind[] = {
      ReservedName::Choice(ReservedName::IX_SDATA,     ENTK_SDATA     ),
      ReservedName::Choice(ReservedName::IX_CDATA,     ENTK_CDATA     ),
      ReservedName::Choice(ReservedName::IX_PI,        ENTK_PI        ),
      ReservedName::Choice(ReservedName::IX_STARTTAG,  ENTK_STARTTAG  ),
      ReservedName::Choice(ReservedName::IX_ENDTAG,    ENTK_ENDTAG    ),
      ReservedName::Choice(ReservedName::IX_MS,        ENTK_MS        ),
      ReservedName::Choice(ReservedName::IX_MD,        ENTK_MD        ),
      ReservedName::Choice(ReservedName::IX_PUBLIC,    ENTK_last      ),
      ReservedName::Choice(ReservedName::IX_SYSTEM,    ENTK_last + 1  ),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND, ENTK_INVALID   )
   };
   static ReservedName::Choice const aChoicesExternalEntityKind[] = {
      ReservedName::Choice(ReservedName::IX_SUBDOC,    ENTK_SUBDOC    ),
      ReservedName::Choice(ReservedName::IX_NDATA,     ENTK_NDATA     ),
      ReservedName::Choice(ReservedName::IX_CDATA,     ENTK_CDATA     ),
      ReservedName::Choice(ReservedName::IX_SDATA,     ENTK_SDATA     ),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND, ENTK_INVALID   )
   };

   ev = YSP__ERROR;
   UnicodeString ucsName;
   EntityFlag bFlag;
   {
      bool isOk = false;
      skip_ps();
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_RNI:
         if (!skip(ReservedName::IX_DEFAULT)) {
            erh << ECE__ERROR << _YSP__INVKEYWD1
                << name(ReservedName::IX_DEFAULT)
                << endm;
         }else {
            bFlag.deflt = 1;
            isOk = true;
         }
         break;
      case Delimiter::IX_NOT_FOUND:
         if (skip(Delimiter::IX_PERO)) {  // "%" + blank is *not* a PERO
            bFlag.parameter = 1;
            skip_ps();
            if (dlmfnd.delimFound() != Delimiter::IX_NOT_FOUND) {
               erh << ECE__ERROR << _YSP__INVNAME << endm;
               break;
            }
         }
         {
            UCS_2 entName[1+NAMELEN_MAX];
            if (grabNameEntity(entName)) {
               ucsName =  UnicodeString(entName);
               isOk = true;
            }
         }
         break;
      default:
         erh << ECE__ERROR << _YSP__INVNAME << endm;
         break;
      }
      if (!isOk) {
         bypassDecl();
         return;
      }
   }
   Entity * pEnt = 0;

   /*----------------------------------+
   | Find out what kind of entity text |
   |  Search for:                      |
   |  - a LIT/LITA (parm lit),         |
   |  - a choice in InternalEntityKind |
   |  - ENTK_SGML => external entity   |
   +----------------------------------*/
   skip_ps();        // lit?  lita?
   int iKind;
   if (dlmfnd.delimFound() != Delimiter::IX_NOT_FOUND) { // assume lit or lita
      iKind = ENTK_TEXT;
   }else {
      iKind = grabChoice(aChoicesInternalEntityKind);
   }
   if (iKind >= ENTK_last) {                     // external entity
      /*----------------------------------+
      | External Entity:                  |
      | - get the external identifier,    |
      | - get the data notation if any    |
      +----------------------------------*/
      bFlag.external = 1;
      bFlag.rcdata = 1;                          // RCDATA ON (default)
      ExternId exid(
         parseExternId((iKind == ENTK_last)?
            ReservedName::IX_PUBLIC : ReservedName::IX_SYSTEM
         )
      );
      if (!exid) {
         bypassDecl();
         return;
      }
      e_EntKind kind;
      DataNotation nad;
      if (dlmfnd.delimFound() != Delimiter::IX_NOT_FOUND) { // assume MDC found
         // assign_parser_kind: ENTK_TEXT
         kind = ENTK_SGML;
      }else {
         kind = (e_EntKind)grabChoice(aChoicesExternalEntityKind);
         switch (kind) {
         case ENTK_INVALID:
            kind = ENTK_SGML;
            break;
         case ENTK_SUBDOC:
            if (bFlag.parameter) {
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__INVPENTK << endm;
               }
               kind = ENTK_SGML;
            }else {
               // assign_parser_kind: ENTK_SUBDOC
               bFlag.rcdata = 0;
            }
            skip_ps();
            break;
         default:                                // NDATA, CDATA, SDATA
            skip_ps();
            nad = parseDataNotation();
            if (bFlag.parameter) {               // if parameter entity
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__INVPENTK << endm;
               }
               kind = ENTK_SGML;
               nad = DataNotation::Nil;
            }else {
               // assign_parser_kind: ENTK_NON_SGML
               bFlag.notation = 1;
               bFlag.rcdata = 0;
            }
            break;
         }
      }
      pEnt = new EntityExt(ucsName, bFlag, kind, exid, nad);
   }else {                                       // kind == SGML - external
      /*------------------------------------+
      | Internal Entity:                    |
      | - skip ps after qualifier (if any), |
      | - get the parameter literal         |
      +------------------------------------*/
      e_EntKind kind = (e_EntKind)iKind;
      switch (kind) {
      case ENTK_TEXT:
         bFlag.rcdata = 1;
         break;
      case ENTK_PI:
         skip_ps();
         break;
      case ENTK_INVALID:    // error already reported
         kind = ENTK_TEXT;
         bFlag.rcdata = 1;
         skip_ps();
         break;
      case ENTK_STARTTAG:
      case ENTK_ENDTAG:
      case ENTK_MS:
      case ENTK_MD:
         bFlag.rcdata = 1;
         skip_ps();
         break;
      default:  // ENTK_CDATA ENTK_SDATA
         if (bFlag.parameter) {
            if (options == YSP__validate) {
               erh << ECE__ERROR << _YSP__INVPENTK << endm;
            }
            kind = ENTK_TEXT;
            bFlag.rcdata = 1;
         }
         skip_ps();
         break;
      }
      {
         Delimiter::e_Ix dlm;
         UnicodeString ucsVal = getParameterLiteral(dlm);
         pEnt = new EntityInt(ucsName, bFlag, kind, ucsVal, dlm);
      }
      skip_ps();
   }

   switch (entMgr.defineEntity(pEnt)) {  // deletes pEnt if not added
   case 0:                             // memory shortage?
      assert (false);
      break;
   case -1:                            // duplicate.  it's OK
      ev = YSP__DCL_NULL;
      break;
   default:                            // case 1: OK
      if (bFlag.parameter) {
         ev = YaspEvent(pEnt, YSP__DCL_PENTITY);
      }else {
         ev = YaspEvent(pEnt, YSP__DCL_ENTITY);
      }
      break;
   }
}

/*----------------------------------------------------------parseDataNotation-+
| Parse a data notation: notation name, data attribute specification?         |
| Enter when at the notation name                                             |
+----------------------------------------------------------------------------*/
DataNotation Yasp3::parseDataNotation()
{
   UCS_2 notName[1+NAMELEN_MAX];
   Notation * pNcb;
   if (
      !grabNameGeneral(notName) ||
      (pNcb = entMgr.defineNotationIfNotThere(notName), !pNcb)
   ) {
      bypassDecl();
      return DataNotation::Nil;
   }
   Attlist attlst = pNcb->inqAttlist();
   skip_ps();
   if (dlmfnd.delimFound() == Delimiter::IX_DSO) {
      bool isConrefSpec;
      dlmfnd.pushMode(MODE_ASL);
      parseAttSpecList(attlst, isConrefSpec);
      dlmfnd.popMode();
      if (dlmfnd.delimFound() != Delimiter::IX_DSC) {
         erh << ECE__ERROR << _YSP__NODSC << endm;
         bypassDecl();
      }else {
         skip_ps();
      }
   }
   return DataNotation(pNcb, attlst);   // assume MDC found
}

/*----------------------------------------------------------parseNotationDecl-+
| Parse a notation decl.                                                      |
| Enter here when NOTATION has been decoded,                                  |
| Returns with curr_char (supposed to ) == MDC                                |
|                                                                             |
|  [148] notation declaration = mdo, "NOTATION", ps+, notation name, ps+,     |
|                               notation identifier, ps*, mdc)                |
|                                                                             |
|  [149] notation identifier = external identifier                            |
+----------------------------------------------------------------------------*/
void Yasp3::parseNotationDecl(YaspEvent & ev)
{
   skip_ps();
   if (dlmfnd.delimFound() == Delimiter::IX_NOT_FOUND) {
      UCS_2 notName[1+NAMELEN_MAX];
      if (!grabNameGeneral(notName)) {
         bypassDecl();
         ev = YSP__ERROR;
         return;
      }
      skip_ps();
      ExternId exid(parseExternId());
      if (!exid) {
         bypassDecl();
         ev = YSP__ERROR;
         return;
      }
      Notation * pNcb = entMgr.defineNotation(notName, exid);
      if (pNcb) {
         ev = YaspEvent(pNcb, YSP__DCL_NOTATION);
         return;
      }
   }
   erh << ECE__ERROR << _YSP__INVNOTDCL << endm;
   bypassDecl();
   ev = YSP__ERROR;
}

/*----------------------------------------------------------------parseUsemap-+
| Parse a USEMAP -- in CON ?????                                              |
| >>>PGR: ** this is just a stub **                                           |
|                                                                             |
| Enter here when USEMAP has been decoded,                                    |
| Returns with curr_char (supposed to ) == MDC                                |
|                                                                             |
| [152] short reference use declaration = mdo, "USEMAP",                      |
|                    ps+, map specification,                                  |
|                    (ps+, associated element type)?, <= one at least in DTD  |
|                    ps*, mdc                                                 |
|                                                                             |
| [153] map specification = map name | (rni,"EMPTY")                          |
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::parseUsemap(YaspEvent & ev)
{
   skip_ps();
   if (dlmfnd.delimFound() == Delimiter::IX_RNI) {
      if (skip(ReservedName::IX_EMPTY)) {
         UnicodeString ucsDefault(
            value(Delimiter::IX_RNI) + name(ReservedName::IX_EMPTY)
         );
         ev = YSP__DCL_NULL;
      }else {
         erh << ECE__ERROR << _YSP__INVKEYWD1
             << name(ReservedName::IX_EMPTY)
             << endm;
         ev = YSP__ERROR;
      }
   }else {
      erh << ECE__ERROR << _YSP__USPDECL
          << name(ReservedName::IX_USEMAP)
          << endm;
      UCS_2 mapName[1+NAMELEN_MAX];
      grabNameGeneral(mapName);
      ev = YSP__ERROR;
   }
   bypassDecl();
}

/*---------------------------------------------------------------parseDoctype-+
| Parse the beginning of a DOCTYPE declaration.                               |
| Pass back the document type name as the token, and its public identifier    |
| as the attribute string.                                                    |
|                                                                             |
| Return in (hopefully) DS mode -- or MD mode if the DOCTYPE had no subset.   |
| Each declaration in the subset is parsed one at a time, and the parser      |
| returns when the corresponding MDC has been decoded.                        |
|                                                                             |
| The MDC closing the DOCTYPE MD (following the DSC, has the DOCTYPE had      |
| a subset) is detected in handleDelim.  Then, the Application is notified    |
| to open the external doctype if any (see sec. 11.1.).  Hence, if the MDC is |
| found in this routine (no subset), unread it so handleDelim can do its job. |
|                                                                             |
| [110] document type declaration =                                           |
|       mdo, "DOCTYPE", ps+, document type name,                              |
|       (ps +, external identifier)?,                                         |
|       (ps +, dso, document type declaration subset, dsc)?,                  |
|       ps*, mdc                                                              |
|                                                                             |
| When entering this routine, MODE_MD should have been pushed.                |
+----------------------------------------------------------------------------*/
void Yasp3::parseDoctype(YaspEvent & ev, bool isSuperfluous)
{
   UCS_2 giName[1+NAMELEN_MAX];

   if (isSuperfluous) {              // issue now, so to point at error
      erh << ECE__ERROR << _YSP__SUPFDOCTYPE << endm;
   }
   if (skip_ps(), !grabNameGeneral(giName)) {
      erh << ECE__ERROR << _YSP__INVDOCTYPE << endm;
      ev = YSP__DCL_NULL;
      bypassDoctypeDecl();
      return;
   }
   if (isSuperfluous) {
      Entity const * pEnt = entMgr.inqEntityDoctype();
      if (!pEnt || !(pEnt->inqName())) {
         setDoctype(giName);
      }
      bypassDoctypeDecl();
      ev = YSP__DCL_NULL;
      return;
   }
   elmMgr.setRootModel(giName);           // set model of <#ROOT>
   if (skip_ps(), dlmfnd.delimFound() != Delimiter::IX_NOT_FOUND) {
      // DSO or MDC
      ev = YaspEvent(entMgr.defineEntityDoctype(giName), YSP__DCL_START);
   }else {
      /*
      | If an external identifier has been found, decode it and update
      | the "doctype" special entity.
      */
      ExternId exid(parseExternId());
      if (!exid) {             // bad externid
         erh << ECE__ERROR << _YSP__INVDOCTYPE << endm;
         bypassDoctypeDecl();  // but the DOCTYPE started, anyway
      }
      ev = YaspEvent(
         entMgr.defineEntityDoctype(
            giName,
            exid.inqSystemId(), exid.inqDelimSystemId(),
            exid.inqPublicId(), exid.inqDelimPublicId()
         ),
         YSP__DCL_START
      );
   }
}

/*-----------------------------------------------------------------handleDecl-+
| Effects:                                                                    |
|   Parse any construction beginning with the MDO delimiter ("<!").           |
|   Returns when the closing MDC is found, except for <!DOCTYPE ...           |
|   Expects to be called when current delimiter is MDO.                       |
+----------------------------------------------------------------------------*/
void Yasp3::handleDecl()
{
   static ReservedName::Choice const aChoicesMarkupDecl[] = {

      //>>PGR: This shouldn't be here.  Fix later
      ReservedName::Choice(ReservedName::IX_SGML),
      ReservedName::Choice(ReservedName::IX_DOCTYPE),
      ReservedName::Choice(ReservedName::IX_LINKTYPE),

      ReservedName::Choice(ReservedName::IX_LINK),
      ReservedName::Choice(ReservedName::IX_USELINK),

      //>>PGR: only these choices should be allowed:
      ReservedName::Choice(ReservedName::IX_ENTITY),
      ReservedName::Choice(ReservedName::IX_ELEMENT),
      ReservedName::Choice(ReservedName::IX_ATTLIST),
      ReservedName::Choice(ReservedName::IX_NOTATION),
      ReservedName::Choice(ReservedName::IX_SHORTREF),
      ReservedName::Choice(ReservedName::IX_USEMAP),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND)
   };
   ReservedName::e_Ix ixDecl = (ReservedName::e_Ix)grabChoice(
      aChoicesMarkupDecl
   );

   if (ixDecl == ReservedName::IX_DOCTYPE) {    // this one is special...
      bool const isSuperfluous = (
         (dlmfnd.mode() == MODE_XDS) || (dlmfnd.mode() == MODE_DS)
      );
      dlmfnd.pushMode(MODE_MD);
      if (pastDOCTYPE) {
         erh << ECE__ERROR << _YSP__DUPDOCTYPE << endm;
         bypassDoctypeDecl();
         evCurr = YSP__DCL_NULL;
      }else {
         parseDoctype(evCurr, isSuperfluous);
      }
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_DSO:
         // iStartDtdSubset = oec.inqPos();
         dlmfnd.pushMode(MODE_DS);
         break;                             // keep going in DS mode
      case Delimiter::IX_MDC:               // No extid, nor subset
         // Leave the MDC to be parsed as next event
         pendingDelim = Delimiter::IX_MDC;
         break;                             // keep going in MD mode
      default:
         dlmfnd.popMode();                  // pop from MD mode
         oec.backup(dlmfnd.delimLength());  // leave delimiter unparsed
         break;
      }
      return;
   }

   dlmfnd.pushMode(MODE_MD);
   if (pastDOCTYPE && (options != YSP__allowWildDecl)) {
      erh << ECE__ERROR << _YSP__WILDDECL << endm;
   }
   switch (ixDecl) {
   case ReservedName::IX_SGML:
      parseSgmlDecl(evCurr);                   // NOT YET IMPLEMENTED
      break;
   case ReservedName::IX_ENTITY:
      parseEntityDecl(evCurr);
      break;
   case ReservedName::IX_ELEMENT:
      parseElementDecl(evCurr);
      break;
   case ReservedName::IX_ATTLIST:
      parseAttlistDecl(evCurr);
      break;
   case ReservedName::IX_NOTATION:
      parseNotationDecl(evCurr);
      break;
   case ReservedName::IX_USEMAP:
      parseUsemap(evCurr);
      break;
   case ReservedName::IX_NOT_FOUND:
      bypassDecl();
      evCurr = YSP__ERROR;
      break;
// case ReservedName::IX_LINKTYPE:
// case ReservedName::IX_SHORTREF:
// case ReservedName::IX_LINK:
// case ReservedName::IX_USELINK:
   default:
      erh << ECE__ERROR << _YSP__USPDECL << name(ixDecl) << endm;
      bypassDecl();
      evCurr = YSP__ERROR;
      break;
   }
   dlmfnd.popMode();
   if (dlmfnd.delimFound() != Delimiter::IX_MDC) {
      erh << ECE__ERROR << _YSP__NOMDC << name(ixDecl) << endm;
      bypassDecl();
   }
}

/*---------------------------------------------------------parseMarkedSection-+
| Parse the beginning of a marked section, starting just after "<![".         |
|                                                                             |
| When returning:                                                             |
|   auxBuf:       List of intervening parameter entities                      |
|   yspBuf:       The resulting MS keyword                                    |
|   atrBuf:       List of MS keywords found/solved                            |
|                                                                             |
| Enter here when a Marked Section Start (MDO_DSO_TK) has been decoded        |
| Returns with the right mode pushed, if not INCLUDE or TEMP.                 |
|                                                                             |
| [93] marked section declaration =                                           |
|              marked section start, status keyword specification, dso,       |
|              marked section, marked section end                             |
|                                                                             |
| [94] marked section start = mdo, dso                                        |
|                                                                             |
| [95] marked section end = msc, mdc                                          |
|                                                                             |
| [96] marked section = SGML character*                                       |
|                                                                             |
| [97] status keyword specification =                                         |
|                (ps+, (status keyword | "TEMP"))*, ps*                       |
|                                                                             |
| [100] status keyword = "CDATA" | "IGNORE" | "INCLUDE" | "RCDATA"            |
|                                                                             |
| As stated in 9.1 and note of 4.263, a "marked section end" in an RCDATA     |
| marked section is not recognized as such if it occurs in an entity          |
| imbedded in the marked section.                                             |
| For RCDATA, dlmfind() will not recognize "]]>"  occuring at a higher entity |
| depth than the entity depth the RCDATA marked section section started with. |
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::parseMarkedSection(YaspEvent & ev)
{
   static ReservedName::Choice const aChoicesMsKey[] = {
      ReservedName::Choice(ReservedName::IX_TEMP,      MSKEY_TEMP     ),
      ReservedName::Choice(ReservedName::IX_INCLUDE,   MSKEY_INCLUDE  ),
      ReservedName::Choice(ReservedName::IX_RCDATA,    MSKEY_RCDATA   ),
      ReservedName::Choice(ReservedName::IX_CDATA,     MSKEY_CDATA    ),
      ReservedName::Choice(ReservedName::IX_IGNORE,    MSKEY_IGNORE   ),
      ReservedName::Choice(ReservedName::IX_NOT_FOUND, MSKEY_INVALID  )
   };

   int const iDepthStart = oec.iDepth;
   bool const isForceInclude = (
      (elmMgr.inqDepth() > 1) &&           // only in doc instance
      (options == YSP__includeIgnoredMS)
   );
   e_MsKey resultingKey = MSKEY_INVALID;
   bool isErrorFound = false;           // Turns true after 1st error
   MarkedSectionData * pMsdata = new MarkedSectionData;

   dlmfnd.pushMode(MODE_MD);
   while (!isErrorFound) {
      skip_ps();
      /*-----------------------+
      |(ps+, keywd)*, ps*, DSO |
      +-----------------------*/
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_NOT_FOUND:
         // Remember parameter entities...
         {
            Entity const * pEnt;
            if (
               (oec.iDepth == iDepthStart + 1) &&
               (oec.inqEntity()->isParameter())
            ) {
               pEnt = oec.inqEntity();
            }else {
               pEnt = 0;
            }
            e_MsKey key = (e_MsKey)grabChoice(aChoicesMsKey);
            switch (key) {
            case MSKEY_INVALID:
               isErrorFound = true;
               break;                     // and exit the loop
            case MSKEY_IGNORE:            // force include if param entity
               if (isForceInclude && pEnt) {
                  key = MSKEY_INCLUDE;
               }
               /* fall thru */
            default:
               if (key > resultingKey) {
                  resultingKey = key;
               }
               pMsdata->add(pEnt, key);
               continue;
            }
         }
         break;       // Troubles...
      case Delimiter::IX_DSO:
         break;       // Success
      default:        // PLUS_GRPO, MINUS_GRPO, RNI, LIT, LITA, MDC, DSC, GRPO
         isErrorFound = true;
         break;
      }
      break;          // break the for loop, if no "continue" bypassed it
   }
   dlmfnd.popMode();
   if (isErrorFound) {
      erh << ECE__ERROR << _YSP__INVMSSTART << endm;
      delete pMsdata;
      ev = YSP__ERROR;
      return;
   }
   /*
   | Adjust the recognition mode
   */
   switch (resultingKey) {
   case MSKEY_IGNORE:
      dlmfnd.pushMsMode(MODE_MS_IGNORE);
      break;
   case MSKEY_CDATA:
      dlmfnd.pushMsMode(MODE_MS_CDATA);
      break;
   case MSKEY_RCDATA:   // MSC_MDC won't be recognized:
      dlmfnd.pushMsMode(MODE_MS_RCDATA, oec.iDepth);
      break;
   case MSKEY_INVALID:  // this means: no key specfd - which is legal
      resultingKey = MSKEY_INCLUDE;
      /* fall thru */
   default:             // INCLUDE, TEMP
      dlmfnd.pushMsMode(MODE_MS_INCLUDE);
      break;
   }
   pMsdata->setResult(resultingKey);
   ev = YaspEvent(pMsdata, YSP__MARKED_START);
}
/*===========================================================================*/
