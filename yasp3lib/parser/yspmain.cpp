/*
* $Id: yspmain.cpp,v 1.15 2002-04-24 03:08:13 jlatone Exp $
*
* Primary external routines for the SGML parser.
*/

/*---------+
| Includes |
+---------*/
#include "../yasp3.h"
#include "../../toolslib/uccompos.h"
#include "../entity/entsolve.h"
#include "../element/elmmgr.h"
#include "CachedDtd.h"

UnicodeString const Yasp3::docEntName("#DOCUMENT");
UnicodeString const Yasp3::dtdEntName("#DTD");
UnicodeString const Yasp3::xmlPi("xml");
UnicodeString const Yasp3::xmlVersion("version");
UnicodeString const Yasp3::xmlEncoding("encoding");
UnicodeString const Yasp3::xmlRequiredMarkup("standalone");

/*----------------------------------------------------------------------Yasp3-+
| Effects:                                                                    |
|   Sets up a parser instance.  Should follow by openDocument.                |
+----------------------------------------------------------------------------*/
Yasp3::Yasp3(UnicodeComposer & erhArg, SgmlDecl::SyntaxFlavour flv) :
   erh(erhArg),
   dlmfnd(sdcl, erhArg),
   sdcl(flv),
   entMgr(erhArg, &oec),
   elmMgr(erhArg),
   evlst(erhArg),
   pSolver(0),
   errorCount(0),
   bParserOk(false),              // be pessimistic
   bMoreToParse(false),
   iTextSize(0),
   pUcText(0),
   isTextOwned(false)
{
   if (!setTextBuffer()) {
      erh << ECE__FATAL << _YSP__BAD_BUFSIZE << endm;
      return;
   }
   if (sdcl.isHTML()) setOption(YSP__HTML, true);
   if (sdcl.isXML()) entMgr.setEncodingCurrent(EncodingModule::UTF_8);
   {
       //>>>PGR: temporary until we really handle the SGML decl
       Quantity & q = (Quantity &)sdcl.quantity(); // violates constness
       q.set(Quantity::IX_ATTCNT,   ATTCNT_MAX);
       q.set(Quantity::IX_ATTSPLEN, ATTSPLEN_MAX);
       q.set(Quantity::IX_BSEQLEN,  BSEQLEN_MAX);
       q.set(Quantity::IX_DTAGLEN,  DTAGLEN_MAX);
       q.set(Quantity::IX_DTEMPLEN, DTEMPLEN_MAX);
       q.set(Quantity::IX_ENTLVL,   ENTLVL_MAX);
       q.set(Quantity::IX_GRPCNT,   GRPCNT_MAX);
       q.set(Quantity::IX_GRPGTCNT, GRPGTCNT_MAX);
       q.set(Quantity::IX_GRPLVL,   GRPLVL_MAX);
       q.set(Quantity::IX_LITLEN,   LITLEN_MAX);
       q.set(Quantity::IX_NAMELEN,  NAMELEN_MAX);
       q.set(Quantity::IX_NORMSEP,  NORMSEP_MAX);
       q.set(Quantity::IX_PILEN,    PILEN_MAX);
       q.set(Quantity::IX_TAGLEN,   TAGLEN_MAX);
       q.set(Quantity::IX_TAGLVL,   TAGLVL_MAX);
   }

   // If we get here, we've allocated all the needed dynamic space.
   // Now set up initial values for everything:
   options << YSP__F_SHORTTAG << YSP__F_OMITTAG;

   // expand all types, except NDATA
   for (int i=0; i < (sizeof isExpanded / sizeof isExpanded[0]); ++i) {
      isExpanded[i] = true;
   }
   isExpanded[(int)ENTK_NDATA] = false;

   // Init current document info
   DTDrequested = false;
   pastDOCTYPE = false;
   docIsOpen = false;
   pendingDelim = Delimiter::IX_NOT_FOUND;
   iLineDelimitedTextStart = 0;

   if (sdcl.isXML() && sdcl.recognizeEntRef()) initXmlEntities();

   bParserOk = true;
}

