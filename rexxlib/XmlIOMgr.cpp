/* $Id: XmlIOMgr.cpp,v 1.36 2011-08-02 15:20:03 pgr Exp $ */
#if !defined COM_JAXO_YAXX_DENY_XML

#include "../toolslib/uccompos.h"
#include "../toolslib/SystemContext.h"
#include "../yasp3lib/yasp3.h"
#include "../yasp3lib/parser/CachedDtd.h"
#include "RexxString.h"
#include "Exception.h"
#include "IOMgr.h"
#include "XmlIOMgr.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-------------------------------------------XmlIOMgr::YaspOutput::YaspOutput-+
|                                                                             |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspOutput::YaspOutput(RexxString & primaryPath) :
   m_strPrimaryPath(primaryPath)
{
   assert (primaryPath.exists());
   m_pXmlOut = SystemContext::makeStream(primaryPath, ios::out);
}

/*-------------------------------------------XmlIOMgr::YaspOutput::YaspOutput-+
| Constructor for the default entry                                           |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspOutput::YaspOutput(ostream & xmlOut) :
   m_strPrimaryPath("[XMLOUT]")
{
   m_pXmlOut = new ostream(xmlOut.rdbuf());
}

/*------------------------------------------------XmlIOMgr::YaspOutput::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
Signaled XmlIOMgr::YaspOutput::write(RexxString & strOut) {
   m_pXmlOut->write(strOut, strOut.length());
   return SIG_NONE;
}

/*------------------------------------------XmlIOMgr::YaspOutput::~YaspOutput-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspOutput::~YaspOutput() {
   delete m_pXmlOut;
}

/*-----------------------------------XmlIOMgr::YaspInput::getExternalProducer-+
| Effects:                                                                    |
|    Get an appropriate istream to read an external entity.                   |
| Returns:                                                                    |
|    Pointer to the stream or 0                                               |
+----------------------------------------------------------------------------*/
istream * XmlIOMgr::YaspInput::getExternalProducer(Entity const & ent)
{
   if (ent.isSpecialDoc()) {
      return m_pXmlIn;
   }else if (ent.isSpecialDtd()) {
      /*
      | THIS IS UNFINISHED -- >>>PGR: FIXME.
      | We should analyze the entity, and find out what it is, then
      | default to HTML if no clue.  Also, setup the YASP3 context
      | accordingly (HTML, XML, SGML, w. or w/o ENTREF)
      | There is an example in../yasp3/solver.cpp: again, pay attention to
      | the HTML option which is turned on.  Actually, YASP3 is constructed
      | being HTML_WITH_NO_ENTREF (search for this string in this module.)
      | This might change...
      */
      CachedDtd const * pDtd = m_dtds["HTML"];
      if (pDtd) {
         m_ysp << YSP__HTML;
         m_ysp.loadCachedDtd(*pDtd);
         return 0;
      }else {
         assert (false);      // HTML should have been preloaded!
         m_ysp.setDoctype(UnicodeString("HTML"));
         return new MemStream(
            getHtmlDtd(), getHtmlDtdLength(), MemStream::Constant
            /*
            | If you want to test the minimal DTD, use:
            | "<!ELEMENT HTML O O ANY>", 23, MemStream::Constant
            */
         );
      }
   }else {
      ExternId const & exid = ent.inqExternId();
      MultibyteString const mbsSystemId(exid.inqSystemId(), encLocale);
      return SystemContext::makeStream(mbsSystemId.inqString(), ios::in);
   }
}

/*---------------------------------------------------------XmlIOMgr::XmlIOMgr-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
XmlIOMgr::XmlIOMgr(CachedDtdList const & dtds, UnicodeComposer & erh) :
   m_dtds(dtds),
   m_erh(erh),
   m_istrDeflt(&SystemContext::cin()),
   m_ostrDeflt(&SystemContext::cout()),
   m_pInputDeflt(0),
   m_pOutputDeflt(0),
   m_iCacheIn(-1),
   m_iCacheOut(-1) {
}

/*--------------------------------------------------------XmlIOMgr::~XmlIOMgr-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
XmlIOMgr::~XmlIOMgr() {
   delete m_pInputDeflt;
   delete m_pOutputDeflt;
}

/*--------------------------------------------------XmlIOMgr::setDefaultXmlIn-+
|                                                                             |
+----------------------------------------------------------------------------*/
void XmlIOMgr::setDefaultXmlIn(istream & xmlIn) {
   delete m_pInputDeflt;
   m_istrDeflt = &xmlIn;
}

