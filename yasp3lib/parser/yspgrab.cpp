/*
* $Id: yspgrab.cpp,v 1.8 2002-04-02 13:53:29 pgr Exp $
*
* Low-level routines to parse names, tokens, groups, etc.
*
* Internal routines to grab or skip a certain class of characters from the
* input stream.  These routines handle simple parsing, such as single
* tokens, text unbroken by markup, etc.
*
* All these routines recognize and dereference parameter entities,
* making it transparent to higher routines.
*
* See 'yspParse' for routines to parse larger SGML constructs, such as
* markup declarations for elements, entities, etc.
*/

/*---------+
| Includes |
+---------*/
#include "../yasp3.h"
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"

#define WORD_MAX_LENGTH 20

/*----------------------------------------------------------bypassDoctypeDecl-+
| Bypass a doctype declaration.                                               |
| Scream if goes over BYPASS_WARNING long, just for paranoia's sake.          |
|                                                                             |
| In MODE_BYP_DOCMD:                                                          |
|                                                                             |
|   MDC         >                                                             |
|   DSO         [                                                             |
|   MDO         <!                                                            |
|   PIO         <?                                                            |
|   STAGO       <                                                             |
|   MDO_COM     <!--                                                          |
|   MDO_DSO     <![                                                           |
|   MDO_MDC     <!>                                                           |
|   MSC_MDC     ]]>                                                           |
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::bypassDoctypeDecl()
{
   int const iLineStart = oec.inqLine();
   dlmfnd.pushMode(MODE_BYP_DOCMD);
   int iCount = 0;                // How far have we skipped?
   while (
      (Delimiter::IX_NOT_FOUND == dlmfnd.find(oec)) && (entMgr.skip())
   ) {
      ++iCount;
   }
   dlmfnd.popMode();
   if (iCount > BYPASS_WARNING) {
      //>>PGR: to fix!  iLineStart and End can be in diff entities!
      erh << ECE__ERROR << _YSP__BIGSKIP
          << iLineStart << oec.inqLine() << endm;
   }
}

/*-----------------------------------------------------------------bypassDecl-+
| Bypass a simple decl: just go to the next MDC                               |
| Scream if goes over BYPASS_WARNING long, just for paranoia's sake.          |
+----------------------------------------------------------------------------*/
void Yasp3::bypassDecl()
{
   if (dlmfnd.delimFound() != Delimiter::IX_MDC) {
      int const iLineStart = oec.inqLine();
      dlmfnd.pushMode(MODE_BYP_MD);
      int iCount = 0;                // How far have we skipped?
      while (
         (Delimiter::IX_NOT_FOUND == dlmfnd.find(oec)) && (entMgr.skip())
      ) {
         ++iCount;
      }
      dlmfnd.popMode();
      if (iCount > BYPASS_WARNING) {
         //>>PGR: to fix!  iLineStart and End can be in diff entities!
         erh << ECE__ERROR << _YSP__BIGSKIP
             << iLineStart << oec.inqLine() << endm;
      }
   }
}

/*----------------------------------------------------------------bypassGroup-+
| Bypass a group, counting balanced GRPO/GRPC                                 |
+----------------------------------------------------------------------------*/
void Yasp3::bypassGroup(int iNest)
{
   if (dlmfnd.delimFound() != Delimiter::IX_MDC) {
      int const iLineStart = oec.inqLine();
      dlmfnd.pushMode(MODE_BYP_GRP);
      int iCount;
      for (iCount = 0; ; ++iCount) {
         switch (dlmfnd.find(oec)) {
         case Delimiter::IX_NOT_FOUND:
            if (entMgr.skip()) continue;
            break;
         case Delimiter::IX_GRPO:
            ++iCount;
            ++iNest;
            continue;
         case Delimiter::IX_GRPC:
            if (--iNest >= 0) continue;
            break;
         default: // Delimiter::IX_MDC:
            break;
         }
         break;     // break for loop unless continue bypassed it
      }
      dlmfnd.popMode();
      if (iCount > BYPASS_WARNING) {
         //>>PGR: to fix!  iLineStart and End can be in diff entities!
         erh << ECE__ERROR << _YSP__BIGSKIP
             << iLineStart << oec.inqLine() << endm;
      }
   }
}

/*--------------------------------------------------------------skip_S_noFind-+
| Effects:                                                                    |
|   Skip over White Space                                                     |
|                                                                             |
| Returns: (NONE)                                                             |
+----------------------------------------------------------------------------*/
inline bool Yasp3::skip_S_noFind()
{
   bool isFound;
   if (sdcl.charset().isSeparator(oec.peek())) {
      isFound = true;
      do {
         oec.skip();
      }while (sdcl.charset().isSeparator(oec.peek()));
   }else {
      isFound = false;
   }
   return isFound;
}

/*---------------------------------------------------------------------skip_S-+
| Skip S separator and issue dlmfind                                          |
+----------------------------------------------------------------------------*/
bool Yasp3::skip_S()
{
   bool isFound = skip_S_noFind();
   dlmfnd.find(oec);
   return isFound;
}

/*--------------------------------------------------------------------skip_ts-+
| Skip token separator                                                        |
|                                                                             |
| [70] ts = s | Ee | parameter entity reference                               |
|  [5]  s = SPACE | RE | RS | SEPCHAR                                         |
|                                                                             |
| RETURN:                                                                     |
|    true if at least one ts was found                                        |
|    false if no ts were found                                                |
+----------------------------------------------------------------------------*/
bool Yasp3::skip_ts()
{
   bool isFound = false;

   for (;;) {
      if (skip_S()) isFound = true;
      switch (dlmfnd.delimFound()) {
//    case Delimiter::IX_PERO_GRPO:          // Specific Parameter Entity
//       break;
      case Delimiter::IX_PERO:               // Parameter Entity
         dereferEntity(false);
         isFound = true;
         continue;
      case Delimiter::IX_NOT_FOUND:
         // BACKUP NEEDED
         if ((oec.peek() == EOF_SIGNAL) && (isFound = true, entMgr.skip())) {
            continue;
         }
         // fall thru
      default:
         return isFound;
      }
   }
}

