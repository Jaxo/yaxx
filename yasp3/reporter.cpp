/*
* $Id: reporter.cpp,v 1.5 2002-04-20 05:34:25 jlatone Exp $
*
* Reports each SGML events with the appropriate infos.
*/

/*---------+
| Includes |
+---------*/
#include <stdio.h>
#include "iomanip.h"
#include "main.h"

UnicodeString const Sample01::Reporter::ucsEllipsis("...");
char const * const Sample01::Reporter::pchError =
   "\n. ****************************************\n\n";

/*------------------------------------------------Sample01::Reporter::Reporter-+
|                                                                             |
+----------------------------------------------------------------------------*/
Sample01::Reporter::Reporter(
   Yasp3 & yspArg,
   Options const & options,
   bool isFullAttlist,
   Writer & outArg
) :
   out(outArg),
   ysp(yspArg),
   stringizer(
      out,
      isFullAttlist
   ),
   isShowLocation(options.isShowLocation()),
   isShowAuxInfo(options.isShowAuxInfo()),
   isTruncateText(options.isTruncateText()),
   iLenError(strlen(pchError)),
   isOk(true)
{
   UCS_2 const * pUcEllipsis = ucsEllipsis;
   pchEllipsis = (char const *)pUcEllipsis;
   iLenEllipsis = (ucsEllipsis.length() + 1) * sizeof(UCS_2);
   if (!ysp || !ysp.setTextBuffer(textBuffer, sizeof textBuffer / sizeof textBuffer[0])) {
      isOk = false;
   }
}

/*----------------------------------------Sample01::Reporter::Leading::Leading-+
|                                                                             |
+----------------------------------------------------------------------------*/
Sample01::Reporter::Leading::Leading()
{
   pchS1[0] = ' ';
   pchS1[1] = '(';
   pchS2[0] = ')';
   pchS2[1] = ' ';
   memset(pchIndent, '_', sizeof pchIndent);
}

/*------------------------------------------Sample01::Reporter::Leading::write-+
| Leading has is the type, followed by iDepth '_' characters.                 |
+----------------------------------------------------------------------------*/
void Sample01::Reporter::Leading::write(
   streambuf * bp, char const * pchTypeArg, int iDepth
) {
   int iLen = strlen(pchTypeArg);
   if (iLen <= sizeof pchType) {
      memcpy(pchType, pchTypeArg, iLen);
      memset(pchType+iLen, ' ', sizeof pchType - iLen);
   }else {
      memcpy(pchType, pchTypeArg, sizeof pchType);
   }
   if (iDepth <= sizeof pchIndent) {
      bp->sputn(pchS1, sizeof pchS1 + sizeof pchType + sizeof pchS2 + iDepth);
   }else {
      sprintf(pchIndent, "(%ld)_", (long)iDepth);
      iLen = strlen(pchIndent);
      bp->sputn(pchS1, sizeof pchS1 + sizeof pchType + sizeof pchS2  + iLen);
      memset(pchIndent, '_', iLen+1);
   }
}

/*--------------------------------------------Sample01::Reporter::printRegular-+
| Write out the report that follows each event parsed.                        |
+----------------------------------------------------------------------------*/
void Sample01::Reporter::printRegular(
   YaspEvent const & ev,
   int iDepthElt,
   int eidCurrent
)
{
   if (isShowLocation) {              // location info
      char pchBuf[11];
      sprintf(pchBuf, "L%4ld#%04ld", (long)ysp.inqLine(), (long)eidCurrent);
      out << setW(10) << pchBuf;
   }
   YaspType type = ev.inqType();
   if (type == YSP__TAG_START) {
      --iDepthElt;                    // already started!
   }
   leading.write(out.rdbuf(), YaspEvent::inqTypeName(type), iDepthElt);

   out << '\'';
   if (type == YSP__TEXT) {
      // Almost identical to what the Yasp3 built-in Stringiser does,
      // the following is rather an example showing how to use our own buffer
      if (isTruncateText) {
         out << setW(CDATALIMIT) << textBuffer
             << setW(iLenEllipsis) << pchEllipsis;
      }else {
         out << textBuffer;
      }
   }else {
      ev.accept(stringizer);
   }
   out << "\'\r\n";
   if (type == YSP__ERROR) {
      out << setW(iLenError) << pchError;
   }
   #if defined _DEBUG || (defined __IBMCPP__  && defined __DEBUG_ALLOC__)
       out << flush;  // flush the buffer now!
   #endif
}

/*===========================================================================*/