/*-------------------------------------------------XmlIOMgr::setDefaultXmlOut-+
|                                                                             |
+----------------------------------------------------------------------------*/
void XmlIOMgr::setDefaultXmlOut(ostream & xmlOut) {
   delete m_pOutputDeflt;
   m_ostrDeflt = &xmlOut;
}

/*--------------------------------------------------------XmlIOMgr::findInput-+
|                                                                             |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspInput * XmlIOMgr::findInput(RexxString & primaryPath)
{
   assert (primaryPath.exists());
   if ((m_iCacheIn >= 0) && (primaryPath == m_pInputCache->m_strPrimaryPath)) {
      return m_pInputCache;
   }else {
      int max = m_inputs.size();
      LinkedList::Iterator it(m_inputs);
      for (int ix=0; ix < max; ++ix) {
         YaspInput * entry = (YaspInput *)it();
         if (primaryPath == entry->m_strPrimaryPath) {
            m_pInputCache = entry;
            m_iCacheIn = ix;
            return entry;
         }
      }
   }
   return 0;
}

/*-------------------------------------------------------XmlIOMgr::findOutput-+
|                                                                             |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspOutput * XmlIOMgr::findOutput(RexxString & primaryPath)
{
   assert (primaryPath.exists());
   if ((m_iCacheOut >= 0) && (primaryPath == m_pOutputCache->m_strPrimaryPath)) {
      return m_pOutputCache;
   }else {
      int max = m_outputs.size();
      LinkedList::Iterator it(m_outputs);
      for (int ix=0; ix < max; ++ix) {
         YaspOutput * entry = (YaspOutput *)it();
         if (primaryPath == entry->m_strPrimaryPath) {
            m_pOutputCache = entry;
            m_iCacheOut = ix;
            return entry;
         }
      }
   }
   return 0;
}

/*------------------------------------------------------XmlIOMgr::ensureInput-+
|                                                                             |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspInput * XmlIOMgr::ensureInput(
   RexxString & primaryPath,
   UnicodeComposer & erh
) {
   YaspInput * entry;
   if (!primaryPath.exists()) {
      if (!m_pInputDeflt) {
         m_pInputDeflt = new YaspInput(*m_istrDeflt, m_dtds, erh);
      }
      entry = m_pInputDeflt;
   }else {
      entry = findInput(primaryPath);
      if (!entry) {
         entry = new YaspInput(primaryPath, m_dtds, erh);
         if (!entry->isReady()) {
            delete entry;
            return 0;
         }
         m_inputs.add(entry);
      }
   }
   return entry;
}

/*-----------------------------------------------------XmlIOMgr::ensureOutput-+
|                                                                             |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspOutput * XmlIOMgr::ensureOutput(
   RexxString & primaryPath
) {
   YaspOutput * entry;
   if (!primaryPath.exists()) {
      if (!m_pOutputDeflt) {
         m_pOutputDeflt = new YaspOutput(*m_ostrDeflt);
      }
      entry = m_pOutputDeflt;
   }else {
      entry = findOutput(primaryPath);
      if (!entry) {
         entry = new YaspOutput(primaryPath);
         m_outputs.add(entry);
      }
   }
   return entry;
}

/*------------------------------------------------------------XmlIOMgr::state-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString & XmlIOMgr::state(RexxString & primaryPath)
{
   YaspInput * entry = findInput(primaryPath);
   if (!entry) {
      if (findOutput(primaryPath)) {
         return Constants::STR_READY;
      }else {
         return Constants::STR_UNKNOWN;
      }
   }else if (!entry->isReady()) {
      return Constants::STR_NOTREADY;
   }else {
      return Constants::STR_READY;
   }
}

/*------------------------------------------------------------XmlIOMgr::xmlin-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString XmlIOMgr::xmlin(RexxString & primaryPath, Signaled & sig)
{
   YaspInput * entry = ensureInput(primaryPath, m_erh);
   if (!entry || !entry->isReady()) {
      sig = SIG_NOTREADY;
      return RexxString(YaspEvent(YSP__PARSE_END));
   }else {
      sig = SIG_NONE;
      return entry->nextEvent();
   }
}

/*-----------------------------------------------------------XmlIOMgr::xmlout-+
|                                                                             |
+----------------------------------------------------------------------------*/
int XmlIOMgr::xmlout(
   RexxString & primaryPath, RexxString & strOut, Signaled & sig
) {
   YaspOutput * entry = ensureOutput(primaryPath);
   if (entry) {
      sig = entry->write(strOut);
   }else {
      sig = SIG_NOTREADY;
   }
   return 0;            // number of chunks remaining to write
}

