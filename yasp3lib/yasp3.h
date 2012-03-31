/*
* $Id: yasp3.h,v 1.12 2002-04-14 23:26:04 jlatone Exp $
*/

#if !defined YASP3_HEADER && defined __cplusplus
#define YASP3_HEADER

/*---------+
| Includes |
+---------*/
#include "yasp3gendef.h"
#include "parser/yspenum.h"
#include "parser/rcdbind.h"     // Record binder (7.6.1)
#include "parser/yspevent.h"    // Yasp3 events
#include "syntax/sdcl.h"        // SGML declaration
#include "syntax/dlmfind.h"     // Delimiter finder
#include "entity/entmgr.h"      // Entity manager
#include "element/elmmgr.h"     // Element manager

class CachedDtd;
class UnicodeComposer;
class Tag;

/*----------------------------------------------------------------------Yasp3-+
| Yasp3: SGML parser object.                                                  |
|                                                                             |
| How to use?                                                                 |
| -----------                                                                 |
| Yasp3 implements a class based on the Visitor pattern.                      |
|                                                                             |
| 1) Derive a class based on YaspEventInspector                               |
|                                       (cut and paste from yspevent.h)       |
|    #include <iostream.h>                                                    |
|    #include "yasp3.h'                                                       |
|                                                                             |
|    class MySgmlInspection : public YaspEventInspector {                     |
|    public:                                                                  |
|       void inspectTagStart(Tag const & tag) {                               |
|          cout << '<' << tag.inqName() << '>' << endl;                       |
|       }                                                                     |
|       void inspectText(TextData const & buffer) {                           |
|          cout << buffer.inqText() << endl;                                  |
|       }                                                                     |
|    };                                                                       |
|                                                                             |
|  2) Derive a class from EntitySolverBase                                    |
|    The main method is: getExternalProducer, which associates an input       |
|    stream to each external entities found during the parse and              |
|    that have to be "opened."   This external entity is described            |
|    throughout an instance of an Entity object passed by argument            |
|    to this method.  You should analyse the public id, system id             |
|    and build a strategy to map all these infos to an appropriate            |
|    stream.                                                                  |
|                                                                             |
|    Generally: "ent.inqExternId().inqSystemId()" would suffice,              |
|    except that is guaranteed that there is no system id for the             |
|    main input document!  Where could it be gotten from?                     |
|                                                                             |
|    Here is a naive example:                                                 |
|                                                                             |
|    #include <entity/ensolve.h>                                              |
|                                                                             |
|    class MySolver : public EntitySolverBase {                               |
|    public:                                                                  |
|       istream * getExternalProducer(Entity const & ent) {                   |
|          return new ifstream(inqAbsolutePath());                            |
|       }                                                                     |
|       UnicodeString const & inqAbsolutePath(Entity const & ent) const {     |
|          if (ent.isSpecialDoc()) {                                          |
|             return new ifstream("myDocument.sgml");                         |
|          }else {                                                            |
|             return new ifstream("ent.inqExternId().inqSystemId()");         |
|          }                                                                  |
|       }                                                                     |
|    };                                                                       |
|                                                                             |
|                                                                             |
| 3) Write a main such as:                                                    |
|                                                                             |
|    int main()                                                               |
|    {                                                                        |
|        MySgmlInspection inspection;                                         |
|        MySolver solver;                                                     |
|        Yasp3 parser;                                                        |
|                                                                             |
|        parser.openDocument(solver);                                         |
|        while (parser.isMoreToParse()) {                                     |
|           parser.next().accept(inspection);                                 |
|        }                                                                    |
|        parser.closeDocument()                                               |
|        return 0;                                                            |
|    }                                                                        |
|                                                                             |
|    After closeDocument(), the DTD is still valid: you may re-openDocument   |
|    to start parsing another document instance of the same kind -- obeying   |
|    the same DTD.                                                            |
|                                                                             |
|                                                                             |
| Caller-controllable options:                                                |
| ----------------------------                                                |
|                                                                             |
| Set or unset by using the chevron operator: << or >>                        |
|    parser << YSP__killRERS;  // kill all RE/RS                              |
|                                                                             |
| YSP__killRERS                                                               |
|    Causes each sequence of carriage-returns and line-feeds to be            |
|    compressed into a single space character.  Those ignored under           |
|    the rules of 7.6.1 are still ignored rather than converted.              |
|                                                                             |
| YSP__killTAB                                                                |
|    Causes all tab characters to be turned into spaces.                      |
|                                                                             |
| YSP__killLEAD                                                               |
|    Strips leading blanks from text chunks.                                  |
|                                                                             |
| YSP__killTRAIL                                                              |
|    Strips trailing blanks from text chunks.                                 |
|                                                                             |
| YSP__keepLineBreak                                                          |
|    Keep line breaks: do not apply rule at 7.6.1.                            |
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API Yasp3 {                     // SGML parser
   friend class YaspSignalEventCB;

public:
   /*
   | Main public methods
   */
   Yasp3(
      UnicodeComposer & logger,
      SgmlDecl::SyntaxFlavour = SgmlDecl::SGML
   );
   ~Yasp3();

   bool openDocument(EntitySolverBase & solverArg);
   bool openDtdAsDocument(EntitySolverBase & solverArg);
   bool closeDocument();

   YaspEvent const & next();

   /*
   | Status inquiries
   */
   bool isParserOk() const;
   bool isMoreToParse() const;                 // doc opened, and not at EOF
   operator void *() const;                    // isOk?
   bool operator!() const;                     // is not ok?

   void errorContext();

   /*
   | options and settings
   */
   Yasp3 & operator<<(YaspOptName nameCode);   // set options
   Yasp3 & operator>>(YaspOptName nameCode);   // unset options
   int operator==(YaspOptName nameCode) const; // is this option set?
   int operator!=(YaspOptName nameCode) const; // is this option not set?
   Yasp3 & operator<<(e_EntKind entk);         // report this kind of entity
   Yasp3 & operator>>(e_EntKind entk);         // don't report this kind
   bool operator==(e_EntKind entk) const;      // do we report this kind?
   bool operator!=(e_EntKind entk) const;      // do we hide this kind?

   bool setOption(YaspOptName nameCode, bool isToSet);
   bool setTextBuffer(
      UCS_2 * pUcTextArg = 0,
      int iTextSizeArg = 0
   );
   void loadCachedDtd(CachedDtd const & dtd);

   /*
   | Miscellaneous constant inquiries
   */
   SgmlDecl const & inqSgmlDecl() const;        // SGML declaration
   EntManager const & inqEntManager() const;    // entity manager used
   ElmManager const & inqElmManager() const;    // element manager used
   EntitySolverBase & inqEntSolver() const;     // what entity solver
   int inqOption(YaspOptName nameCode) const;

   UnicodeString name(Delimiter::e_Ix dlmix) const;
   UnicodeString const & name(ReservedName::e_Ix ix) const;
   UnicodeString const & value(Delimiter::e_Ix dlmix) const;

   int inqTotalOffset() const;       // total number of bytes parsed so far
   int inqDepth() const;             // how many entities we're nested in

                                     // Relative to the current entity:
   UnicodeString getLastParsedChunk();
   Entity const & inqEntity() const; // what entity?
   int inqLine() const;              // what line?
   int inqColumn() const;            // what column?
   int inqOffset() const;            // number of characters parsed
   int inqSize() const;              // byte length (if avail - takes time)
   Encoder const & inqEncoderCurrent() const;      // current encoder ...
   EncodingModule::Id inqEncodingCurrent() const;  // or encoding of last external?

   /*
   | Manually, establish a parsing context
   */
   bool setEncodingCurrent(EncodingModule::Id encoding);

   bool setDoctype(UnicodeString const & ucsDoctypeName);

   bool addElementEmpty(UCS_2 const * pUcName);
   bool addElementCdata(UCS_2 const * pUcName);
   bool addElementRcdata(UCS_2 const * pUcName);
   bool addElement(
      UnicodeString const & ucsGi,
      ModelTree const & model,
      Attlist const & attlst
   );

   int inqElementIxOrCreate(UnicodeString const & ucsGi);
   bool startElement(UnicodeString const & ucsGi);
   bool endElement();

   /*
   | Stringize info not directly pertinent to the SGML document
   */
   UnicodeString extractXmlPi(XmlPiData const & xmlpi) const;

   /*
   | Public Data
   */
   static UnicodeString const docEntName;
   static UnicodeString const dtdEntName;

   class GrabGroupCB {
   public:
      virtual ~GrabGroupCB();
      virtual bool call(Yasp3 * p) = 0;
   };