/*--------------------------------------------------------------setTextBuffer-+
| Effects:                                                                    |
|   setTextBuffer(myBuffer, itsLength);                                       |
|       sets up an external text buffer of length `itsLength'                 |
|       it will stay until setTextBuffer(0, ???) is called                    |
|   setTextBuffer(0, textLength);                                             |
|       sets up an internal text buffer of length: textLength                 |
|   setTextBuffer(0, 0);                                                      |
|       reset the internal buffer to its default length                       |
|   setTextBuffer(myBuffer, 0);                                               |
|       sets up an external text buffer of default length                     |
+----------------------------------------------------------------------------*/
bool Yasp3::setTextBuffer(UCS_2 * pUcTextArg, int iTextSizeArg)
{
   int iTextSizeTemp = iTextSizeArg;
   UCS_2 * pUcTextTemp = pUcTextArg;
   if (!iTextSizeTemp) {              // if 0,  use the default size
      iTextSizeTemp = DEFAULT_BUFFER_SIZE;
   }
   if (!pUcTextArg) {                // caller wants to use internal buffer
      if (iTextSizeTemp == iTextSize) {
         return true;
      }
      pUcTextTemp = new UCS_2[iTextSizeTemp];
      if (!pUcTextTemp) return false;
   }
   if (isTextOwned) {
      delete [] pUcText;
      isTextOwned = false;
   }
   if (pUcTextArg) {                 // caller provides its own buffer
      isTextOwned = false;
   }else {
      isTextOwned = true;
   }
   pUcText = pUcTextTemp;
   iTextSize = iTextSizeTemp;
   evText.setBuffer(pUcText);
   return true;
}

/*---------------------------------------------------------------------~Yasp3-+
| Effects:                                                                    |
|    Closes all open entities and the parser itself.                          |
+----------------------------------------------------------------------------*/
Yasp3::~Yasp3()
{
   if (docIsOpen) closeDocument();
   if (isTextOwned) delete [] pUcText;
   isTextOwned = false;
   pUcText = 0;
}

/*--------------------------------------------------------------closeDocument-+
| Effects:                                                                    |
|    Pop out of all open entitites and reset the entire parse state to init   |
|    to handle a new file if needed.                                          |
|    However, does not reset to the core concrete syntax if changed.          |
|    Does not reset #CURRENT attributes to unset value.                       |
+----------------------------------------------------------------------------*/
bool Yasp3::closeDocument()
{
   entMgr.closeAll();
   dlmfnd.reset();             // Start in MODE_INI
   rcdbnd.reset();
   elmMgr.reset();

   evlst.reset();
   docIsOpen = false;
   bMoreToParse = false;
   pendingDelim = Delimiter::IX_NOT_FOUND;
   // iStartDtdSubset = 0;
   iLineDelimitedTextStart = 0;
   return true;
}

