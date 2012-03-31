/* $Id: CachedDtd.h,v 1.4 2002-04-24 03:08:13 jlatone Exp $ */

#ifndef COM_JAXO_YAXX_CACHEDDTD_H_INCLUDED
#define COM_JAXO_YAXX_CACHEDDTD_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "../../toolslib/tpset.h"
#include "../yasp3.h"
#include "../element/elmmgr.h"
#include "../entity/entsolve.h"
#include "../entity/entmgr.h"

/*------------------------------------------------------------------CachedDtd-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API CachedDtd : public RefdKeyRep {
public:
   CachedDtd(
      char const * doctypeName,
      ElmManager const & elmMgr,
      EntManager const & entMgr
   );
   void load(ElmManager & elmMgr, EntManager & entMgr) const;

private:
   ElementList m_elmlst;
   EntityList m_entlstGeneral;
   EntityList m_entlstParam;
   NotationList m_ncblst;
   RefdKey m_kiEntDoctype;
   RefdKey m_kiEntDefault;
};

/*--------------------------------------------------------------CachedDtdList-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API CachedDtdList : public TpSet {
public:
   CachedDtd * operator[](UnicodeString const & ucs) const {
      return (CachedDtd *)findData(ucs); }
// TP_MakeKeyCollection(CachedDtd);
};

/*-------------------------------------------------------CachedDtdListFactory-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API CachedDtdListFactory : public EntitySolverBase {
public:
   CachedDtdListFactory(
      UnicodeComposer & erh,
      CachedDtdList & list
   );
   ~CachedDtdListFactory();
   void addDtd(
      char const * docType,
      istream * pIst      // CAUTION: it will be deleted (PGR should FIXME)
   );

private:
   Yasp3 & m_ysp;	
   istream * m_pIst;
   Encoder encLocale;
   CachedDtdList & m_list;

   // resolution of EntitySolverBase pure virtuals
   istream * getExternalProducer(Entity const & ent);
   EncodingModule::Id inqEncoding(Entity const & ent) const;
   UnicodeString const & inqAbsolutePath(Entity const & ent) const;
};

inline CachedDtdListFactory::CachedDtdListFactory(
   UnicodeComposer & erh,
   CachedDtdList & list
) : m_ysp(*(new Yasp3(erh))), m_list(list) {}
#endif
/*===========================================================================*/