/*---------------------------------------------XmlIOMgr::YaspInput::YaspInput-+
|                                                                             |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspInput::YaspInput(
   RexxString & primaryPath,
   CachedDtdList const & dtds,
   UnicodeComposer & erh
) :
   m_strPrimaryPath(primaryPath),
   m_ucsPrimaryPath((char const *)primaryPath),
   m_ysp(*(new Yasp3(erh, SgmlDecl::HTML_WITH_NO_ENTREF))),
   m_erh(erh),
   m_dtds(dtds)
{
   assert (primaryPath.exists());
   //>>>PGR: this is deleted by the entity manager and... this is BAD,
   //        since it violates the gold rule: who creates it deletes it.
   //        This was so because of the bad INSO habits, but I can fix it
   //        now.  So I'll go:
   // - getProducer returns a streambuffer (use rdbuf())
   // - then the entity manager build its own stream above it
   //   (if it uses new, that's his problem)
   // No time today...  but this should be red priority.
   m_pXmlIn = SystemContext::makeStream(primaryPath, ios::in);
   m_ysp.openDocument(*this);
}

/*---------------------------------------------XmlIOMgr::YaspInput::YaspInput-+
| Constructor for the default entry                                           |
+----------------------------------------------------------------------------*/
static char const defaultXmlInPath[] = "[XMLIN]";
XmlIOMgr::YaspInput::YaspInput(
   istream & xmlIn,
   CachedDtdList const & dtds,
   UnicodeComposer & erh
) :
   m_strPrimaryPath(defaultXmlInPath),
   m_ucsPrimaryPath(defaultXmlInPath),
   m_ysp(*(new Yasp3(erh, SgmlDecl::HTML_WITH_NO_ENTREF))),
   m_erh(erh),
   m_dtds(dtds)
{
   //>>>PGR: caution!  this is deleted by the entity manager (see above)
   m_pXmlIn = new istream(xmlIn.rdbuf());  // so it may be safely deleted
   m_ysp.openDocument(*this);
}

/*--------------------------------------------XmlIOMgr::YaspInput::~YaspInput-+
|                                                                             |
+----------------------------------------------------------------------------*/
XmlIOMgr::YaspInput::~YaspInput() {
   m_ysp.closeDocument();
   delete &m_ysp;
}

/*---------------------------------------------XmlIOMgr::YaspInput::nextEvent-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString XmlIOMgr::YaspInput::nextEvent()
{
   assert (isReady());
   m_erh.pushWatcher(new YaspMsgWatcher(m_ysp));
   for (;;) {
      YaspEvent const & ev = m_ysp.next();
      switch (ev.inqType()) {
      case YSP__TAG_START:        // Tag
      case YSP__TAG_END:
      case YSP__TAG_EMPTY:
      case YSP__TEXT:
      case YSP__COMMENT_TEXT:
      case YSP__PI_TEXT:
      case YSP__SDATA_TEXT:       // TextData
      case YSP__CDATA_TEXT:
      case YSP__PARSE_START:
      case YSP__PARSE_END:
         m_erh.popWatcher();
         return ev;
      case YSP__ERROR:
         return m_ysp.getLastParsedChunk();
      default:
         continue;
      }
   }
}

/*---------------------------------------XmlIOMgr::YaspInput::inqAbsolutePath-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString const & XmlIOMgr::YaspInput::inqAbsolutePath(Entity const & ent) const {
   if (ent.isSpecialDoc()) {
      return m_ucsPrimaryPath;
   }else {
      ExternId const & exid = ent.inqExternId();
      return exid.inqSystemId();
   }
}

/*-------------------------------------------XmlIOMgr::YaspInput::inqEncoding-+
| Effects:                                                                    |
|    Get the encoding attached to an entity.                                  |
| Note:                                                                       |
|   THIS IS NOT YET IMPLEMENTED.                                              |
+----------------------------------------------------------------------------*/
EncodingModule::Id XmlIOMgr::YaspInput::inqEncoding(Entity const & ent) const
{
   return encLocale.inqEncoding();   // not implemented.
}

/*-----------------------------------XmlIOMgr::YaspMsgWatcher::notifyNewEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool XmlIOMgr::YaspMsgWatcher::notifyNewEntry(UnicodeComposer::Message & msg)
{
   SystemContext::cerr() << msg.stringize() << std::endl;
   if (!isAttachment) {
      isAttachment = true;
      m_ysp.errorContext();
      isAttachment = false;
   }
   return true;
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif  // COM_JAXO_YAXX_DENY_XML
/*===========================================================================*/