/*------------------------------------------------------------tryProcessXmlPi-+
| Process any spefic XML processing instruction.                              |
| Enter this routine after a PIO followed by xmlPi has been recognized.       |
|                                                                             |
| XML documents may, and should, begin with an XML declaration                |
|                                                                             |
|  [24]  XMLDecl      ::=  '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'  |
|                                                                             |
| External parsed entities may each begin with a text declaration.            |
|  [78]  TextDecl ::=  '<?xml' VersionInfo? EncodingDecl S? '?>'              |
|                                                                             |
| With:                                                                       |
|  [25]  VersionInfo  ::=  S 'version' Eq ('"VersionNum"' | "'VersionNum'")   |
|  [26]  Eq           ::=  S? '=' S?                                          |
|  [27]  VersionNum   ::=  ([a-zA-Z0-9_.:] | '-')+                            |
|  [32]  SDDecl       ::=  S 'standalone' Eq "'" ('yes' | 'no') "'"           |
|                        | S 'standalone' Eq '"' ('yes' | 'no') '"'           |
|  [81]  EncodingDecl ::=  S 'encoding' Eq '"' EncName '"' | "'" EncName "'"  |
|  [82]  EncName      ::=  [A-Za-z] ([A-Za-z0-9._] | '-')*                    |
|                                                                             |
+----------------------------------------------------------------------------*/
bool Yasp3::tryProcessXmlPi()
{
   if (!skip(xmlPi, false)) {
      return false;
   }

   // if an XML pi was found and if the current mode is not XML
   // turn it on on the fly.
   if (!sdcl.isXML()) {
      if (sdcl.recognizeEntRef()) {
         sdcl.resetSyntax(SgmlDecl::XML);
         initXmlEntities();
      }else {
         sdcl.resetSyntax(SgmlDecl::XML_WITH_NO_ENTREF);
      }
      dlmfnd.resetDelimiters(sdcl, erh);
      //>>>PGR: entMgr.setEncodingCurrent(Encoder::UTF_8); ???
   }

   UCS_2 ucVerDlm;
   UnicodeString ucsVer;
   bool isValidXmlPi = readXmlPiAttribute(xmlVersion, ucsVer, ucVerDlm);
   if (isValidXmlPi && ucsVer.empty() && (entMgr.inqDepth() == 1)) {
      erh << ECE__ERROR << _YSP__NOXMLVER << endm;
   }

   UCS_2 ucEncDlm;
   UnicodeString ucsEnc;
   if (isValidXmlPi) {
      isValidXmlPi = readXmlPiAttribute(xmlEncoding, ucsEnc, ucEncDlm);
   }

   UCS_2 ucRmdDlm;
   UnicodeString ucsRmd;
   if (isValidXmlPi && (entMgr.inqDepth() == 1)) {
      isValidXmlPi = readXmlPiAttribute(xmlRequiredMarkup, ucsRmd, ucRmdDlm);
   }

   // eat the PIC while it's time: encoding will change!
   if (!endXmlPi()) {
      isValidXmlPi = false;
   }

   // Time to set the encoding
   if (ucsEnc.good()) {
      entMgr.setEncodingCurrent(Encoder::getEncoding(ucsEnc));
   }
   evCurr = YaspEvent(
      new XmlPiData(
         ucsVer, ucsEnc, ucsRmd,
         ucVerDlm, ucEncDlm, ucRmdDlm
      ),
      YSP__XML_PI
   );

   // recovery (avoid data loss.)  If the syntax is invalid, the remaining
   // data is processed as regular PI's.
   if (!isValidXmlPi) {
      erh << ECE__ERROR << _YSP__BADXMLPI << endm;
      dlmfnd.pushMode(MODE_PI);
      evlst += YaspEvent(YSP__PI_START);
   }
   return true;
}

/*------------------------------------------------------------------setOption-+
| Effects:                                                                    |
|   Changes parse settings for a token stream.                                |
|   The names callers must pass are declared as an enum in ysp.h.             |
|   bool values should be sent as true or false.                              |
+----------------------------------------------------------------------------*/
bool Yasp3::setOption(
   YaspOptName nameCode,
   bool isToSet
) {
   bool isOk = true;
   bool isDone = false;

   switch (nameCode) {

      // In no case these options should belong to an all-purpose int.
      // It has to comply the rules of the "features"/"charset" objects.
      // All these casts violate constness.

   case YSP__F_DATATAG:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_DATATAG, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_OMITTAG:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_OMITTAG, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_RANK:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_RANK, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_SHORTTAG:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_SHORTTAG, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_FORMAL:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_FORMAL, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_CONCUR:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_CONCUR, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_SIMPLE:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_SIMPLE, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_IMPLICIT:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_IMPLICIT, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_EXPLICIT:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_EXPLICIT, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__F_SUBDOC:
      if (Features::OK != ((Features &)(sdcl.features())).set(
         ReservedName::IX_SUBDOC, isToSet
      )) {
         isOk = false;
      }
      break;
   case YSP__nameCase_G:            // name case general
      ((Charset &)(sdcl.charset())).setCaseGeneral(isToSet);
      break;
   case YSP__nameCase_E:            // name case for entities
      ((Charset &)(sdcl.charset())).setCaseEntity(isToSet);
      break;
   case YSP__HTML:
      if (isToSet) {
         if (!sdcl.isHTML()) {
            sdcl.resetSyntax(SgmlDecl::HTML_WITH_NO_ENTREF);
         }
         options >> YSP__validate;
         evlst.setValidation(true);
      }
      break;
   case YSP__validate:
      if (isToSet) {
         evlst.setValidation(true);
      }else {
         if (options != YSP__HTML) evlst.setValidation(false);
      }
      break;
   case YSP__keepLineBreak:
      if (isToSet) {
         rcdbnd.disable();
      }else {
         rcdbnd.enable();
      }
      break;
   default:                         // regular YSP stuff.
      break;
   }
   if (isOk && !isDone) {
      if (isToSet) {
         options << nameCode;
      }else {
         options >> nameCode;
      }
   }
   if (!isOk) {
      erh << ECE__ERROR << _YSP__INVOPTION << endm;
   }
   return isOk;
}

