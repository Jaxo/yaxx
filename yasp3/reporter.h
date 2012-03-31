/*
* $Id: reporter.h,v 1.3 2002-04-01 10:21:21 pgr Exp $
*/

#ifndef REPORTER_HEADER
#define REPORTER_HEADER

/*-------------------------------------------------------------------Reporter-+
|                                                                             |
+----------------------------------------------------------------------------*/
class Reporter {
public:
   Reporter(
      Yasp3 & yspArg,
      Options const & options,
      bool isFullAttlist,
      Writer & outArg
   );
   void printRegular(
      YaspEvent const & ev,
      int iDepthElt,
      int eidCurrent
   );
   bool operator!() const;
   operator void *() const;
private:
   class Leading {
   public:
      Leading();
      void write(streambuf * bp, char const * pchType, int iDepth);
   private:
      char pchS1[2];
      char pchType[22];
      char pchS2[2];
      char pchIndent[100];        // "_"s to track nesting level
   };
   enum {
      MAXREC = 1024,              // buffers to stringize infos
      CDATALIMIT = 40             // How much CDATA to be narrow
   };
   Writer & out;
   Yasp3 & ysp;
   YaspStringizer stringizer;
   UCS_2 textBuffer[MAXREC];
   Leading leading;

   bool isOk;
   bool const isShowLocation;
   bool const isShowAuxInfo;
   bool const isTruncateText;

   static UnicodeString const ucsEllipsis;
   char const * pchEllipsis;
   int iLenEllipsis;
   static char const * const pchError;
   int const iLenError;
};

#endif /* REPORTER_HEADER ===================================================*/
