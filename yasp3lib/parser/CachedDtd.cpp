/* $Id: CachedDtd.cpp,v 1.9 2002-08-31 10:23:49 pgr Exp $ */

#include <assert.h>
#include "CachedDtd.h"
#ifdef __MWERKS__
#define XxxFileInStream PalmFileStream
#else
#define XxxFileInStream StdFileInStream
#endif

/*-------------------------------------------------------CachedDtd::CachedDtd-+
|                                                                             |
+----------------------------------------------------------------------------*/
CachedDtd::CachedDtd(
   char const * doctypeName,  // catenate the Doctype || SysId || PubId ?
   ElmManager const & elmMgr,
   EntManager const & entMgr
) :
   RefdKeyRep(doctypeName),
   m_elmlst(elmMgr.inqListElement()),
   m_entlstGeneral(entMgr.inqListEntityGeneral()),
   m_entlstParam(entMgr.inqListEntityParameter()),
   m_ncblst(entMgr.inqListNotation()),
   m_kiEntDoctype(entMgr.inqEntityDoctype()),
   m_kiEntDefault(entMgr.inqEntityDefault())
{
}

/*------------------------------------------------------------CachedDtd::load-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CachedDtd::load(ElmManager & elmMgr, EntManager & entMgr) const
{
   elmMgr.setListElement(m_kiEntDoctype, m_elmlst);
   entMgr.setLists(
      m_entlstGeneral,
      m_entlstParam,
      m_ncblst,
      m_kiEntDoctype,
      m_kiEntDefault
   );
}

/*-----------------------------------------------CachedDtdListFactory::addDtd-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CachedDtdListFactory::addDtd(
   char const * doctypeName,
   istream * pIst
) {
   m_pIst = pIst;
   bool isOk = m_ysp.openDtdAsDocument(*this);
   while (m_ysp.isMoreToParse()) {
      YaspEvent const & ev = m_ysp.next();
   }
   m_ysp.setDoctype(UnicodeString(doctypeName));
   m_list += new CachedDtd(
      doctypeName,
      m_ysp.inqElmManager(),
      m_ysp.inqEntManager()
   );
   m_ysp.closeDocument();
}

/*----------------------------------CachedDtdListFactory::getExternalProducer-+
| Effects:                                                                    |
|    Get an appropriate istream to read an external entity.                   |
| Returns:                                                                    |
|    Pointer to the stream or 0                                               |
+----------------------------------------------------------------------------*/
istream * CachedDtdListFactory::getExternalProducer(Entity const & ent)
{
   if (ent.isSpecialDoc()) {
      return m_pIst;
   }else if (ent.isSpecialDtd()) {
      assert (false);
      return 0;
   }else {
      ExternId const & exid = ent.inqExternId();
      MultibyteString const mbsSystemId(exid.inqSystemId(), encLocale);
      return new XxxFileInStream(mbsSystemId.inqString());
   }
}

/*--------------------------------------CachedDtdListFactory::inqAbsolutePath-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString const & CachedDtdListFactory::inqAbsolutePath(
   Entity const & ent
) const {
   assert (ent.isSpecialDoc() == false);
   return ent.inqExternId().inqSystemId();
}

/*------------------------------------------CachedDtdListFactory::inqEncoding-+
| Effects:                                                                    |
|    Get the encoding attached to an entity.                                  |
| Note:                                                                       |
|   THIS IS NOT YET IMPLEMENTED.                                              |
+----------------------------------------------------------------------------*/
EncodingModule::Id CachedDtdListFactory::inqEncoding(Entity const & ent) const
{
   return encLocale.inqEncoding();   // not implemented.
}

/*-------------------------------~CachedDtdListFactory::~CachedDtdListFactory-+
|                                                                             |
+----------------------------------------------------------------------------*/
CachedDtdListFactory::~CachedDtdListFactory() {
   delete &m_ysp;
}

/*===========================================================================*/