/*------------------------------------------------------------------inqOption-+
| Effects: Returns tag delimiters or parse settings for a token stream.       |
|          The names callers must pass are declared as an enum in ysp.h.      |
+----------------------------------------------------------------------------*/
int Yasp3::inqOption(YaspOptName nameCode) const
{
   return options == nameCode;
}

/*------------------------------------------------------------initXmlEntities-+
| Define the reserved XML entities                                            |
+----------------------------------------------------------------------------*/
void Yasp3::initXmlEntities()
{
   EntityFlag bFlag;
   bFlag.rcdata = 1;
   entMgr.defineEntity(new EntityInt("amp",   bFlag, ENTK_TEXT, "&"));
   entMgr.defineEntity(new EntityInt("lt",    bFlag, ENTK_TEXT, "<"));
   entMgr.defineEntity(new EntityInt("gt",    bFlag, ENTK_TEXT, ">"));
   entMgr.defineEntity(new EntityInt("quot",  bFlag, ENTK_TEXT, "\""));
   entMgr.defineEntity(new EntityInt("apos",  bFlag, ENTK_TEXT, "\'"));
}

/*-----------------------------------------------------------------addElement-+
| This is a convenient short cut to ElmManager::defineElement in order to     |
| add simple EMPTY, ANY, or CDATA element.                                    |
+----------------------------------------------------------------------------*/
bool Yasp3::addElement(
   UCS_2 const * pUcName, ModelTree::e_DeclaredClass dcl
) {
   UnicodeString ucsGi(pUcName);
   if (!ucsGi) {
      return false;
   }
   if (sdcl.charset().isCaseGeneral()) {
      UCS_2 const * pUcGi = ucsGi;  // equal required by MSVC
      sdcl.charset().uppercase((UCS_2 *)pUcGi);
   }
   ModelTree model;
   model << dcl << endm;
   if (elmMgr.defineElement(ucsGi, model, false, true)) {
      return true;
   }else {
      return false;
   }
}

/*----------------------------------------------------------Yasp3::addElement-+
| Add an element given a model and an attlist                                 |
+----------------------------------------------------------------------------*/
bool Yasp3::addElement(
   UnicodeString const & ucsGi,
   ModelTree const & model,
   Attlist const & attlst
) {
   Element * pElm = elmMgr.defineElement(ucsGi, model, false, true);
   if (pElm) {
      pElm->attachAttlist(attlst);
      return true;
   }else {
      return false;
   }
}
/*------------------------------------------------Yasp3::inqElementIxOrCreate-+
| Set or inquire the ix value (handle) for an element                         |
+----------------------------------------------------------------------------*/
int Yasp3::inqElementIxOrCreate(
   UnicodeString const & ucsGi
) {
   Element const * pElm = elmMgr.defineElementIfNotThere(ucsGi);
   if (pElm) return pElm->inqIx(); else return -1;
}

