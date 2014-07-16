/* $Id: ConsoleForm.cpp,v 1.44 2002/08/25 19:31:39 jlatone Exp $ */

#include "Common.h"

#include "ConsoleForm.h"
#include "PalmDbmStream.h"

#include "IRexxApp.h"

#include "StringBuffer.h"
#include "Rexx.h"

//
//Abnormal exit test by running rexxtry and testing all scenarios:
//
//1. Press App
//2. Press Tools->Exit
//3. Enter "exit" and then press App
//4. Enter "exit" and then press Tools->Exit
//5. Enter "exit" and then launch via Find!
//6. Press Calc
//7. Run rexxtry from within rexxtry as a Rexxlet and press App
//   (NOT WORKING ON PALM OS 5.0 FOR SAME REASON
//   THE EMBEDDED REXXLET LAUNCHER DOES NOT WORK!)
//8. Press Calc and run a Rexxlet
//   (NOT WORKING ON PALM OS 5.0)
//
//All take different paths and have caused problems
//because of the order of tear-down.  #3 is a problem
//because we do not control the exit.
//
//If we want to get out cleanly, , via Stop() or exit(), then we have to close
//this form first, otherwise we core dump in streambuf::sync, which happens to
//be a pure virtual, which probably means we're freeing the underlying
//concrete object (a PalmConsole) more than once.  This may have to do with
//the ConsoleForm class structure and automatic cleanup via CloseAllForms.
//FIXED THIS ON JULY 22, 2002.

/*-------------------------------------------------------CConsoleForm::OnOpen-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnOpen(EventPtr pEvent, Boolean & bHandled)
{
   int rc;
   RexxString result;

// RegisterFilter(this);   >>>PGR: done with initFilterConsole.
   initFilterConsole(
      GetFormPtr(),
      this,
      ConsoleIOField,
      ConsoleIOScrollBar,
      ConsoleReturnCodeField,
      ConsoleDoneButton
   );

   showInitialDisplay();

   try {
      ((CIRexxApp *)CIRexxApp::GetInstance())->interpret(result, rc);
   } catch (AppStopEventException) {
      CloseAllForms();
      //It's ok to just STOP here because we know we did an AppStop
      CPalmStApp<CIRexxApp>::Stop();
      return true;
   }

   showFinalDisplay(rc);

   bHandled = true;
   return true;
}

/*-------------------------------------------------------CConsoleForm::OnExit-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnExit(EventPtr pEvent, Boolean & bHandled)
{
   CloseAllForms();
   
   //Ever wonder why Palm apps never have an Exit button?
   //It's because there's no place to go!
   //I've know this for some time (from Palm docs),
   //but your app is not supposed to exit simply by stopping.
   //It's supposed to exit as a result of some user action
   //and explicitly go somewhere else, even if it's the app launcher.
   //I discovered this because of we switch to Rexx via SysUIAppSwitch(),
   //then when Rexx does it's exit without going anywhere, the Palm
   //relaunches the app that did the SysUIAppSwitch...ad infinitum.
   //I think the Palm does this because if it has no explicit place to go,
   //it tries to go to the most-recent valid application.  This is
   //typically the system launcher, but now that we're launching rexx too,
   //we ran into this problem.    
//   CPalmStApp<CIRexxApp>::Stop();
   GotoLauncher();
   return true;
}

/*-------------------------------------------------------CConsoleForm::OnDone-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnDone(EventPtr pEvent, Boolean & bHandled)
{
   ((CIRexxApp *)CIRexxApp::GetInstance())->RET();
   bHandled = true;
   return true;
}

/*------------------------------------------------------CConsoleForm::OnAbout-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnAbout(EventPtr pEvent, Boolean & bHandled)
{
   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   pApp->doAbout();
   return true;
}

/*-------------------------------------------------------CConsoleForm::OnUndo-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnUndo(EventPtr pEvent, Boolean & bHandled) {
   m_memo.Undo();
   return true;
}

/*--------------------------------------------------------CConsoleForm::OnCut-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnCut(EventPtr pEvent, Boolean & bHandled) {
   if (isInEditableArea()) {
      m_memo.Cut();
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------------------------CConsoleForm::OnCopy-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnCopy(EventPtr pEvent, Boolean & bHandled) {
   m_memo.Copy();
   return true;
}

/*------------------------------------------------------CConsoleForm::OnPaste-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnPaste(EventPtr pEvent, Boolean & bHandled) {
   if (isInEditableArea()) {
      m_memo.Paste();
      return true;
   }else {
      return false;
   }
}

/*--------------------------------------------------CConsoleForm::OnSelectAll-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnSelectAll(EventPtr pEvent, Boolean & bHandled) {
   m_memo.SetSelection(0, m_memo.GetTextLength());
   return true;
}

/*---------------------------------------------------CConsoleForm::OnKeyboard-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnKeyboard(EventPtr pEvent, Boolean & bHandled) {
   SysKeyboardDialog(kbdDefault);
   return true;
}

/*-----------------------------------------------CConsoleForm::OnGraffitiHelp-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CConsoleForm::OnGraffitiHelp(EventPtr pEvent, Boolean & bHandled) {
   SysGraffitiReferenceDialog(referenceDefault);
   return true;
}

/*===========================================================================*/
