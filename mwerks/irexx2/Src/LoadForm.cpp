/* $Id: LoadForm.cpp,v 1.27 2002/08/26 04:17:29 jlatone Exp $ */

#include "Common.h"

#include "LoadForm.h"

#include "IRexxApp.h"
#include "ArgsForm.h"

Boolean CLoadForm::OnOpen(EventPtr pEvent, Boolean& bHandled)
{
   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   if (!pApp->isScriptIndexDBInitialized()) {
      pApp->NewScriptIndexDB();
   }
   int numberOfScripts = pApp->numberOfScripts();
   CString numberOfScriptsAsString;
   numberOfScriptsAsString.Format("%4ld", (long)numberOfScripts);
   CField nos(LoaditNumberOfScriptsField);
   nos.Replace(numberOfScriptsAsString);
   m_grid.Attach(LoaditListTable, LoaditListScrollBar, pApp->databaseOfScripts());
   m_grid.SetGridMode(CGrid::GM_FULLROWSELECT);
   m_grid.SetDrawStyle(CGrid::DS_COLUMNS);
   m_grid.MakeRowVisible(pApp->getLoadPosition());
   bHandled = false;
   return false;
}

const Char * CMyDBGrid::GetItemText(Int16 row, Int16 col)
{
   static CString s;
   CIRexxApp * pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   s.Empty();
   switch (col) {
   case 0:
      s.Format(
        "%ld. ",
        (long)(1 + GetDatabase()->PositionInCategory(
           GetRowID(row), GetCategory())
        )
      );
      pApp->titleOfScript(GetRowID(row), s);
      break;
   case 1:
      pApp->dbIndicatorOfScript(GetRowID(row), s);   
      break;
   default:
      s = "";
   }
   return s;
}

Boolean CLoadForm::OnRun(EventPtr pEvent, Boolean& bHandled)
{
   if (!checkSelectedDBRecord()) { return true; }
   copySelectedDBRecord();
   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   pApp->setLoadPosition(m_grid.GetSelection());
   pApp->BALR(GetFormID(), ConsoleForm);
   return true;
}

Boolean CLoadForm::OnRunWithArgs(EventPtr pEvent, Boolean& bHandled)
{
   if (!checkSelectedDBRecord()) { return true; }
   copySelectedDBRecord();
   CArgsForm frmArgs;
   if (frmArgs.DoModal()) {
      CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
      pApp->setArgs(frmArgs.m_strArgsField, frmArgs.m_strArgsField.GetLength());
      pApp->setLoadPosition(m_grid.GetSelection());
      pApp->BALR(GetFormID(), ConsoleForm);
   }
   return true;
}

Boolean CLoadForm::OnEdit(EventPtr pEvent, Boolean& bHandled)
{
   copySelectedDBRecord();
   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   pApp->setLoadPosition(m_grid.GetSelection());
   CForm::GotoForm(EditForm);
   return true;
}

Boolean CLoadForm::OnExit(EventPtr pEvent, Boolean& bHandled)
{
   //CPalmStApp<CIRexxApp>::Stop();  see comment in console form's exit
   GotoLauncher();
   return true;
}

Boolean CLoadForm::OnAbout(EventPtr pEvent, Boolean& bHandled)
{
   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   pApp->doAbout();
   return true;
}

bool CLoadForm::checkSelectedDBRecord()
{
   UInt16 row = m_grid.GetCurrentIndex();
   if (row == (UInt16)-1) {
      FrmCustomAlert(InformationAlert, "Please first select a script.", 0, 0);
      return false;
   }
   return true;
}

void CLoadForm::copySelectedDBRecord()
{
   UInt16 row = m_grid.GetCurrentIndex();
   if (row == (UInt16)-1) { return; }
   CIRexxApp * pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   pApp->copyScriptFromIndexDatabase(row);
   return;
}

/*===========================================================================*/
