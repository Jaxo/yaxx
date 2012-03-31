/*
* $Id: main.h,v 1.4 2002-04-01 10:21:21 pgr Exp $
*/

#ifndef MAIN_HEADER
#define MAIN_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3lib/yasp3.h"
#include "../toolslib/uccompos.h"

/*-------------------------------------------------------------------Sample01-+
|                                                                             |
+----------------------------------------------------------------------------*/
class Sample01 :
   private YaspEventInspector
{
public:
   Sample01(UnicodeComposer & erhArg, int argc, char const * const argv[]);
   bool operator!() const;
   operator void *() const;
   void run();

   #include "options.h"
   #include "reporter.h"

private:
   UnicodeComposer & erh;   // must be initialized before Options
   Options options;         // must be initialized before Yasp3
   Yasp3 ysp;

   enum { MAX_TAGDEPTH  = 1024 };
   int eidCurrent;
   int iDepthElt;
   bool bOk;

   void bumpEid();
   void inspectTagEmpty(Tag const & tag);
   void inspectTagStart(Tag const & tag);
   void inspectTagEnd(Tag const & tag);
   void inspectSdataText(TextData const & text);
   void inspectCdataText(TextData const & text);
   void inspectText(TextData const & text);
   void inspectEntityRef(Entity const & ent);
   void inspectEntityStart(Entity const & ent);
   void inspectEntityCantOpen(Entity const & ent);
   void inspectDtdRequest(Entity const & ent);
};

/*--------------------------------------------------------------------INLINES-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline bool Sample01::operator!() const {
   if (bOk) return false; else return true;
}
inline Sample01::operator void *() const {
   if (bOk) return (void *)this; else return 0;
}
inline bool Sample01::Options::isOk() const {
   if (is.bOk) return true; else return false;
}
inline bool Sample01::Options::isShowLocation() const {
   if (is.bShowLocation) return true; else return false;
}
inline bool Sample01::Options::isShowAuxInfo() const {
   if (is.bShowAuxInfo) return true; else return false;
}
inline bool Sample01::Options::isTruncateText() const {
   if (is.bTruncateText) return true; else return false;
}
inline bool Sample01::Options::isHtml() const {
   if (is.bHtml) return true; else return false;
}
inline bool Sample01::Options::isValidate() const {
   if (is.bValidate) return true; else return false;
}
inline bool Sample01::Options::isStats() const {
   if (is.bStats) return true; else return false;
}
inline bool Sample01::Options::isLongTagNames() const {
   if (is.bLongTagNames) return true; else return false;
}
inline bool Sample01::Options::isCompressBlanks() const {
   if (is.bCompressBlanks) return true; else return false;
}
inline bool Sample01::Options::isIncludeIgnoredMS() const {
   if (is.bIncludeIgnoredMS) return true; else return false;
}
inline bool Sample01::Options::isReportEntities() const {
   if (is.bReportEntities) return true; else return false;
}
inline bool Sample01::Options::isDontExpandPIorSDATA() const {
   if (is.bDontExpandPIorSDATA) return true; else return false;
}
inline bool Sample01::Options::isDontExpandExternal() const {
   if (is.bDontExpandExternal) return true; else return false;
}
inline bool Sample01::Options::isCountNodes() const {
   if (is.bCountNodes) return true; else return false;
}
inline bool Sample01::Options::isPrintRast() const {
   if (is.bPrintRast) return true; else return false;
}
inline bool Sample01::Options::isPrintNormalized() const {
   if (is.bPrintNormalized) return true; else return false;
}
inline bool Sample01::Options::isPrintMinimized() const {
   if (is.bPrintMinimized) return true; else return false;
}
inline bool Sample01::Options::isPrintRegular() const {
   if (is.bPrintRegular) return true; else return false;
}
inline bool Sample01::Options::isXml() const {
   if (is.bXml) return true; else return false;
}
inline bool Sample01::Options::isStamped() const {
   if (is.bStamped) return true; else return false;
}
inline UnicodeString const & Sample01::Options::inqDtd() const {
   return ucsDtd;
}
inline char const * Sample01::Options::inqEmpty(int i) const {
   if (i >= iCountEmpties) return 0; else return ppszEmpties[i];
}
inline int Sample01::Options::inqInterval() const {
   return iInterval;
}
inline UnicodeString const & Sample01::Options::inqPrimaryPath() const {
   return ucsPrimaryPath;
}
inline char const * Sample01::Options::inqLogPath() const {
   return pszLogPath;
}
inline TpIxList const & Sample01::Options::inqListWildOptions() const {
   return listWildOptions;
}
inline bool Sample01::Options::isPrintOnHold(Yasp3 & ysp, int eidCurrent) {
   if (iWhyPrintOnHold == FOR_NO_REASONS) {
      return false;
   }else {
      return inqIsPrintOnHold(ysp, eidCurrent);
   }
}
inline bool Sample01::Reporter::operator!() const {
   if (isOk) return false; else return true;
}
inline Sample01::Reporter::operator void *() const {
   if (isOk) return (void *)this; else return 0;
}
#endif /* MAIN_HEADER =======================================================*/