/*----------------------------------------------------------Yasp3::setDoctype-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Yasp3::setDoctype(UnicodeString const & ucsDoctypeName)
{
   elmMgr.setRootModel(ucsDoctypeName);         // set model for <#ROOT>
   entMgr.defineEntityDoctype(ucsDoctypeName);  // reset doctype entity name
   return true;
}

/*---------------------------------------------------------Yasp3::openDoctype-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::openDoctype()
{
   Entity const * pEnt = entMgr.inqEntityDoctype();
   if (pEnt && entMgr.openExternal(*pEnt, *pSolver)) {
      dlmfnd.pushMode(MODE_XDS);    // now in INI-XDS
      entMgr.freezeFloorDepth();
   }
   DTDrequested = true;
}

/*-------------------------------------------------------Yasp3::loadCachedDtd-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::loadCachedDtd(CachedDtd const & dtd) {
   dtd.load(elmMgr, entMgr);
}

/*--------------------------------------------------Yasp3::getLastParsedChunk-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString Yasp3::getLastParsedChunk() {
   return oec.getInputBackAt(offset);
}

/*-----------------------------------------------------------------------next-+
| Get the next relevant event from parsing the input stream.                  |
+----------------------------------------------------------------------------*/
YaspEvent const & Yasp3::next()
{
   if (!bMoreToParse) {
      evCurr = YSP__ERROR_FATAL;
      return evCurr;
   }
   offset = (int)oec.inqPos() - value(pendingDelim).length();
   *pUcText = 0;

   /* If the current entity is not reparseable, just report its contents
   |  until we reach the end.
   */
   if (!entMgr.inqEntity().isRcdata()) {
      Entity const & ent = entMgr.inqEntity();  // equal required by MSVC
      int iTextLength = entMgr.gets(pUcText, iTextSize);
      if (iTextLength) {
         entMgr.skip(iTextLength);
         switch (ent.inqKind()) {
         case ENTK_PI:
            evText.setTypeAndLength(YSP__PI_TEXT, iTextLength);
            break;
         case ENTK_SDATA:
            evText.setTypeAndLength(YSP__SDATA_TEXT, iTextLength);
            break;
         default:
         // case ENTK_CDATA:
            evText.setTypeAndLength(YSP__CDATA_TEXT, iTextLength);
            break;
         }
         return evText;
      }else {
         switch (ent.inqKind()) {
         case ENTK_PI:
            evCurr = YaspEvent(&ent, YSP__ENT_PI_END);
            break;
         case ENTK_SDATA:
            evCurr = YaspEvent(&ent, YSP__SDATA_END);
            break;
         default:
         // case ENTK_CDATA:
            evCurr = YaspEvent(&ent, YSP__CDATA_END);
            break;
         }
         entMgr.close();
         return evCurr;
      }
   }

   for (;;) {
      /* A pendingEvent is generated:
      |  1) by grabText
      |  2) by handleDelim: handleStartTag / handleEndTag
      |     on needing an omitted tag
      */
      if (evlst.isEventWaiting()) {            // Found new event
         evlst.next(evCurr);
         return evCurr;
      }

      /*
      | Anything left over from last call to grabText / grabDelimitedText?
      */
      if (pendingDelim != Delimiter::IX_NOT_FOUND) {
         Delimiter::e_Ix delimFound = pendingDelim;
         pendingDelim = Delimiter::IX_NOT_FOUND;
         if (handleDelim(delimFound)) {
            return evCurr;
         }
      }

      /*
      | Check current char for special cases.
      | In XDS_mode, the external subset may have ended at any point.
      | Let's close it and notify DCL_END.
      | Otherwhise, this is the end of the primary entity.
      */
      if ((oec.peek() == EOF_SIGNAL)&&(oec.iDepth <= entMgr.inqFloorDepth())) {
         // BACKUP NEEDED
         if (dlmfnd.mode() == MODE_XDS) {
            assert  (!pastDOCTYPE);
            pastDOCTYPE  = true;
            dlmfnd.popMode();              // pop from INI-XDS
            entMgr.thawFloorDepth();
            evCurr = YSP__DCL_END;
         }else {
            if ((options != YSP__noEndElementAtEof) && endAllElements()) {
               evlst.next(evCurr);
            }else {
               evCurr = YSP__PARSE_END;
               bMoreToParse = false;
            }
         }
         return evCurr;
      }

      switch (dlmfnd.mode()) {

      case MODE_INI:   // MDO, STAGO, MDO_COM, MDO_MDC, PIO, CRO
         skip_S();
         pendingDelim = dlmfnd.delimFound();
         switch (pendingDelim) {
         case Delimiter::IX_NOT_FOUND:    // some data..
         case Delimiter::IX_STAGO:        // or a STAGO
         case Delimiter::IX_STAGO_TAGC:   // or an empty STAGO
            /*
            | Added 09/16/92 sjd for where we found something other than
            | spaces or MDO before seeing a DOCTYPE declaration.  Hopefully in
            | such cases the caller will go find the DTD and make us open it.
            */
            if (!DTDrequested) {
               if (options == YSP__validate) {
                  erh << ECE__ERROR << _YSP__NODTD << endm;
               }
               oec.backup(dlmfnd.delimLength());  // later
               pendingDelim = Delimiter::IX_NOT_FOUND;
               evCurr = YaspEvent(entMgr.inqEntityDoctype(), YSP__DCL_START);
               openDoctype();
               return evCurr;
            }
            /* fall thru */
         default:     // MDO, MDO_COM, MDO_MDC, PIO, (CRO???):
            /*
            | If the DTD was requested, and we are back from XDS to INI,
            | this stuff really belongs to CON mode.
            | If the DTD request was not serviced, then pastDOCTYPE is
            | still false: notify a DCL__END first, keep the delim pending.
            */
            if (DTDrequested) {
               //>>>PGR: with shortrefs, should be MODE_CON (time comsuming!)
               dlmfnd.setConMode(MODE_CON_XML, 0, false);
               if (!pastDOCTYPE) {
                  pastDOCTYPE = true;
                  oec.backup(dlmfnd.delimLength());  // later
                  pendingDelim = Delimiter::IX_NOT_FOUND;
                  evCurr = YSP__DCL_END;
                  return evCurr;
               }
            }
            break;
         }
         break;

      case MODE_DS:
      case MODE_XDS:
         if (!skip_ds()) {                // some data..
            evCurr = YSP__DCL_NULL;
            int const maxToPeek = 8;
            UnicodeString const ucs(entMgr.peekNextChars(maxToPeek));
            UnicodeString const & mdo = sdcl.delimList()[Delimiter::IX_MDO];
            if (mdo.compare(ucs, mdo.length())) {
               erh << ECE__ERROR << _YSP__BADDATAINDS << ucs << endm;
            }else {                       // unrecognized MDO ("<!  >")
               if (options != YSP__HTML) {
                  erh << ECE__ERROR << _YSP__BADDECL << endm;
               }
            }
            bypassDecl();
            return evCurr;
         }
         pendingDelim = dlmfnd.delimFound();
         break;                         // let handleDelim do this

      case MODE_MD:
         assert  (false);             // should not happen
         skip_ps();
         if (dlmfnd.delimFound() == Delimiter::IX_NOT_FOUND) { // some data..
            int const maxToPeek = 8;
            erh << ECE__ERROR << _YSP__BADDATAINMD
                << entMgr.peekNextChars(maxToPeek) << endm;
            bypassDecl();
            pendingDelim = dlmfnd.delimFound();
            evCurr = YSP__ERROR;
            return evCurr;
         }
         pendingDelim = dlmfnd.delimFound();
         break;

      case MODE_MD_COM:
         skip_S();
         switch (dlmfnd.delimFound()) {
         case Delimiter::IX_COM:
            dlmfnd.pushMode(MODE_COMMENT); // comment (again)
            break;
         case Delimiter::IX_MDC:
            dlmfnd.popMode();
            evCurr = YSP__COMMENT_END;
            return evCurr;
         default:
            erh << ECE__ERROR << _YSP__BADCOMMENT << endm;
            bypassDecl();
            dlmfnd.popMode();
            evCurr = YSP__COMMENT_END;
            return evCurr;
         }
         /* fall thru */

      case MODE_COMMENT:
         {
            int iTextLength = grabDelimitedText(
               pUcText, iTextSize, Delimiter::IX_COM
            );
            if (iTextLength) {
               evText.setTypeAndLength(YSP__COMMENT_TEXT, iTextLength);
               return evText;
            }
         }
         break;

      case MODE_MD_HCOM:
         {
            int iTextLength = grabDelimitedText(
               pUcText, iTextSize, Delimiter::IX_COM_MDC
            );
            if (iTextLength) {
               evText.setTypeAndLength(YSP__COMMENT_TEXT, iTextLength);
               return evText;
            }
         }
         break;

      case MODE_PI:
         {
            int iTextLength = grabDelimitedText(
               pUcText, iTextSize, Delimiter::IX_PIC
            );
            if (iTextLength) {
               evText.setTypeAndLength(YSP__PI_TEXT, iTextLength);
               return evText;
            }
         }
         break;

      case MODE_MS_IGNORE:                // pendingDelim=="]]>" or EOF
         skipMarkedSection();
         break;

      default:
         /*
         | Mode is one of: MODE_CON, MODE_CON_NOSR,
         |                 MODE_CON_XML, MODE_CON_XML_WNET,
         |                 MODE_CON_CDATA, MODE_CON_RCDATA,
         |                 MODE_MS_CDATA, MODE_MS_RCDATA
         |
         | call grabText() to read up to the next delimiter or the end
         | of a reportable entity, or the end of the primary entity.
         | - it returns the length of the text.  If 0, text was not grabbed,
         |   because a delimiter was immediately found, or because text
         |   implies first a document structure change.   For the latter,
         |   the stack of pendingEvent may have been feeded;
         | - pendingDelim is set to the next delimiter -- if any,
         |   for example STAGO, or ERO on a reportable entity.
         */
         {
            int iTextLength = grabText(pUcText, iTextSize);
            if (iTextLength) {
               evText.setTypeAndLength(YSP__TEXT, iTextLength);
               return evText;
            }
         }
         break;
      }
   }
}

