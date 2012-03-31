/* $Id: XmlIOMgr.h,v 1.14 2002-06-18 10:36:30 pgr Exp $ */

#ifndef COM_JAXO_YAXX_XMLIOMGR_H_INCLUDED
#define COM_JAXO_YAXX_XMLIOMGR_H_INCLUDED

#if !defined COM_JAXO_YAXX_DENY_XML

/*---------+
| Includes |
+---------*/
#include "../yasp3lib/entity/entsolve.h"
#include "Constants.h"
#include "../toolslib/LinkedList.h"

class CachedDtdList;

#ifdef TOOLS_NAMESPACE
namespace TOOL_NAMESPACE {
#endif
class UnicodeComposer;
#ifdef TOOLS_NAMESPACE
}
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class RexxString;
enum Signaled;

/*----------------------------------------------------------- class XmlIOMgr -+
|                                                                             |
+----------------------------------------------------------------------------*/
class XmlIOMgr : private Constants {
public:
   XmlIOMgr(CachedDtdList const & dtds, UnicodeComposer & erh);
   ~XmlIOMgr();

   void setDefaultXmlIn(istream & xmlIn);
   void setDefaultXmlOut(ostream & xmlOut);

   RexxString xmlin(RexxString & name, Signaled & sig);
   int xmlout(RexxString & name, RexxString & strOut, Signaled & sig);
   RexxString & state(RexxString & name);

private:

   class YaspMsgWatcher : public UnicodeComposer::Watcher {
   public:
      YaspMsgWatcher(Yasp3 & ysp) : m_ysp(ysp), isAttachment(false) {}
      bool notifyNewEntry(UnicodeComposer::Message & msg);
   private:
      bool isAttachment;
      Yasp3 & m_ysp;
   };

   class YaspInput : public LinkedList::Item, EntitySolverBase {
      friend class YaspMsgWatcher;
   public:
      YaspInput(
         RexxString & name,
         CachedDtdList const & dtds,
         UnicodeComposer & erh
      );
      YaspInput(
         istream & xmlIn,
         CachedDtdList const & dtds,
         UnicodeComposer & erh
      );
      ~YaspInput();
      bool isReady() const;
      RexxString nextEvent();

      RexxString m_strPrimaryPath;
   private:
      Yasp3 & m_ysp;
      CachedDtdList const & m_dtds;
      istream * m_pXmlIn;
      UnicodeComposer & m_erh;
      UnicodeString m_ucsPrimaryPath;
      Encoder encLocale;

      // resolution of EntitySolverBase pure virtuals
      istream * getExternalProducer(Entity const & ent);
      EncodingModule::Id inqEncoding(Entity const & ent) const;
      UnicodeString const & inqAbsolutePath(Entity const & ent) const;
   };

   class YaspOutput : public LinkedList::Item {
   public:
      YaspOutput(RexxString & name);
      YaspOutput(ostream & xmlOut);
      ~YaspOutput();

      Signaled write(RexxString & strOut);
      RexxString m_strPrimaryPath;
   private:
      ostream * m_pXmlOut;
   };

   UnicodeComposer & m_erh;
   CachedDtdList const & m_dtds;
   istream * m_istrDeflt;
   ostream * m_ostrDeflt;
   int m_iCacheIn;
   int m_iCacheOut;
   YaspInput * m_pInputDeflt;
   YaspOutput * m_pOutputDeflt;
   YaspInput * m_pInputCache;
   YaspOutput * m_pOutputCache;
   LinkedList m_inputs;
   LinkedList m_outputs;

   YaspInput * ensureInput(RexxString & name, UnicodeComposer & erh);
   YaspInput * findInput(RexxString & name);
   YaspOutput * ensureOutput(RexxString & name);
   YaspOutput * findOutput(RexxString & name);
};

inline bool XmlIOMgr::YaspInput::isReady() const {
   return (m_ysp.isParserOk() && m_ysp.isMoreToParse());
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif  // COM_JAXO_YAXX_DENY_XML
#endif  // COM_JAXO_YAXX_XMLIOMGR_H_INCLUDED
/*===========================================================================*/