/*--------------------------------------------------------------------skip_ps-+
| Skip parameter separator                                                    |
|                                                                             |
| ASSUME: mode is MD_MODE                                                     |
|                                                                             |
| [65] ps = s | Ee | parameter entity reference | comment                     |
|                                                                             |
| IMPLEMENTATION NOTE:                                                        |
| [70] ts = s | Ee | parameter entity reference                               |
|     ==> ps = ts | comment                                                   |
|                                                                             |
| RETURN:                                                                     |
|    true if at least one ps was found                                        |
|    false if no ps were found                                                |
|                                                                             |
+----------------------------------------------------------------------------*/
bool Yasp3::skip_ps()
{
   bool isFound = false;

   for (;;) {
      if (skip_ts()) isFound = true;
      if (dlmfnd.delimFound() == Delimiter::IX_COM) {
         while (!skip(Delimiter::IX_COM)) {
            if (!entMgr.skip()) return isFound;   // EOF reached
         }
         isFound = true;
         continue;
      }
      return isFound;
   }
}

/*------------------------------------------------------------skip_psRequired-+
| Required by ISO 8879, when para 3 of 10.1.1 doesn't apply.                  |
+----------------------------------------------------------------------------*/
bool Yasp3::skip_psRequired()
{
   Delimiter::e_Ix prevDelim = dlmfnd.delimFound();

   if (
      !skip_ps() &&
      (prevDelim == Delimiter::IX_NOT_FOUND) &&
      (dlmfnd.delimFound() == Delimiter::IX_NOT_FOUND)
   ) {
      erh << ECE__ERROR << _YSP__NOPS << endm;
      return false;
   }
   return true;
}

/*--------------------------------------------------------------------skip_ds-+
| Skip declaration separator                                                  |
|                                                                             |
| [71] ds = s | Ee | parameter entity reference | comment declaration         |
|         | processing instruction | marked section declaration               |
|                                                                             |
| IMPLEMENTATION NOTE:                                                        |
|    Stop when a delim reportable by handleDelim is found.                    |
|                                                                             |
| Return:                                                                     |
|    true if a valid delimiter (processable by handleDelim) is found          |
|    false if troubles                                                        |
|                                                                             |
+----------------------------------------------------------------------------*/
bool Yasp3::skip_ds()
{
   for (;;) {
      skip_S();
      switch (dlmfnd.delimFound()) {

//    case Delimiter::IX_PERO_GRPO:      // Specific Parameter Entity
      case Delimiter::IX_PERO:           // Parameter Entity
         {
            int iToSkip;
            Entity const * pEnt = parseEntityReference(false, iToSkip);
            if (!pEnt || (pEnt->inqKind() == ENTK_INVALID)) {
               // ENTK_INVALID: entity created because it didn't exist
               oec.skip(iToSkip);
               continue;
            }else if (pEnt->isRcdata() && (options!=YSP__reportEntityEvents)) {
               // reparseable (not a PI) and user doesn't want to see it...
               oec.skip(iToSkip);
               open(pEnt, true);          // issue an error message if bad
               continue;                  // entity was dereferenced.  opened?
            }else {
               break;
            }
         }
         break;                          // valid entity not to be dereferenced
      case Delimiter::IX_NOT_FOUND:
         // BACKUP NEEDED
         if (oec.peek() != EOF_SIGNAL) {
            return false;
         }
         if (resumePreviousEntity()) {
            continue;
         }
         break;
      default:                           // MDO_COM, MDO_MDC, MDO_DSO, MSC_MDC
         break;
      }
      break;
   }
   pendingDelim = dlmfnd.delimFound();
   return true;
}

/*-----------------------------------------------------------------------peek-+
| Effects:                                                                    |
|   Peek to find a given UnicodeString                                        |
| Args:                                                                       |
|   ucs:        string to look for                                            |
|   isNameCase: true - case is irrelevant                                     |
|               false - exact compare                                         |
|   iOffest     when to peek the first character                              |
| Returns:                                                                    |
|   true is the string was found, false otherwise.                            |
|   If true, iOffset is the new offset of the next unparsed character         |
+----------------------------------------------------------------------------*/
bool Yasp3::peek(
   UnicodeString const & ucs, bool isNameCase, int & iOffset
) {
   int const iNewOffset = iOffset + ucs.length();
   UCS_2 const * pUcSource = ucs;
   if (isNameCase) {
      for (int i=iOffset; i < iNewOffset; ++i) {   // upper EOF is 0!
         if (*pUcSource++ != sdcl.charset().uppercase(oec.peek(i))) {
            return false;
         }
      }
   }else {
      for (int i=iOffset; i < iNewOffset; ++i) {
         if (*pUcSource++ != oec.peek(i)) {
            return false;
         }
      }
   }
   iOffset = iNewOffset;
   return true;
}