/*----------------------------------------------------------------handleDelim-+
| Effects:                                                                    |
|   Called when we've hit a delimiter that wasn't handled by grabText         |
+----------------------------------------------------------------------------*/
bool Yasp3::handleDelim(
   Delimiter::e_Ix delimFound           // what is on stream?
) {
   switch (delimFound) {

   case Delimiter::IX_MDO_MDC:          // empty comment ("<!>")
      evCurr = YSP__DCL_NULL;
      return true;

   case Delimiter::IX_MDO_COM:          // comment starts (<!--)
      if (options == YSP__HTML) {
         dlmfnd.pushMode(MODE_MD_HCOM); // COM-MDC
      }else {
         dlmfnd.pushMode(MODE_MD_COM);  // COM and MDC
         dlmfnd.pushMode(MODE_COMMENT); // COM only
      }
      evCurr = YSP__COMMENT_START;
      return true;

   case Delimiter::IX_COM:
      dlmfnd.popMode();                 // back to MD mode
      return false;

   case Delimiter::IX_COM_MDC:          // HTML mode only
      dlmfnd.popMode();
      evCurr = YSP__COMMENT_END;
      return true;

   case Delimiter::IX_MDO:              // Markup Declaration Open
      handleDecl();
      return true;

   case Delimiter::IX_DSC:              // Dcl Subset Close  MODE_DS only!
      dlmfnd.popMode();                 // back from DS to MD
      skip_ps();
      if (dlmfnd.delimFound() != Delimiter::IX_MDC) {
         erh << ECE__ERROR << _YSP__INVDOCTYPE << endm;
      }
      /* fall thru */

   case Delimiter::IX_MDC:              // Markup Declaration Close
      /*
      | This can be only a MDC closing a <!DOCTYPE .. declaration
      | DTD_REQUEST must now be issued.
      */
      if (DTDrequested) {               // because <!DOCTYPE [ ]> was passed
         dlmfnd.popMode();              // back from MD to XDS
         return false;
      }else {
         dlmfnd.popMode();              // back from MD to INI
         evCurr = YaspEvent(entMgr.inqEntityDoctype(), YSP__DCL_START);
         openDoctype();
         return true;
      }

   case Delimiter::IX_MDO_DSO:          // Marked section open
      parseMarkedSection(evCurr);
      return true;

   case Delimiter::IX_MSC_MDC:          // Marked section close
      if (!dlmfnd.popMsMode()) {
         break;
      }
      evCurr = YSP__MARKED_END;
      return true;

   case Delimiter::IX_PIO:              // Processing Instruction
      if (!tryProcessXmlPi()) {
         dlmfnd.pushMode(MODE_PI);
         evCurr = YSP__PI_START;
      }
      return true;

   case Delimiter::IX_PIC:              // Processing Instruction Close
      evCurr = YSP__PI_END;
      dlmfnd.popMode();
      return true;

   case Delimiter::IX_STAGO:            // STAGO      "<"
   case Delimiter::IX_STAGO_TAGC:       // STAGO_TAGC "<>"
      handleStartTag();                 // also, skip any trailing <?@wx>
      return true;

   case Delimiter::IX_ETAGO:            // regular end tag
   case Delimiter::IX_ETAGO_TAGC:       // empty end tag
   case Delimiter::IX_NET:              // null end tag
      handleEndTag();
      return true;

   case Delimiter::IX_CRO:              // character entity
      evCurr = YSP__ERROR;
      return true;

   case Delimiter::IX_PERO:             // parameter entity
      return handleEntity(false);

   case Delimiter::IX_ERO:              // general entity
      return handleEntity(true);

   default:
      break;
   }
   /*
   | If we get some other delimiter, it was recognized in context but
   | we don't know what to do, so it's a programming error.  In that
   | case, report it and then treat it as literal text.  This causes a
   | token break because it's convenient that way.
   */
   value(delimFound).ncopy(pUcText, iTextSize);
   if (delimFound == Delimiter::IX_MSC_MDC) {
      erh << ECE__ERROR << _YSP__INVMSEND << endm;
   }else {
      erh << ECE__ERROR << _YSP__INVDELIM << pUcText << endm;
   }
   evText.setTypeAndLength(YSP__TEXT, value(delimFound).length());
   evCurr = evText;
   return true;
}

