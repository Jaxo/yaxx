/* $Id: ConsoleRexxletForm.cpp,v 1.20 2002/08/16 13:15:30 pgr Exp $ */

#include "Common.h"

#include "ConsoleRexxletForm.h"
#include "PalmDbmStream.h"
#include "IRexxApp.h"

#include "StringBuffer.h"
#include "Rexx.h"

/*------------------------------------CConsoleRexxletForm::PreSystemEventHook-+
| Called everywhere!  And, that's what's good about it because it will give   |
| us a change to handle exceptional events like AppStop that can cause        |
| problems depending on the context, e.g., pause on return in a Rexxlet.      |
+----------------------------------------------------------------------------*/
Boolean CConsoleRexxletForm::PreSystemEventHook(EventType *pEvent)
{
   switch (pEvent->eType) {
   case appStopEvent:
      //We have to be sure we don't jump out of this form
      //because its event loop pushes/pops the current active form
      //(THAT OF THE LAUNCHING APP!) and if we exit this form via
      //exception, and bypass the pop, then the launching app is DOA.
      if (isInterpreterRunning) { // throw is properly dealt with
         throw AppStopEventException();
      } else { // too late to deal with throw, so close down normally
         CloseForm(2);
         return true;
      }
   default:
      return false;
   }
}

/*--------------------------------------------------CConsoleRexxletForm::open-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool CConsoleRexxletForm::open() {
// RegisterFilter(this);        >>>PGR: Now in initFilterConsole.  misplaced?
   showInitialDisplay();
   return true;
}

/*------------------------------------------------CConsoleRexxletForm::OnOpen-+
| Dialog Result:                                                              |
|    0 -> Application ended                                                   |
|    1 -> Application ended - a Pause was done                                |
|    2 -> Application has been stopped                                        |
+----------------------------------------------------------------------------*/
Boolean CConsoleRexxletForm::OnOpen(EventPtr pEvent, Boolean & bHandled)
{
   int rc;
   RexxString result;

   initFilterConsole(
      GetFormPtr(),
      this,
      RexxletConsoleIOField,
      RexxletConsoleIOScrollBar,
      RexxletConsoleReturnCodeField,
      RexxletConsoleDoneButton
   );

   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();

   try {
      isInterpreterRunning = true;
      pApp->interpret(result, rc);
      isInterpreterRunning = false;  // needed to properly deal with appStop
                                     // in the pre-system event hook above
   }catch (AppStopEventException) {
      isInterpreterRunning = false;
      CloseForm(2);              // stopped!
      bHandled = true;
      return true;
   }

   if (isOpened() && pApp->isRexxletPauseWhenDone()) {
      showFinalDisplay(rc);
   }else {
      CloseForm(0);
   }

   bHandled = true;
   return true;
}

/*------------------------------------------------CConsoleRexxletForm::OnDone-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleRexxletForm::OnDone(EventPtr pEvent, Boolean& bHandled)
{
   CloseForm(1);               // no need for a Pause (already done!)
   return true;
}

/*===========================================================================*/
