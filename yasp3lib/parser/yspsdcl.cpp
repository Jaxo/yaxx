/*
* $Id: yspsdcl.cpp,v 1.2 2002-01-18 14:54:29 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include "../yasp3.h"

/*---------------------------------------------------------------parseSgmlDecl-+
| Parse the SGML declaration                                                   |
| Enter here after: "<!SGML"                                                   |
+-----------------------------------------------------------------------------*/
bool Yasp3::parseSgmlDecl(YaspEvent & ev)
{
    ev = YSP__DCL_END;
    UnicodeString ppinfo("PPINFO");  // the A is checked separately

    UCS_2 ucLowA = (UCS_2)'a';
    UCS_2 ucUppA = (UCS_2)'A';
    int iCur;

    do {                            // look for an 'A'
       while (iCur = oec.get(), (iCur != ucLowA) && (iCur != ucUppA)) {
          if (iCur == EOF_SIGNAL) return false;
       }
    }while (!skip(ppinfo, true));   // uppercased followed by "PPINFO"

    // Success! look for MDC.  No alternate syntax (or I would know it!)

    UCS_2 ucMDC = (UCS_2)'>';
    while (iCur = oec.peek(), iCur != ucMDC) {
       if (iCur == EOF_SIGNAL) return false;
       oec.skip();
    }
    dlmfnd.find(oec, iCur);
    return true;    // done.
}
/*============================================================================*/