private:
   friend class NameGrabGroupCB;
   friend class AttDclvGrabGroupCB;

   /*--------------------+
   | Private Definitions |
   +--------------------*/
   enum { BYPASS_WARNING=1024 }; // Warn if bypass goes more
   enum { DEFAULT_BUFFER_SIZE = 1024 };

   class YASP3_API Options {
   public:
      Options() {
         iOptions[0] = 0;
         iOptions[1] = 0;
         iOptions[2] = 0;
         iOptions[3] = 0;
      };
      bool operator==(YaspOptName opt) const {              // is set?
         if (iOptions[opt >> 4] & (1 << (opt & 0xF))) {
            return true;
         }else {
            return false;
         }
      }
      bool operator!=(YaspOptName opt) const {              // is not set?
         if (operator==(opt)) return false; else return true;
      }
      Options & operator<<(YaspOptName opt) {               // set.
         iOptions[opt >> 4] |= (1 << (opt & 0xF));
         return *this;
      }
      Options & operator>>(YaspOptName opt) {               // unset.
         iOptions[opt >> 4] &= ~(1 << (opt & 0xF));
         return *this;
      }
   private:
      unsigned int iOptions[4];
   };

   /*-------------+
   | Private Data |
   +-------------*/
   static UnicodeString const xmlPi;
   static UnicodeString const xmlVersion;
   static UnicodeString const xmlEncoding;
   static UnicodeString const xmlRequiredMarkup;

   bool bParserOk;
   UnicodeComposer & erh;         // to log errors
   EntitySolverBase * pSolver;    // to map an entity to a istream
   SgmlDecl sdcl;                 // SGML declaration
   DelimFinder dlmfnd;            // Delimiter finder
   EntManager entMgr;             // entity manager
   ElmManager elmMgr;             // element Manager
   RecordBinder rcdbnd;           // Solve rule at 7.6.1
   OpenEntityCache oec;           // Optimizer to access entities

   YaspEventText evText;          // for text events (special)
   UCS_2 * pUcText;               // text contents (internal/user)
   int iTextSize;                 // max length of it
   bool isTextOwned;              // if false, caller provides it
   YaspEvent evCurr;              // Current event
   YaspEvent evStartEntity;       // Delayed until real read
   YaspEventList evlst;           // FIFO. Events parsed on last call, waiting

   // Information about the current state of the parse

   // -- per document
   bool docIsOpen;                // Has YSPopenDocument been done?
   bool bMoreToParse;             // Did we reach the end (or FATAL_ERROR?)
   Delimiter::e_Ix pendingDelim;  // Delimiter we already found
   int iLineDelimitedTextStart;   // internal to getDelimitedText()

   // -- per session
   bool DTDrequested;             // Have we been asked/called for DOCTYPE?
   bool pastDOCTYPE;              // Have we finished <!DOCTYPE...[...]>
   int errorCount;                // Total number of user syntax errs
   int offset;                    // Last offset set by next()

   Options options;
   bool isExpanded[ENTK_last];    // What kinds of entities to replace

   /*----------------+
   | Private Methods |
   +----------------*/
   /*-- YaspMain --*/
   void initXmlEntities();
   bool tryProcessXmlPi();
   void openDoctype();
   bool addElement(UCS_2 const *, ModelTree::e_DeclaredClass);
   bool handleDelim(Delimiter::e_Ix delimFound);
   void dummyMethod();

   /*-- YaspGrab --*/
   void      bypassDoctypeDecl();
   void      bypassDecl();
   void      bypassGroup(int iNest);

   bool   skip_S_noFind();
   bool   skip_S();
   bool   skip_ts();
   bool   skip_ps();
   bool   skip_psRequired();
   bool   skip_ds();
   bool   skip(UnicodeString const & ucs, bool isNameCase);
   bool   skip(Delimiter::e_Ix dlmix);
   bool   skip(ReservedName::e_Ix ix);

   void      grabGroup(GrabGroupCB & cb);
   bool peek(
      UnicodeString const & ucs, bool isNameCase, int & iOffset
   );
   int peekName(UCS_2 * buf, bool isNamecase);
   int peekRefc(int iOffset);
   int grabNameGeneral(UCS_2 * buf);
   int grabNameEntity(UCS_2 * buf);
   int grabNameToken(UCS_2 * buf);
   int grabChoice(
      ReservedName::Choice const * aChoices,
      bool isReportErrors = true
   );
   UnicodeString getParameterLiteral(Delimiter::e_Ix &);
   UnicodeString getMinimumLiteral();
   UnicodeString getInternalPi();

   int grabDelimitedText(
      UCS_2 * buf, int max, Delimiter::e_Ix ixDlm
   );
   void skipMarkedSection();
   int grabText(UCS_2 *buf, int max);

   bool readXmlPiAttribute(
      UnicodeString const & ucsName,
      UnicodeString & ucsValue,
      UCS_2 & ucDelim
   );
   UnicodeString getXmlPiAttrValue(UCS_2 & ucDelim);
   bool endXmlPi();

   /*-- YaspParse --*/
   bool  parseContentModel(ModelTree & tree);
   bool  parseElementContent(ModelTree & tree);
   bool  parseNameGroup(TpIxList & ilist, bool isNotation = false);
   bool  parseExceptionGroup(TpIxList & ilist);
   void     parseElementDecl(YaspEvent & ev);
   void     parseAttlistDecl(YaspEvent & ev);
   ExternId parseExternId(ReservedName::e_Ix ixKey=ReservedName::IX_NOT_FOUND);
   void     parseEntityDecl(YaspEvent & ev);
   DataNotation parseDataNotation();
   void     parseNotationDecl(YaspEvent & ev);
   void     parseUsemap(YaspEvent & ev);
   void     parseDoctype(YaspEvent & ev, bool);
   void     parseMarkedSection(YaspEvent & ev);
   void handleDecl();

   /*-- YaspTag --*/
   bool endAllElements(int iMax = 0);
   void handleStartTag();
   void handleEndTag();
   void closeTag(bool isReportErrors);
   bool startPcData();

   UnicodeString grabValueSpec(
      Attribute const * pAtt,
      Delimiter::e_Ix & dlmStartRet
   );
   bool checkIdUnique(
      Attribute const * pAtt, UCS_2 const * pUcToken
   ) const;
   bool checkValueInList(
      Attribute const * pAtt, UCS_2 const * pUcToken
   ) const;
   bool checkEntity(
      Attribute const * pAtt, UCS_2 const * pUcToken
   ) const;
   bool checkDclVal(
      e_AttDclVal dclvalToCheck,
      Attribute const * pAtt,
      UCS_2 const * pUcToken
   ) const;

   bool parseAttSpecList(
      Attlist & attlst, bool & isConrefSpec
   );

   /*-- YaspInput --*/
   bool open(Entity const * pEnt, bool isReportErrors);
   Entity const * parseEntityReference(bool isGeneral, int & iLen);
   Entity const * dereferEntity(bool isGeneral);
   void registerEntityStart();
   bool handleEntity(bool isGeneral);
   bool resumePreviousEntity();
   UCS_2 dereferRegCharRef(Charset::e_CharType & ct, int & iLen);
   UCS_2 tryDereferHexCharRef(int & iLen, bool isKeepUcCharRef);

   /*-- YaspError --*/
   void startLocationMessage(Entity const &, bool);
   void showErrorLocation();

   /*-- YaspSdcl --*/
   bool   parseSgmlDecl(YaspEvent & ev);

};

