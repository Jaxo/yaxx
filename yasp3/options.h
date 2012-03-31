/*
* $Id: options.h,v 1.2 2002-04-01 10:21:21 pgr Exp $
*/

#ifndef OPTIONS_HEADER
#define OPTIONS_HEADER

/*--------------------------------------------------------------------Options-+
|                                                                             |
+----------------------------------------------------------------------------*/
class Options {
public:
   Options(
      UnicodeComposer & erh,
      int argc,
      char const * const argv[]
   );
   static void usage(UnicodeComposer & erh);

   bool isOk() const;
   bool isPrintOnHold(Yasp3 & ysp, int eidCurrent);
   bool isShowLocation() const;
   bool isShowAuxInfo() const;
   bool isTruncateText() const;
   bool isValidate() const;
   bool isStats() const;
   bool isCompressBlanks() const;
   bool isIncludeIgnoredMS() const;
   bool isReportEntities() const;
   bool isDontExpandPIorSDATA() const;
   bool isDontExpandExternal() const;
   bool isCountNodes() const;
   bool isLongTagNames() const;
   bool isPrintRast() const;
   bool isPrintNormalized() const;
   bool isPrintMinimized() const;
   bool isPrintRegular() const;
   bool isStamped() const;
   bool isHtml() const;
   bool isXml() const;

   UnicodeString const & inqDtd() const;           // DTD file-name
   UnicodeString const & inqPrimaryPath() const;   // primary path
   char const * inqEmpty(int i) const;             // get empty element i
   int inqInterval() const;                        // between checkpoints
   char const * inqLogPath() const;                // Where to write the log
   TpIxList const & inqListWildOptions() const;    // Internal Use Only

private:
   enum {
     FOR_NO_REASONS    = 0,
     BECAUSE_QUIET     = 1 << 0,
     BECAUSE_WAIT_EID  = 1 << 1,
     BECAUSE_WAIT_LINE = 1 << 2,
     DEFAULT_REPORT_INTERVAL = 500   // How often to print check ponts
   };
   struct Flags {
      unsigned int bOk : 1;
      unsigned int bShowLocation : 1;
      unsigned int bShowAuxInfo : 1;
      unsigned int bTruncateText : 1;
      unsigned int bHtml : 1;
      unsigned int bValidate : 1;
      unsigned int bStats : 1;
      unsigned int bLongTagNames : 1;
      unsigned int bCompressBlanks : 1;
      unsigned int bIncludeIgnoredMS : 1;
      unsigned int bReportEntities : 1;
      unsigned int bDontExpandPIorSDATA : 1;
      unsigned int bDontExpandExternal : 1;
      unsigned int bCountNodes : 1;
      unsigned int bPrintRast : 1;
      unsigned int bPrintNormalized : 1;
      unsigned int bPrintMinimized : 1;
      unsigned int bPrintRegular : 1;
      unsigned int bStamped : 1;
      unsigned int bXml : 1;
   };

   Flags is;
   int iWhyPrintOnHold;
   int iWaitLine;                    // Line to start output at
   int iWaitEid;                     // Element to start output at
   int iInterval;                    // How often to checkpoint
   UnicodeString ucsPrimaryPath;

   int iCountEmpties;
   char const * const * ppszEmpties; // List of SGML EMPTY tags

   char const * pszLogPath;          // log file
   TpIxList listWildOptions;         // Internal Use Only!

   UnicodeString ucsDtd;             // DTD file-name

   bool inqIsPrintOnHold(Yasp3 & ysp, int eidCurrent);
   static bool getValue(
      char const * pszName, char const *& pszValue,
      int argc, char const * const * argv,
      UnicodeComposer & erh
   );
   static bool getValue(
      char const * pszName, int & iValue,
      int argc, char const * const * argv,
      UnicodeComposer & erh
   );
};

#endif /* OPTIONS_HEADER ====================================================*/
