/*
* $Id: ysperror.cpp,v 1.7 2002-09-04 07:47:04 pgr Exp $
*
* Error location reporting.
*
*/

/*---------+
| Includes |
+---------*/
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"
#include "../entity/entsolve.h"
#include "../yasp3.h"

#define YSPERR__MAX_TEXT 59  // How much entity context to show

/*--------------------------------------------------------Yasp3::errorContext-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::errorContext()
{
   ++errorCount;
   if (options != YSP__validate) {
      showErrorLocation();
      return;
   }

   /*
   | Set up the input-location from all open entities.
   */
   EntIterator iter(&entMgr);
   if (iter.top()) {
      showErrorLocation();
      while (iter.previous()) {
         startLocationMessage(iter.inqEntity(), false);
         erh << iter.inqLine() << iter.inqColumn() << endm;
      }
   }

   /*
   | Grab the open-tag-stack and report that part of our state:
   */
   {
      UnicodeMemWriter umost;
      if (elmMgr.inqTagLastOpened().inqLongName(umost)) {
         erh << ECE__INFO << _YSP__FQGI << umost << endm;
      }
   }
}

/*---------------------------------------------------Yasp3::showErrorLocation-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Yasp3::showErrorLocation()
{
   Entity const & ent = entMgr.inqEntity();
   if (!ent) return;

   int iPrev = entMgr.inqColumn();
   startLocationMessage(ent, true);
   erh << entMgr.inqLine() << iPrev << endm;

   if (iPrev > YSPERR__MAX_TEXT) iPrev = YSPERR__MAX_TEXT;
   int iNext;
   for (iNext=0; iNext < YSPERR__MAX_TEXT; ++iNext) {
      switch (entMgr.peek(iNext)) {
      case EOF_SIGNAL:
      case (UCS_2)'\n':
      case (UCS_2)'\r':
         break;
      default:
         continue;
      }
      break;
   }
   if (iPrev) {
      UnicodeString ucsPrev = entMgr.peekPrevChars(iPrev);
      iPrev = 0;
      {
         for (UCS_2 const * pUcBuf = ucsPrev; *pUcBuf; ++pUcBuf) {
            ++iPrev;
            if (*pUcBuf == (UCS_2)'\t') {      // make tabs == 1
               *(UCS_2 *)pUcBuf = (UCS_2)' ';  // violates constness
            }
         }
      }
      if (iNext) {
         erh << ECE__INFO << _YSP__CONTEXT1
             << ucsPrev << entMgr.peekNextChars(iNext) << endm;
      }else {
         erh << ECE__INFO << _YSP__CONTEXT2 << ucsPrev << endm;
      }
      --iPrev;
      UCS_2 * pUcBuf = new UCS_2[YSPERR__MAX_TEXT+2];
      int i;
      for (i=0; i < iPrev; ++i) {
          pUcBuf[i] = (UCS_2)'-';
      }
      pUcBuf[i] = (UCS_2)'*';
      pUcBuf[i+1] = 0;
      erh << ECE__INFO << _YSP__CONTEXT2 << pUcBuf << endm;
      delete [] pUcBuf;
   }else if (iNext) {
      erh << ECE__INFO << _YSP__CONTEXT2
          << entMgr.peekNextChars(iNext) << endm;
   }
}

/*------------------------------------------------Yasp3::startLocationMessage-+
| Effects:                                                                    |
|    Depending on the entity, if it is external, internal, the currently      |
|    opened one (isFirst), or a parent one,                                   |
|    start the appropriate location message.                                  |
+----------------------------------------------------------------------------*/
void Yasp3::startLocationMessage(Entity const & ent, bool isFirst)
{
   erh << ECE__INFO;
   UnicodeString ucsName;
   if (ent.isSpecialDtd()) {
      ucsName = dtdEntName;
   }else if (ent.isSpecialDoc()) {
      ucsName = docEntName;
   }else {
      ucsName = ent.inqName();
   }
   if (ent.isExternal()) {
      UnicodeString const & ucsPath = pSolver->inqAbsolutePath(ent);
      if (ucsPath.good()) {
         if (isFirst) {
            erh << _YSP__LOCATION1WF << ucsName << ucsPath;
         }else {
            erh << _YSP__LOCATION2WF << ucsName << ucsPath;
         }
         return;
      }
   }
   if (isFirst) {
      erh << _YSP__LOCATION1 << ucsName;
   }else {
      erh << _YSP__LOCATION2 << ucsName;
   }
}

/*===========================================================================*/