/*--------------------------------------------------------Yasp3::extractXmlPi-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString Yasp3::extractXmlPi(XmlPiData const & xmlpi) const
{
   UnicodeMemWriter umost;
   umost << value(Delimiter::IX_PIO) << xmlPi;
   if (xmlpi.inqVersion().good()) {
      umost
         << (UCS_2)' ' << xmlVersion << (UCS_2)'='
         << xmlpi.inqVersionDelim()
         << xmlpi.inqVersion()
         << xmlpi.inqVersionDelim();
   }
   if (xmlpi.inqEncoding().good()) {
      umost
         << (UCS_2)' ' << xmlEncoding << (UCS_2)'='
         << xmlpi.inqEncodingDelim()
         << xmlpi.inqEncoding()
         << xmlpi.inqEncodingDelim();
   }
   if (xmlpi.inqRequiredMarkup().good()) {
      umost
         << (UCS_2)' ' << xmlRequiredMarkup << (UCS_2)'='
         << xmlpi.inqRmdDelim()
         << xmlpi.inqRequiredMarkup()
         << xmlpi.inqRmdDelim();
   }
   umost << value(Delimiter::IX_PIC);
   return umost;
}

/*---------------------------------------------------------Yasp3::dummyMethod-+
| This is just to force Visual C++ to export methods which are not called     |
| in this module.  This should never be call, but, in case it is, it does     |
| nothing.                                                                    |
+----------------------------------------------------------------------------*/
void Yasp3::dummyMethod() {
   if (this == 0) {
      errorContext();
   }
}

/*===========================================================================*/