/* -- INLINES -- */
inline bool Yasp3::isParserOk() const {
   return bParserOk;
}
inline bool Yasp3::isMoreToParse() const {
   return bMoreToParse;
}
inline Yasp3::operator void *() const {
   if (bParserOk) return (void *)this; else return 0;
}
inline bool Yasp3::operator!() const {      // is not ok?
   if (bParserOk) return false; else return true;
}
inline bool Yasp3::skip(Delimiter::e_Ix dlmix) {
   return skip(sdcl.delimList()[dlmix], false);
}
inline bool Yasp3::skip(ReservedName::e_Ix ix) {
   return skip(sdcl.reservedName()[ix], sdcl.charset().isCaseGeneral());
}
inline UnicodeString Yasp3::name(Delimiter::e_Ix dlmix) const {
   return sdcl.reservedName()[dlmix];
}
inline UnicodeString const & Yasp3::name(ReservedName::e_Ix ix) const {
   return sdcl.reservedName().operator[](ix);
}
inline UnicodeString const & Yasp3::value(Delimiter::e_Ix dlmix) const {
   return sdcl.delimList()[dlmix];
}
inline bool Yasp3::setEncodingCurrent(EncodingModule::Id encoding) {
   return entMgr.setEncodingCurrent(encoding);
}
inline Encoder const & Yasp3::inqEncoderCurrent() const {
   return entMgr.inqEncoderCurrent();
}
inline EncodingModule::Id Yasp3::inqEncodingCurrent() const {
   return entMgr.inqEncoderCurrent().inqEncoding();
}
inline SgmlDecl const & Yasp3::inqSgmlDecl() const {
   return sdcl;
}
inline int Yasp3::inqTotalOffset() const {
   return entMgr.inqTotalOffset();
}
inline int Yasp3::inqDepth() const {
   return entMgr.inqDepth();
}
inline Entity const & Yasp3::inqEntity() const {
   return entMgr.inqEntity();
}
inline int Yasp3::inqLine() const {
   return entMgr.inqLine();
}
inline int Yasp3::inqColumn() const {
   return entMgr.inqColumn();
}
inline int Yasp3::inqOffset() const {
   return entMgr.inqOffset();
}
inline int Yasp3::inqSize() const {
   return entMgr.inqSize();
}
inline EntManager const & Yasp3::inqEntManager() const {
   return entMgr;
}
inline ElmManager const & Yasp3::inqElmManager() const {
   return elmMgr;
}
inline EntitySolverBase & Yasp3::inqEntSolver() const {
   return *pSolver;
}
inline Yasp3 & Yasp3::operator<<(YaspOptName nameCode) {
   setOption(nameCode, true);
   return *this;
}
inline Yasp3 & Yasp3::operator>>(YaspOptName nameCode) {
   setOption(nameCode, false);
   return *this;
}
inline int Yasp3::operator==(YaspOptName nameCode) const {
   return inqOption(nameCode);
}
inline int Yasp3::operator!=(YaspOptName nameCode) const {
   if (inqOption(nameCode)) return false; else return true;
}
inline Yasp3 & Yasp3::operator<<(e_EntKind kind) {
   isExpanded[(int)kind] = true;
   return *this;
}
inline Yasp3 & Yasp3::operator>>(e_EntKind kind) {
   isExpanded[(int)kind] = false;
   return *this;
}
inline bool Yasp3::operator==(e_EntKind kind) const {
   return isExpanded[(int)kind];
}
inline bool Yasp3::operator!=(e_EntKind kind) const {
   if (isExpanded[(int)kind]) return false; else return true;
}
inline bool Yasp3::addElementEmpty(UCS_2 const * pUcName) {
   return addElement(pUcName, ModelTree::EMPTY);
}
inline bool Yasp3::addElementCdata(UCS_2 const * pUcName) {
   return addElement(pUcName, ModelTree::CDATA);
}
inline bool Yasp3::addElementRcdata(UCS_2 const * pUcName) {
   return addElement(pUcName, ModelTree::RCDATA);
}
#endif /* YASP3_HEADER ======================================================*/
