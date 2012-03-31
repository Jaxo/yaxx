/*
* $Id: options.cpp,v 1.4 2002-04-01 10:21:21 pgr Exp $
*
* Process command line options.
*/

/*---------+
| Includes |
+---------*/
#include "../reslib/resources.h"
#include "main.h"

/*----------------------------------------------------Sample01::Options::usage-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Sample01::Options::usage(UnicodeComposer & erh)
{
   erh << ECE__INFO << _SMP__USAGE00 << endm;
   erh << ECE__INFO << _SMP__USAGE01 << endm;
   erh << ECE__INFO << _SMP__USAGE02 << endm;
   erh << ECE__INFO << _SMP__USAGE03 << endm;
   erh << ECE__INFO << _SMP__USAGE04 << endm;
   erh << ECE__INFO << _SMP__USAGE05 << endm;
   erh << ECE__INFO << _SMP__USAGE06 << endm;
   erh << ECE__INFO << _SMP__USAGE07 << endm;
   erh << ECE__INFO << _SMP__USAGE08 << endm;
   erh << ECE__INFO << _SMP__USAGE09 << endm;
   erh << ECE__INFO << _SMP__USAGE10 << endm;
   erh << ECE__INFO << _SMP__USAGE11 << endm;
   erh << ECE__INFO << _SMP__USAGE12 << endm;
   erh << ECE__INFO << _SMP__USAGE13 << endm;
   erh << ECE__INFO << _SMP__USAGE14 << endm;
   erh << ECE__INFO << _SMP__USAGE15 << endm;
   erh << ECE__INFO << _SMP__USAGE16 << endm;
   erh << ECE__INFO << _SMP__USAGE17 << endm;
   erh << ECE__INFO << _SMP__USAGE18 << endm;
   erh << ECE__INFO << _SMP__USAGE19 << endm;
   erh << ECE__INFO << _SMP__USAGE20 << endm;
   erh << ECE__INFO << _SMP__USAGE21 << endm;
   erh << ECE__INFO << _SMP__USAGE22 << endm;
}

/*-------------------------------------------------Sample01::Options::getValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline bool Sample01::Options::getValue(
   char const * pszName, char const *& pszValue,
   int argc, char const * const * argv,
   UnicodeComposer & erh
) {
   if (strcmp(argv[0]+1, pszName)) {
      erh << ECE__ERROR << _SMP__OPTUNKNOWN << argv[0] << endm;
      return false;
   }else if ((argc <= 1) || (argv[1][0] == '-')) {
      erh << ECE__ERROR << _SMP__OPTNOVAL << argv[0] << endm;
      return false;
   }else {
      pszValue = argv[1];
      return true;
   }
}

inline bool Sample01::Options::getValue(
   char const * pszName, int & iValue,
   int argc, char const * const * argv,
   UnicodeComposer & erh
) {
   if (strcmp(argv[0]+1, pszName)) {
      erh << ECE__ERROR << _SMP__OPTUNKNOWN << argv[0] << endm;
      return false;
   }else if ((argc <= 1) || (argv[1][0] > '9') || (argv[1][0] < '0')) {
      erh << ECE__ERROR << _SMP__OPTNOVAL << argv[0] << endm;
      return false;
   }else {
      iValue = atoi(argv[1]);
      return true;
   }
}

/*--------------------------------------------------Sample01::Options::Options-+
| Runs down the argument list and records any options which were set.         |
|                                                                             |
| Options For Internal Use Only:                                              |
|    $$ permits to enter any options based on the numbers found in yspenum.h  |
|    To turn on  YSP__includeIgnoredMS = 12                                   |
|    and:        YSP__keepUcCharRef = 17                                      |
|    enter:      -$$ 12 -$$ 17                                                |
|    And DON'T GOOF!                                                          |
+----------------------------------------------------------------------------*/
Sample01::Options::Options(
   UnicodeComposer & erh,
   int argc,
   char const * const argv[]
) :
// iWhyPrintOnHold(BECAUSE_QUIET),   // quiet is by default
   iWhyPrintOnHold(FOR_NO_REASONS),   // quiet is by default
   iWaitLine(0),
   iWaitEid(0),
   iInterval(DEFAULT_REPORT_INTERVAL),
   iCountEmpties(0),
   pszLogPath(0)
{

   memset((void *)&is, '\0', sizeof is);
   is.bOk = 1;
   is.bShowLocation = 1;
   is.bPrintRegular = 1;
   is.bStamped = 1;

   char const * pszValue;
   char const * pszInputEncoding = 0;
   char const * pszOutputEncoding = 0;
   char const * pszDtd = 0;
   while (--argc) {
      const char *pch = *++argv;
      switch (*pch) {                  // keywords parm
      case '-':
         bool isStickyOptionOk;
         do {
            isStickyOptionOk = false;
            switch (*++pch) {
            case 'a':                  // -a
               is.bShowAuxInfo = 1;
               isStickyOptionOk = true;
               break;
            case 'd':                  // -dtd (dtd_filename)
               if (getValue("dtd", pszDtd, argc, argv, erh)) {
                  --argc, ++argv;
               }else {
                  is.bOk = 0;
               }
               break;
            case 'e':                  // -e (tag, tag, ...)
               if (argv[0][2]) {
                  erh << ECE__ERROR << _SMP__OPTUNKNOWN << argv[0] << endm;
                  is.bOk = 0;
               }else if ((argc <= 2) || (argv[1][0] == '-')) {
                  erh << ECE__ERROR << _SMP__OPTNOVAL << argv[0] << endm;
                  is.bOk = 0;
               }else {
                  --argc;
                  ppszEmpties = ++argv;
                  iCountEmpties = 1;   // in case of dup.
                  while ((argc > 2) && (argv[1][0] != '-')) {
                     ++iCountEmpties;
                     ++argv, --argc;
                  }
               }
               break;
            case 'f':                  // -f (input_filename)
               if (getValue("f", pszValue, argc, argv, erh)) {
                  ucsPrimaryPath = pszValue;
                  ++argv, --argc;
               }else {
                  is.bOk = 0;
               }
               break;
            case 'g':                  // -g -- long gi names
               is.bLongTagNames = 1;
               isStickyOptionOk = true;
               break;
            case 'h':                  // -h -- HTML
               is.bHtml = 1;
               isStickyOptionOk = true;
               break;
            case 'i':                  // -i (input_encoding)
               if (getValue("i", pszInputEncoding, argc, argv, erh)) {
                  ++argv, --argc;
               }else {
                  is.bOk = 0;
               }
               break;
            case 'k':                  // -k (output_encoding)
               if (getValue("k", pszOutputEncoding, argc, argv, erh)) {
                  ++argv, --argc;
               }else {
                  is.bOk = 0;
               }
               break;
            case 'l':
               if (getValue("log", pszLogPath, argc, argv, erh)) {
                  ++argv, --argc;
               }else {
                  is.bOk = 0;
               }
               break;
            case 'n':                  // -n -- Narrow
               is.bTruncateText = 1;
               isStickyOptionOk = true;
               break;
            case 'p':                  // -p -- printnonumbers
               is.bShowLocation = 0;
               isStickyOptionOk = true;
               break;
            case 'q':                  // -q -- quiet
               iWhyPrintOnHold |= BECAUSE_QUIET;
               isStickyOptionOk = true;
               break;
            case 'r':                  // -r -- reportInterval
               if (getValue("r", iInterval, argc, argv, erh)) {
                  ++argv, --argc;
               }else {
                  is.bOk = 0;
               }
               break;
            case 's':                  // -s -- stats
               is.bStats = 1;
               is.bCountNodes = 1;
               isStickyOptionOk = true;
               break;
            case 't':                  // -t / -tl (lineno) / -te (eid)
               iWhyPrintOnHold &= ~BECAUSE_QUIET;
               switch (pch[1]) {
               case 0:
                  break;
               case 'l':
                  if (getValue("tl", iWaitLine, argc, argv, erh)) {
                     ++argv, --argc;
                  }else {
                     is.bOk = 0;
                  }
                  break;
               case 'e':
                  if (getValue("te", iWaitEid, argc, argv, erh)) {
                     ++argv, --argc;
                  }else {
                     is.bOk = 0;
                  }
                  break;
               default:
                  isStickyOptionOk = true;
               }
               break;
            case 'u':                  // -u -- no stamps, UNDOCUMENTED
               is.bStamped = 0;
               isStickyOptionOk = true;
               break;
            case 'v':                  // -v -- validate
               is.bValidate = 1;
               isStickyOptionOk = true;
               break;
            case 'x':                  // -x -- XML
               is.bXml = 1;
               isStickyOptionOk = true;
               break;
            case 'y':                  // -y -- MS. UNDOCUMENTED
               is.bIncludeIgnoredMS = 1;
               isStickyOptionOk = true;
               break;
            case '~':
               is.bDontExpandPIorSDATA = 1;
               isStickyOptionOk = true;
               break;
            case '#':
               is.bCompressBlanks = 1;
               isStickyOptionOk = true;
               break;
            case ']':
               is.bReportEntities = 1;
               isStickyOptionOk = true;
               break;
            case '$':
               {
                  // Internal Use Only
                  int iValue;
                  if (getValue("$$", iValue, argc, argv, erh)) {
                     listWildOptions += iValue;
                     ++argv, --argc;
                  }else {
                     is.bOk = 0;
                  }
               }
               break;
            default:
               #ifdef _WIN32
               {
                  // Visual C++ 4.1 opt does not understand...
                  UCS_2 uc = UCS_2(*pch);
                  erh << ECE__ERROR << _SMP__OPTUNKNOWN << uc << endm;
               }
               #else
               erh << ECE__ERROR << _SMP__OPTUNKNOWN << *pch << endm;
               #endif
               is.bOk = 0;
               break;
            }
         }while (isStickyOptionOk && pch[1]);
         break;
      case '?':
         is.bOk = 0;
         break;
      default:
         erh << ECE__ERROR << _SMP__OPTMISPLACED << pch << endm;
         is.bOk = 0;
         break;
      }
   }

   /*--------------------+
   | Check Validity      |
   | against environment |
   +--------------------*/

   if (is.bOk) {
      if (pszDtd) {
         ucsDtd = UnicodeString(pszDtd, Encoder());
      }
      if (iWaitLine) {
         iWhyPrintOnHold |= BECAUSE_WAIT_LINE;
         erh << ECE__INFO << _SMP__OPTWAITLINE << iWaitLine << endm;
      }
      if (iWaitEid) {
         iWhyPrintOnHold |= BECAUSE_WAIT_EID;
         erh << ECE__INFO << _SMP__OPTWAITELT << iWaitEid << endm;
      }
      if (iInterval) {
         erh << ECE__INFO << _SMP__OPTCHKPTS << iInterval << endm;
      }
   }
   if (!is.bOk) {
      usage(erh);
   }
}

/*-----------------------------------------Sample01::Options::inqIsPrintOnHold-+
| Suppress report if we're not up to desired starting line.         |
+----------------------------------------------------------------------------*/
bool Sample01::Options::inqIsPrintOnHold(Yasp3 & ysp, int eidCurrent)
{
   if (iWhyPrintOnHold & BECAUSE_QUIET) return true;
   if (
      (iWhyPrintOnHold & BECAUSE_WAIT_LINE) &&
      (ysp.inqDepth()==1) && (ysp.inqLine() >= iWaitLine)
   ) {
      iWhyPrintOnHold &= ~BECAUSE_WAIT_LINE;
   }
   if ((iWhyPrintOnHold & BECAUSE_WAIT_EID) && (eidCurrent >= iWaitEid)) {
      iWhyPrintOnHold &= ~BECAUSE_WAIT_EID;
   }
   if (iWhyPrintOnHold == FOR_NO_REASONS) return false;
   return true;
}
/*===========================================================================*/