/*-----------------------------------------------------------------------skip-+
| Effects:                                                                    |
|   Skip over a given UnicodeString                                           |
| Args:                                                                       |
|   ucs:        string to look for                                            |
|   isNameCase: true - case is irrelevant                                     |
|               false - exact compare                                         |
| Returns:                                                                    |
|   true if the string matches.                                               |
|   false if not such string exists.  The stream is left unchanged.           |
+----------------------------------------------------------------------------*/
bool Yasp3::skip(UnicodeString const & ucs, bool isNameCase)
{
   int iOffset = 0;
   if (peek(ucs, isNameCase, iOffset)) {
      oec.skip(iOffset);
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------------Yasp3::GrabGroupCB::~GrabGroupCB-+
| grabGroup call back (virtual destructor)                                    |
+----------------------------------------------------------------------------*/
Yasp3::GrabGroupCB::~GrabGroupCB() {
}

/*------------------------------------------------------------------grabGroup-+
| Effects:                                                                    |
|   Grabs a delimited group, including the GRPO and GRPC delimiters.          |
|                                                                             |
| Requires:                                                                   |
|   Expects the stream to be pointing right at the start of the GRPO.         |
|   Push and pop GRP mode here, not in caller.                                |
|                                                                             |
| NOTES:                                                                      |
|   Unrecognized delimiter or eof terminates the scan.                        |
|   The scan continues to the balancing GRPC even on buffer overflow.         |
|                                                                             |
|   The call back function is called when a new token or name is ready to be  |
|   parsed.  The call back function is expected to grab the name and return   |
|   true if it's OK.                                                          |
|                                                                             |
| Returns:                                                                    |
|   First character *beyond* the group-close delimiter.                       |
+----------------------------------------------------------------------------*/
void Yasp3::grabGroup(GrabGroupCB & cb)
{
   int iNest = 0;                  // () depth

   dlmfnd.pushMode(MODE_GRP);
   for (;;) {
      skip_ts();                          // opt ts after GRPO
      if (dlmfnd.delimFound() == Delimiter::IX_NOT_FOUND) {
         if (!cb.call(this)) {            // have the caller grab the name
            bypassGroup(iNest);           // caller said it was wrong...
            break;                        // return in error
         }
      }else {                             // expecting a name, *not* a delim!
         erh << ECE__ERROR << _YSP__INVNAME << endm;
      }
      skip_ts();                          // skip opt ts after name
      switch (dlmfnd.delimFound()) {
      case Delimiter::IX_AND:
      case Delimiter::IX_SEQ:
      case Delimiter::IX_OR:
         continue;                        // more to do
      case Delimiter::IX_GRPO:            // allow nesting (not said in ISO)
         ++iNest;
         continue;
      case Delimiter::IX_GRPC:
         if (--iNest >= 0) continue;      // more to do
         break;                           // Success!
      default:
         oec.backup(dlmfnd.delimLength()); // not for us
         erh << ECE__ERROR << _YSP__INVGROUP << endm;
         bypassGroup(iNest);
         break;                           // in error
      }
      break;                              // if no continue bypasses it...
   }
   dlmfnd.popMode();
}

/*-------------------------------------------------------------------peekName-+
| Effects:                                                                    |
|    Get a name                                                               |
| Returns:                                                                    |
|    The length of the name                                                   |
| Assumptions:                                                                |
|  - buf is a real buffer of ((1+NAMELEN) * sizeof(UCS_2))                    |
+----------------------------------------------------------------------------*/
int Yasp3::peekName(UCS_2 * pUcBuf, bool isNamecase)
{
   int const iMax = sdcl.quantity()[Quantity::IX_NAMELEN];
   int iLen = -1;
   UCS_2 uc;
   while (uc = oec.peek(++iLen), sdcl.charset().isName(uc)) {
      if (iLen >= iMax) {
         erh << ECE__ERROR << _YSP__BIGNAME << iLen << endm;
         break;
      }
      if (isNamecase) {
         *pUcBuf++ = sdcl.charset().uppercase(uc);
      }else {
         *pUcBuf++ = uc;
      }
   }
   *pUcBuf = 0;
   return iLen;
}

/*-------------------------------------------------------------------peekRefc-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Yasp3::peekRefc(int iOffset)
{
   UCS_2 const * pUcRefc = value(Delimiter::IX_REFC);
   int iNewOffset = iOffset;
   while (*pUcRefc) {
      if (*pUcRefc++ != oec.peek(iNewOffset++)) {
         int c = oec.peek(iOffset);
         if (
            sdcl.charset().isREchar(c) ||
            sdcl.charset().isRSchar(c)       // for unix
         ) {
           return iOffset+1;                 // suppress RE (9.4.5)
         }else {
           return iOffset;
         }
      }
   }
   return iNewOffset;
}

/*------------------------------------------------------------grabNameGeneral-+
| Get a general name.                                                         |
+----------------------------------------------------------------------------*/
int Yasp3::grabNameGeneral(UCS_2 * buf)
{
   int iLen = peekName(buf, sdcl.charset().isCaseGeneral());

   if (!iLen || !sdcl.charset().isNamestart(*buf)) {
      erh << ECE__ERROR << _YSP__INVNAME << endm;
   }
   oec.skip(iLen);
   return iLen;
}

/*-------------------------------------------------------------grabNameEntity-+
| Get an entity name.                                                         |
+----------------------------------------------------------------------------*/
int Yasp3::grabNameEntity(UCS_2 * buf)
{
   int iLen = peekName(buf, sdcl.charset().isCaseEntity());

   if (!iLen || !sdcl.charset().isNamestart(*buf)) {
      erh << ECE__ERROR << _YSP__INVNAME << endm;
   }
   oec.skip(iLen);
   return iLen;
}

/*--------------------------------------------------------------grabNameToken-+
| Get a token                                                                 |
+----------------------------------------------------------------------------*/
int Yasp3::grabNameToken(UCS_2 * buf)
{
   int iLen = peekName(buf, sdcl.charset().isCaseGeneral());

   if (!iLen || !sdcl.charset().isName(*buf)) {
      erh << ECE__ERROR << _YSP__INVTOKEN << endm;
   }
   oec.skip(iLen);
   return iLen;
}

/*-----------------------------------------------------------------grabChoice-+
| Find a keyword in a list of choices, return its associated value            |
+----------------------------------------------------------------------------*/
int Yasp3::grabChoice(
    ReservedName::Choice const * aChoices,
    bool isReportErrors
) {
   UCS_2 name[1+NAMELEN_MAX];
   int iLen = peekName(name, sdcl.charset().isCaseGeneral());
   int iValue;
   ReservedName::ChoiceList list(sdcl.reservedName(), aChoices);
   if (list.find(name, iValue)) {
      oec.skip(iLen);
   }else {
      if (isReportErrors) {
         erh << ECE__ERROR << _YSP__INVKEYWD << list.stringize(", ") << endm;
         oec.skip(iLen);
      }
   }
   return iValue;
}


/*--------------------------------------------------------getParameterLiteral-+
| Get a parameter literal into undelimited UnicodeString.                     |
|                                                                             |
| The mode is LITP_MODE, so CRO_TK, LIT_TK, LITA_TK, PERO_TK, PERO_GRPO_TK    |
| are recognized.  Since this function is only called while parsing the       |
| prolog, PERO_GRPO is an error (see 9.4.4).                                  |
|                                                                             |
| Recognized in MODE_LITP:  CRO, LIT, LITA, PERO, PERO_GRPO                   |
|                                                                             |
| Implementation Note:                                                        |
|   this routine is very close from grabValueSpec in ysptag                   |
|   but no enough close so we can merge both into a unique routine            |
|   (at least not now, 03/08/96 - PGR)                                        |
|   If bugs are found, you might want to examine the sister routine!          |
+----------------------------------------------------------------------------*/
UnicodeString Yasp3::getParameterLiteral(
   Delimiter::e_Ix & dlmStartRet   // (rtn) what delimiter started the literal
) {
   Delimiter::e_Ix dlmStart = dlmfnd.delimFound();
   switch (dlmStart) {
   case Delimiter::IX_LIT:
   case Delimiter::IX_LITA:
      break;
   default:
      erh << ECE__ERROR << _YSP__NOLIT << endm;
      return UnicodeString::Nil;
   }

   UnicodeBuffer buf;
   int const iLineStart = oec.inqLine();
   int const iDepthStart = oec.iDepth;
   bool isReported = false;             // if true: stop yelling at errors
   bool isMssNow = false;
   bool isMssBefore = false;
   dlmfnd.pushMode(MODE_LITP);

   /*-------------------------------------+
   | Loop until the starting delimiter is |
   | found at the same nesting level      |
   +-------------------------------------*/
   for (;;) {
      int iCur = oec.peek();
      if (iCur == EOF_SIGNAL) {
         // BACKUP NEEDED
         dlmfnd.setMode(MODE_LITP);        // might have been disabled!
         if (oec.iDepth > iDepthStart) {
            entMgr.skip();                 // Ignore Ee (7.9.3)
            continue;                      // pursue
         }
         erh << ECE__ERROR << _YSP__UBLNLIT << iLineStart << endm;
         break;                            // exit: unbalanced LIT's
      }
      Delimiter::e_Ix dlmFound;
      if (isMssBefore = isMssNow, isMssBefore == true) {
         isMssNow = false;
         dlmFound = Delimiter::IX_NOT_FOUND;
      }else {
         dlmFound = dlmfnd.find(oec, iCur);
         if ((dlmFound == dlmStart) && (oec.iDepth == iDepthStart)) {
            break;                         // exit: all is ok
         }
      }
      switch (dlmFound) {

      case Delimiter::IX_CRO:
         {
            Charset::e_CharType ct;
            int iLen;
            iCur = dereferRegCharRef(ct, iLen);
            oec.skip(iLen);
         }
         break;                  //>>PGR: examine the char type!

      case Delimiter::IX_HCRO:
         {
            int iLen;
            iCur = tryDereferHexCharRef(iLen, options == YSP__keepUcCharRef);
            if (!iLen) {                  // means: do not dereference!
               oec.backup(dlmfnd.delimLength());
               iCur =  oec.peek();
               oec.skip();
            }else {
               oec.skip(iLen);
            }
         }
         break;                  //>>PGR: examine the char type!


      case Delimiter::IX_NOT_FOUND:
         oec.skip();
         if (sdcl.charset().isMsChar(iCur)) {
            /*
            | MSO/MSI/MSS CHAR recognition
            */
            if (sdcl.charset().isMsoChar(iCur)) {    // 9.7:
               if (!isMssBefore) dlmfnd.setMode(MODE_VOID);
            }else if (sdcl.charset().isMsiChar(iCur)) {
               dlmfnd.setMode(MODE_LITP);
            }else if (sdcl.charset().isMssChar(iCur)) {
               isMssNow = true;
            }
         }
         break;

      case Delimiter::IX_PERO:             // LITP_MODE only
         dereferEntity(false);
         continue;

      default:                             // PERO_GRPO
         if (!isReported) erh << ECE__ERROR << _YSP__INVDLM << endm;
         /* fall thru */

      case Delimiter::IX_LIT:
      case Delimiter::IX_LITA:
         if (dlmfnd.delimLength() > 1) {
            oec.backup(dlmfnd.delimLength()-1);
         }
         break;
      }

      /*------------------------+
      | Enter the new character |
      +------------------------*/
      buf << iCur;
   }
   /*---------------------+
   | End of literal found |
   +---------------------*/
   dlmfnd.popMode();
   dlmStartRet = dlmStart;

   assert (buf);
   if (buf.length() > sdcl.quantity()[Quantity::IX_LITLEN]) {
      //>>PGR: to fix!  iLineStart and End can be in diff entities!
      erh << ECE__ERROR << _YSP__BIGPLIT
          << iLineStart << oec.inqLine()
          << sdcl.quantity()[Quantity::IX_LITLEN] << endm;
   }
   return buf.getResult();
}

/*----------------------------------------------------------getMinimumLiteral-+
| Get minimum literal into undelimited UnicodeString.                         |
+----------------------------------------------------------------------------*/
UnicodeString Yasp3::getMinimumLiteral()
{
   UnicodeString ucsDlm;
   int const iMax = sdcl.quantity()[Quantity::IX_LITLEN];
   switch (dlmfnd.delimFound()) {
   case Delimiter::IX_LIT:
      ucsDlm = value(Delimiter::IX_LIT);
      break;
   case Delimiter::IX_LITA:
      ucsDlm = value(Delimiter::IX_LITA);
      break;
   default:
      erh << ECE__ERROR << _YSP__NOLIT << endm;
      return UnicodeString::Nil;
   }

   /*--------------------------------------------------+
   | Loop until:                                       |
   | - the ending delimiter is found;                  |
   | - an Ee is found - in this case, an appropriate   |
   |    message is issued, and the loop breaks         |
   |    as if the ending delimiter was found           |                           |
   +--------------------------------------------------*/
   int const iLineStart = oec.inqLine();
   UnicodeBuffer buf;
   bool isSpaceWaiting = false;

   for (;;) {                                   // main loop
      int iCur = oec.peek();                    // current character read
      if (iCur == EOF_SIGNAL) {                 // Ee found:
         // BACKUP NEEDED
         erh << ECE__ERROR << _YSP__UBLNLIT << ucsDlm << iLineStart << endm;
         break;                                 // leave
      }
      if (oec.skip(ucsDlm)) {                   // ending delim?
         break;                                 // Yes! leave
      }
      // For minimum literal, compress separators (10.1.7)
      // Note: sepchars are not truly min lit data (catched earlier)
      if (sdcl.charset().isSeparator(iCur)) {   // RE, RS, SPACE, Sepchar
         if (!sdcl.charset().isRSchar(iCur) &&  // if not RS,
            (buf.length())                      // nor leading sepchars
         ) {                                    // just: RE, TAB, SPACE,..
            isSpaceWaiting = true;              // reduce to 1 space
         }
         oec.skip();
         continue;
      }
      if (isSpaceWaiting) {
         isSpaceWaiting = false;
         buf << sdcl.charset().inqUcSpace();
      }
      buf << iCur;                              // store the character
      oec.skip();
      if (sdcl.isSGML() &&
         (options == YSP__validate) &&
         (!sdcl.charset().isMinData(iCur))      // if not real data...
      ) {
         erh << ECE__ERROR << _YSP__INVMLCHAR << endm;
      }
   }
   assert (buf);
   if (buf.length() > iMax) {
      erh << ECE__ERROR << _YSP__BIGMLIT << iLineStart << oec.inqLine()
          << sdcl.quantity()[Quantity::IX_LITLEN] << endm;
   }
   return buf.getResult();
}

/*--------------------------------------------------------------getInternalPi-+
| Get internal PI into undelimited UnicodeString.                             |
+----------------------------------------------------------------------------*/
UnicodeString Yasp3::getInternalPi()
{
   UnicodeString const & ucsDlm = value(Delimiter::IX_PIC);
   int iMax = sdcl.quantity()[Quantity::IX_LITLEN];
   int const iLineStart = oec.inqLine();

   /*--------------------------------------------------+
   | Loop until:                                       |
   | - the ending delimiter is found;                  |
   | - an Ee is found - in this case, an appropriate   |
   |    message is issued, and the loop breaks         |
   |    as if the ending delimiter was found           |                           |
   +--------------------------------------------------*/
   UnicodeBuffer buf;

   for (;;) {                           // main loop
      int iCur = oec.peek();            // current character read
      if (iCur == EOF_SIGNAL) {         // Ee found:
         // BACKUP NEEDED
         erh << ECE__ERROR << _YSP__UBLNPI << ucsDlm << iLineStart << endm;
         break;                         // leave
      }
      if (oec.skip(ucsDlm)) {           // try skipping delimiter
         break;                         // OK! leave.
      }
      buf << iCur;                      // store the character
      oec.skip();
   }
   assert (buf);
   if (buf.length() > iMax) {
      erh << ECE__ERROR << _YSP__BIGPI << iLineStart << oec.inqLine()
          << sdcl.quantity()[Quantity::IX_LITLEN] << endm;
   }
   return buf.getResult();
}

/*----------------------------------------------------------grabDelimitedText-+
| Grab input data that is simply terminated with a single delimiter.          |
|                                                                             |
| No markup will be recognized, other than the delimiter that terminates it.  |
| Hence this function simply reads until the ending delimiter is reached.     |
+----------------------------------------------------------------------------*/
int Yasp3::grabDelimitedText(
   UCS_2 * buf,             // Buffer
   int max,                     // Buffer size limit
   Delimiter::e_Ix ixDlm        // ending delimiter
) {
   assert  ((max > 1) && (buf));
   int iLen = 0;                // How full is the buffer?
   UCS_2 * pUcCur = buf;    // -> last char put in buffer
   UnicodeString const & ucsDlm = value(ixDlm);

   if (!iLineDelimitedTextStart) {
      iLineDelimitedTextStart = oec.inqLine();
   }
   --max;                       // reserve space for ending 0;
   for (;;) {
      /*
      | Loop until:
      | - the ending delimiter is found;
      | - an Ee is found - in this case, an appropriate
      |    message is issued, and the loop breaks
      |    as if the ending delimiter was found
      */
      int iCur = oec.peek();                    // current character read
      if (iCur == EOF_SIGNAL) {                 // Ee found:
         // BACKUP NEEDED
         // cannot cross an entity boundary!
         // however, if we want to forgive users sins...
         /*
         | if (
         |    (options != YSP__synchroElmWithEnt) &&
         |    resumePreviousEntity()
         | ) {
         |     continue;
         | }
         */
         switch (ixDlm) {                     // issue appropriate message
         case Delimiter::IX_PIC:
            erh << _YSP__UBLNPI;
            break;
         case Delimiter::IX_COM:
            dlmfnd.popMode();                 // COMMENT -> MD_COM
            ixDlm = Delimiter::IX_COM_MDC;    // simulate the end
            /* fall thru */
         case Delimiter::IX_COM_MDC:
            erh << _YSP__UBLNCOM;
            break;
         default:
            assert  (false);
            break;
         }
         erh << ECE__ERROR << value(ixDlm) << iLineDelimitedTextStart << endm;
         iLineDelimitedTextStart = 0;
         pendingDelim = ixDlm;
         break;                                 // leave: end forced.
      }
      if (oec.skip(ucsDlm)) {                   // try skipping delimiter
         iLineDelimitedTextStart = 0;           // worked!
         pendingDelim = ixDlm;
         break;                                 // leave: success.
      }
      //>>> PGR: MSO, MSI, MSS!!!
      *pUcCur++ = iCur;
      oec.skip();
      if (                   // quick hack to usually break at word bounds.
         (++iLen > (max-WORD_MAX_LENGTH)) &&
         ((sdcl.charset().isSeparator(iCur)) || (iLen >= max))
      ) {
         break;
      }
   }
   *pUcCur = 0;
   return iLen;
}

/*----------------------------------------------------------skipMarkedSection-+
| Process ignored marked section.                                             |
| We don't do any grab here -- but we might.                                  |
| This block is reached in MODE_MS_IGNORE, just after we processed MS keyword.|
| When a marked section is being ignored, it's faster to skip on the          |
| fly all nested marked sections while matching MDSO/MSDCs                    |
+----------------------------------------------------------------------------*/
void Yasp3::skipMarkedSection()
{
   int iIgnoreDepth = 1;
   int const iLineStart = oec.inqLine();
   for (;;) {
      switch (dlmfnd.find(oec)) {
      case Delimiter::IX_MDO_DSO:
         ++iIgnoreDepth;
         continue;
      case Delimiter::IX_MSC_MDC:
         if (--iIgnoreDepth > 0) {
            continue;
         }
         break;
      default:     // IX_NOT_FOUND
         if (entMgr.skip()) {
            continue;
         }
         erh << ECE__ERROR << _YSP__UBLNMS << iLineStart << endm;
         dlmfnd.popMsMode();
         break;
      }
      break;
   }
   pendingDelim = dlmfnd.delimFound();
}

/*-------------------------------------------------------------------grabText-+
| Effects:                                                                    |
|     Grab the maximum of text in CON mode.                                   |
|     When returning:                                                         |
|     - pendingDelim is set to the next delimiter -- if any,                  |
|       for example STAGO, or ERO  on a reportable entity;                    |
|     - the stack of pendingEvent may have been feeded if entering text       |
|       requires a structure change                                           |
|                                                                             |
| Returns:                                                                    |
|    The length of the text that has grabbed.                                 |
|                                                                             |
| Calling sequence:                                                           |
|    Called early in almost every Yasp3::next().                              |
|    - If text cannot be immediately entered because of a structure change,   |
|      return: next() uses the stack of pending events to report the change   |
|      story.                                                                 |
|    - When a delimiter-in-context is found, save it in pendingDelim and      |
|      return:  next() reports an event for any data found before the         |
|      delimiter, and, back in next(), pendingDelim triggers handleDelim()    |
|    - If none of this, stop when the buffer is full, or when a separator     |
|      is found in the last WORD_MAX_LENGTH of the buffer -- so we gently     |
|      break at word bounds.                                                  |
|                                                                             |
| When recognizing text, apply rules at 7.6.3.  The RecordBinder is used      |
| to detect a relevant RE while storing the current state (state graph.)      |
|                                                                             |
| Remember the OS differences:                                                |
|    UNIX uses line-feed (d10),                                               |
|    MAC uses carriage-return (d13),                                          |
|    DOS uses both (d13, d10),                                                |
|    IBM mainframes use length prefixes.                                      |
|                                                                             |
| SGML's RS and RE normally map to LF and CR: this doesn't fit Unix.          |
| The rules say RS is always ignored, but if we did this, UNIX files          |
| would be trashed, since words would join up across records.                 |
|                                                                             |
| Note: this algorithm is one of the most important for the whole parser.     |
|       it has been heavily tested, commented, optimized.                     |
|       It is not an easy one to fix: carefull!                               |
+----------------------------------------------------------------------------*/
int Yasp3::grabText(
   UCS_2 * buf,                        // Buffer
   int max                             // Buffer size limit
) {
   bool isDataAllowed = false;
   pendingDelim = Delimiter::IX_NOT_FOUND;  // assume so.
   UCS_2 * pUcCur = buf;               // -> last char put in buffer
   int iRemain = max-1;                // Reserve space for ending zero
   bool isDiscardableData = false;     // Discard white content of undef elmt
   RecordBinder rcdbndBackup;

   for (;;) {
      int iCur = oec.peek();           // Character being processed
      Charset::e_CharType typeCur;     // what is its type?
      int iToSkip;                     // what is its length?

      switch (dlmfnd.find(oec, iCur)) {
      case Delimiter::IX_NOT_FOUND:    // vanilia character
         if (iCur == EOF_SIGNAL) {
            // BACKUP NEEDED
            if (evStartEntity.inqType() != YSP__noType) {
               assert  (pUcCur == buf);
               registerEntityStart();  // start still pending
               return 0;
            }
            if (!resumePreviousEntity()) goto done;
            continue;
         }
         typeCur = sdcl.charset().charType(iCur);
         iToSkip = 1;
         break;

      case Delimiter::IX_ERO:          // general entity
/*       -- I should implement this:
|        if (isDiscardableData) {      -- means: leading spaces in undef elm
|           isDiscardableData = false;
|           #ifdef DEBUG
|              assert  (false == startPcData()); // no events!
|           #else
|              startPcData();
|           #endif
|        }
*/
         if (evStartEntity.inqType() != YSP__noType) { // start still pending
            oec.backup(dlmfnd.delimLength());          // backup!
            assert  (pUcCur == buf);                   // no data were entered
            registerEntityStart();
            return 0;                  // no data entered so far!
         }
         {
            Entity const * pEnt = parseEntityReference(true, iToSkip);
            if (!pEnt || (pEnt->inqKind() == ENTK_INVALID)) {
               /*
               | Entity is invalid: keep it as if it was text.
               | This also will allow things like AT&T to pass thru.
               */
               oec.backup(dlmfnd.delimLength());
               iCur = oec.peek();
               typeCur = Charset::TYPE_REGULAR;
               iToSkip = 1;
               break;

            }else if (!pEnt->isRcdata()) { // NDATA, SDATA, CDATA, SUBDOC
               /*
               | Non-reparseable data entities always break this loop
               | (and are then reported.)
               | handleEntity() will be called by next() (thru pendingDelim.)
               | If the entity is expandable (C/S data only!),
               | handleEntity() issues START and open the entity.
               | Then grabText() will be called again for this entity.
               | If non-expandable, handleEntity() will generate REF.
               */
               pendingDelim = Delimiter::IX_ERO;  // keep pending delimiter
               if (!isDataAllowed && !elmMgr.isDataAllowed()) {
                  startPcData();           // might stack events: report first!
                  return 0;                // DONE.  No text entered so far
               }
               goto done;

            }else if (options == YSP__reportEntityEvents) {
               /*
               | Reparseable character data entities and reporting ON: break,
               | so to report.  Keep pendingDelim so "handleEntity()"
               | will later be called.
               | Assume for the moment that an ENTITY_START will be reported.
               | If not expandable, then handleDelim will turn it to a REF.
               */
               evStartEntity = YaspEvent(pEnt, YSP__ENTITY_START);
               pendingDelim = Delimiter::IX_ERO;  // keep pending delimiter
               goto done;                  // DONE.
            }else {
               /*
               | Reparseable character data entities and reporting OFF:
               | dereference in-line.
               */
               oec.skip(iToSkip);
               open(pEnt, true);           // issue an error message if bad
               continue;                   // entity was dereferenced.  opened?
            }
         }
         break;

      case Delimiter::IX_CRO:              // character ref entity
         iCur = dereferRegCharRef(typeCur, iToSkip);
         /*
         | CRO constraint (&#, followed by a name char, or digit)
         | makes iToSkip should never be zero,
         */
         if (typeCur == Charset::TYPE_NON_SGML) {
            /*
            | CRO is invalid: keep it as if it was text.
            | This also will allow things like &#SPOOKY to pass thru.
            */
            oec.backup(dlmfnd.delimLength());
            iCur = oec.peek();
            typeCur = Charset::TYPE_REGULAR;
            iToSkip = 1;
         }
         break;

      case Delimiter::IX_HCRO:         // hexa character ref entity
         typeCur = Charset::TYPE_REGULAR;
         iCur = tryDereferHexCharRef(
            iToSkip, options == YSP__keepUcCharRef  //@014
         );
         /*
         | HCRO constraint (&#x, followed by an hexadecimal digit)
         | makes iToSkip should never be zero,
         | except that tryDereferHexCharRef() forces a zero if:
         |   - &#xBEEF is found; and,
         |   - isKeepUcCharRef is turned on; and,
         |   - the entity "xBEEF" exists.
         | In this case we break, but the entity reference now
         | should looks &xBEEF, so we handle it properly when
         | we come back next time.
         */
         if (!iToSkip) {               // entity should not be dereferenced
            oec.backup(1);             // make &#xyz be seen as &xyz: get 'x',
            pendingDelim = Delimiter::IX_ERO;  // and pretend ERO was there.
            if (!isDataAllowed && !elmMgr.isDataAllowed()) {
               startPcData();          // might stack events: report first!
               return 0;               // DONE.  No text entered so far
            }
            goto done;                 // DONE.
         }
         break;

      case Delimiter::IX_STAGO:        // start tag: may generate events!
         /*
         | let handleStart register the EntityStart after
         | the reporting of all inplied events, if any
         */
         pendingDelim = Delimiter::IX_STAGO;       // keep pending delimiter
         goto done;                    // DONE.

      default:                         // any other delimiter
         if (evStartEntity.inqType() != YSP__noType) { // start still pending
            assert  (pUcCur == buf);   // no data were entered
            registerEntityStart();
         }
         pendingDelim = dlmfnd.delimFound();       // keep pending delimiter
         goto done;                    // DONE.
      }

      /*--------------------------------------------------------------------+
      | Current delimiter is either NOT_FOUND or CRO, and the current char  |
      | is data or a separator.  isDataAllowed is false if the element was  |
      | never checked against entering #PCDATA -- just after the start-tag. |
      +--------------------------------------------------------------------*/
      assert  (isDataAllowed || (pUcCur == buf));  // no data were entered
      if (!isDataAllowed && elmMgr.isDataAllowed()) {
         if (evStartEntity.inqType() != YSP__noType) {
            registerEntityStart();
            return 0;
         }
         isDataAllowed = true;
      }
      if (!isDataAllowed) {
         ModelTree const & mdl = elmMgr.inqElementLastOpened()->inqModel();
         if (
            (sdcl.charset().isSeparator(iCur)) &&
            (typeCur != Charset::TYPE_REGULAR) &&   // numeric CRO!
            (options != YSP__keepLineBreak)
         ) {
            if (evStartEntity.inqType() != YSP__noType) {
               registerEntityStart();
               return 0;
            }
            if (!mdl.isDefined()) {   // undefined element!
               /*
               | Special case when the model is not defined:
               | Consider it as being element content if the final
               | buffer is entirely blank.  Since we don't know this now,
               | we act as if the undefined element is not element-content,
               | and enter all data in the buffer.
               | Precautions:
               | - Carefully backup the state of the RecordBinder.
               |   In case we discover (at the end) a blank buffer,
               |   restore the RecordBinder state: new state is irrelevant.
               | - defer the call "startPcData()" to avoid to turn on
               |   elmMgr.isDataAllowed().  We know that PcData is allowed.
               | Caveats:
               | As any recovery, this one has limitations:
               | - the spaces at the beginning of X in:
               |   "<X>   <Y>Hello</Y> World</X> are suppressed.
               | - blank lines bigger than the buffer size will be
               |   discarded.
               | In XML, these blanks should be passed to the Application
               | as regular data.
               */
               //>>> For XML: if (options != YSP__XML) {}
               rcdbndBackup = rcdbnd;  // we don't know if RE/RS are true ones.
               isDiscardableData = true;

            }else if (mdl.isElementContent()) {
               /*
               | If the model is pure element content (no data) and if a true
               | separator has been found (not made by a numeric char ref)
               | skip as many separators as you can, and loop.
               | In XML, we should signal to the Application these are
               | white space in element content.
               */
               oec.skip(iToSkip);
               while (sdcl.charset().isSeparator(oec.peek())) oec.skip();
               continue;
            }
         }
         /*
         | In mixed content (7.6) even an RE is recognized as DATA before
         | to consider if it can be ignored.  Try starting #PCDATA.
         */
         if (!isDiscardableData) {
            bool isEventsGenerated = startPcData();
            if (evStartEntity.inqType() != YSP__noType) {
               registerEntityStart();
               isEventsGenerated = true;
            }
            if (isEventsGenerated) {
               if (dlmfnd.delimFound() ==  Delimiter::IX_CRO) {
                  oec.backup(dlmfnd.delimLength());
               }
               return 0;                  // DONE.  No text entered so far
            }
         }
         isDataAllowed = true;
      }

      /*------------------------------------------------------------------+
      | Apply the RE/RS rule at 7.6.1: if a relevant RE is generated,     |
      | because of data, it must be inserted before the current char.     |
      | While we know the type of character: try to break at word bound!  |
      +------------------------------------------------------------------*/
      switch (typeCur) {
      case Charset::TYPE_SEPA_RS:
         oec.skip(iToSkip);            // skip over
         if (rcdbnd.tellRS()) {
            if (options == YSP__killRERS) {
               *pUcCur++ = sdcl.charset().inqUcSpace();
            }else {
               *pUcCur++ = iCur;
            }
            if (--iRemain < WORD_MAX_LENGTH) goto done;
         }
         continue;
      case Charset::TYPE_SEPA_RE:
         oec.skip(iToSkip);            // skip over
         if (rcdbnd.tellRE()) {
            if (options == YSP__killRERS) {
               *pUcCur++ = sdcl.charset().inqUcSpace();
            }else {
               *pUcCur++ = iCur;
            }
            if (--iRemain < WORD_MAX_LENGTH) goto done;
         }
         continue;
      case Charset::TYPE_SEPA_TAB:
         if (options == YSP__killTAB) {
            iCur = sdcl.charset().inqUcSpace();
         }
         /* fall thru */
      case Charset::TYPE_SEPA_SPACE:
      case Charset::TYPE_SEPA_OTHER:
         if (iRemain <= WORD_MAX_LENGTH) iRemain = 1;  // hence: we will stop
         break;
      default:                         // true DATA
         // Charset::TYPE_MSI:         >>>PGR: I should do this
         // Charset::TYPE_MSO:
         // Charset::TYPE_MSS:
         // Charset::TYPE_FUNCTION:
         // Charset::TYPE_NON_SGML:
         // Charset::TYPE_REGULAR:
         if (isDiscardableData) {
            isDiscardableData = false;
            bool isEvent = startPcData();
            assert (false == isEvent); // no events!
         }
         break;
      }
      if (rcdbnd.tellData()) {         // relevant RE should be inserted first!
         if (options == YSP__killRERS) {
            *pUcCur++ = sdcl.charset().inqUcSpace();
         }else {
            *pUcCur++ = sdcl.charset().RS(); // line seps are RS (FIX?)
         }
         if (--iRemain < WORD_MAX_LENGTH) {
            if (dlmfnd.delimFound() ==  Delimiter::IX_CRO) {
               oec.backup(dlmfnd.delimLength());
            }
            goto done;                 // DONE.  Keep unbreakable data waiting
         }
      }
      oec.skip(iToSkip);               // skip examined chunk
      *pUcCur = iCur;                  // enter new char

      /* OPTIMIZATION
      | w/o changing the behaviour, next for loop can be changed for:
      |   ++pUcCur;
      |   if (--iRemain <= 0) goto done;
      | But:
      | - data is allowed,
      | - the record binder state is up to date
      | lets jump on the occasion to put as many characters as we can:
      */
      for (;;) {
         *++pUcCur = iCur = oec.peek();  // assume it's ok (set, and test)
         if (--iRemain <= 0) goto done;
         if (!dlmfnd.isDelimStart(iCur)) {
            if (sdcl.charset().isRegular(iCur)) {
               if (isDiscardableData) break;
               oec.skip();
               continue;               // it was OK.
            }
            if (sdcl.charset().isSPACEchar(iCur)) {
               oec.skip();
               if (iRemain <= WORD_MAX_LENGTH) {
                  // --iRemain;        // for correctness, but it's no use
                  ++pUcCur;
                  goto done;
               }
               continue;
            }
         }
         break;
      }
   }
done:
   if (isDiscardableData) {   // undefined element with blank content
      rcdbnd = rcdbndBackup;
      buf[0] = 0;
      return 0;
   }

   /*------------------------------------------------------------------+
   |  Make other pass(es) over the buffer to kill trailing or leading  |
   |  spaces it the Application wants that.                            |
   +------------------------------------------------------------------*/
   int iLen = pUcCur - buf;
   if (iLen) {
      if (options == YSP__killLEAD) {
         UCS_2 * from = buf;
         while (iLen && sdcl.charset().isSeparator(*from)) {
            ++from, --iLen;
         }
         memmove(buf, from, sizeof(UCS_2) * iLen);
      }
      if (options == YSP__killTRAIL) {
         pUcCur = buf + iLen;
         while (iLen && sdcl.charset().isSeparator(*--pUcCur)) {
            --iLen;
         }
      }
   }
   buf[iLen] = 0;   // upward compatibility (0 sentinel to end the buffer)
   return iLen;
}

/*----------------------------------------------------------getXmlPiAttrValue-+
| Effects:                                                                    |
|   Called by readXmlPiAttribute to return the value, or Nil if syntax error. |
+----------------------------------------------------------------------------*/
UnicodeString Yasp3::getXmlPiAttrValue(UCS_2 & ucDelim)
{
   /*-- look for Eq */
   skip_S_noFind();
   if (oec.peek() != (UCS_2)'=') {
      return UnicodeString::Nil;
   }
   oec.skip();
   skip_S_noFind();

   /*-- Look for a quoted literal */
   int iQuote = oec.peek();
   if ((iQuote != (UCS_2)'\"') && (iQuote != (UCS_2)'\'')) {
      return UnicodeString::Nil;
   }

   /*-- Get the value */
   int iCur;
   int iLen = 0;
   while ((iCur=oec.peek(++iLen)), (iCur != iQuote)) {
      // BACKUP NEEDED
      if (
         (iCur == EOF_SIGNAL) ||
         (iCur == (UCS_2)'?') ||
         (iCur == (UCS_2)'>')
      ) {
         return UnicodeString::Nil;
      }
   }
   oec.skip(iLen+1);
   ucDelim = (UCS_2)iCur;
   return UnicodeString(entMgr.peekPrevChars(iLen), iLen-1);
}

/*---------------------------------------------------------readXmlPiAttribute-+
| Effects:                                                                    |
|   Get the value out of: S LatinName Eq value                                |
+----------------------------------------------------------------------------*/
bool Yasp3::readXmlPiAttribute(
   UnicodeString const & ucsName,
   UnicodeString & ucsValue,
   UCS_2 & ucDelim
) {
   ucDelim = 0;
   ucsValue = UnicodeString::Nil;

   skip_S_noFind();
   if (
      skip(ucsName, false) &&         // name found
      (ucsValue = getXmlPiAttrValue(ucDelim), ucsValue.empty())
   ) {
      return false;
   }
   return true;
}

/*-------------------------------------------------------------------endXmlPi-+
| Effects:                                                                    |
|   End the XML PI.                                                           |
+----------------------------------------------------------------------------*/
bool Yasp3::endXmlPi()
{
   skip_S_noFind();
   if ((oec.peek() == (UCS_2)'?') && (oec.peek(1) == (UCS_2)'>')) {
      oec.skip(2);
      return true;
   }
   return false;
}

/*===========================================================================*/
