
#include "Common.h"

#include "IRexxApp.h"
#include "EditForm.h"
#include "ConsoleForm.h"
#include "LoadForm.h"
#include "ArgsForm.h"

CEditForm* CEditForm::m_pInstance = NULL;

static const char * msgNoScript = "Please enter or load a script to run.";
static const char * msgNoScriptsFound = "There were no scripts found to load.";

Boolean CEditForm::OnOpen(EventPtr pEvent, Boolean& bHandled)
{
	CIRexxApp * pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   if (!pApp->isScriptIndexDBInitialized()) {
      pApp->NewScriptIndexDB();
   }
	m_memo.Attach(EditInstructionsField, EditInstructionsScrollBar);
	DrawForm();
	if (pApp->isScriptEmpty()) {
		//if (!pApp->isScriptDirty()) {
			//m_memo.Insert("SAY \"Enter a Rexx script here,\n"
			//			  "or press Load to select one from MemoPad"
			//			  " (simply start typing to erase this).\"");
			//m_memo.SetSelection(0, m_memo.GetTextLength());
		//}
		SetFocus(EditInstructionsField);	
		m_memo.SetInsPtPosition(0);
	} else {
		UInt16 insert, scroll, select1, select2;
		pApp->restoreScriptPositions(insert, scroll, select1, select2);
		m_memo.HideMemo();
		pApp->copyScriptToMemo(m_memo);
		m_memo.DrawField();
		if (select1 != select2) { m_memo.SetSelection(select1, select2); }
		m_memo.SetInsPtPosition(insert);
		m_memo.ShowMemo();
		SetFocus(EditInstructionsField);	
		m_memo.SetScrollPosition(scroll);
		m_memo.UpdateScrollBar();
	}		  
	CString numberOfScripts;
	numberOfScripts.Format("%4ld", (long)pApp->numberOfScripts());
	CField nos(EditNumberOfScriptsField);
	nos.Replace(numberOfScripts);
	bHandled = true;
	return true;
}

Boolean CEditForm::OnExit(EventPtr pEvent, Boolean& bHandled)
{
   //CPalmStApp<CIRexxApp>::Stop();  see comment on console form's exit
   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
	pApp->setScript(m_memo.GetTextPtr(), m_memo.GetTextLength());
   GotoLauncher();
   return true;
}

Boolean CEditForm::OnAbout(EventPtr pEvent, Boolean& bHandled)
{
   CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
   pApp->doAbout();
	return true;
}

Boolean CEditForm::OnRun(EventPtr pEvent, Boolean& bHandled)
{
	if (m_memo.GetTextLength() == 0) { 
   	FrmCustomAlert(InformationAlert, msgNoScript, 0, 0);
		return true; 
	}
	setScriptAndSaveScriptPositions();
	CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
	pApp->BALR(GetFormID(), ConsoleForm);	
	return true;
}

Boolean CEditForm::OnRunWithArgs(EventPtr pEvent, Boolean& bHandled)
{
	if (m_memo.GetTextLength() == 0) { 
   FrmCustomAlert(InformationAlert, msgNoScript, 0, 0);
		return true; 
	}
	CArgsForm frmArgs;
	if (frmArgs.DoModal())
	{
		CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
		pApp->setArgs(frmArgs.m_strArgsField, frmArgs.m_strArgsField.GetLength());
		setScriptAndSaveScriptPositions();
		pApp->BALR(GetFormID(), ConsoleForm);	
	}
	return true;
}

Boolean CEditForm::OnLoad(EventPtr pEvent, Boolean& bHandled)
{
	CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
	if (pApp->numberOfScripts() == 0) { 
   	FrmCustomAlert(InformationAlert, msgNoScriptsFound, 0, 0);
		return true; 
	}
	if (m_memo.GetTextLength() != 0) { 
		setScriptAndSaveScriptPositions();
	}
	CForm::GotoForm(LoaditForm);	
	return true;
}

Boolean CEditForm::OnClear(EventPtr pEvent, Boolean& bHandled)
{
	CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
	m_memo.Replace("");
	pApp->setScript("", 0);
	return true;
}

void CEditForm::setScriptAndSaveScriptPositions()
{
	UInt16 s1 = 0, s2 = 0;
	CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
	pApp->setScript(m_memo.GetTextPtr(), m_memo.GetTextLength());
	m_memo.GetSelection(s1, s2);
	pApp->saveScriptPositions(
		m_memo.GetInsPtPosition(), 
		m_memo.GetScrollPosition(),
		s1, s2);
	return;
}	

/*----------------------------------------------------------CEditForm::OnUndo-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditForm::OnUndo(EventPtr pEvent, Boolean & bHandled) {
   m_memo.Undo();
   return true;
}

/*-----------------------------------------------------------CEditForm::OnCut-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditForm::OnCut(EventPtr pEvent, Boolean & bHandled) {
   m_memo.Cut();
   return true;
}

/*----------------------------------------------------------CEditForm::OnCopy-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditForm::OnCopy(EventPtr pEvent, Boolean & bHandled) {
   m_memo.Copy();
   return true;
}

/*---------------------------------------------------------CEditForm::OnPaste-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditForm::OnPaste(EventPtr pEvent, Boolean & bHandled) {
   m_memo.Paste();
   return true;
}

/*-----------------------------------------------------CEditForm::OnSelectAll-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditForm::OnSelectAll(EventPtr pEvent, Boolean & bHandled) {
   m_memo.SetSelection(0, m_memo.GetTextLength());
   return true;
}

/*------------------------------------------------------CEditForm::OnKeyboard-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditForm::OnKeyboard(EventPtr pEvent, Boolean & bHandled) {
   SysKeyboardDialog(kbdDefault);
   return true;
}

/*--------------------------------------------------CEditForm::OnGraffitiHelp-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditForm::OnGraffitiHelp(EventPtr pEvent, Boolean & bHandled) {
   SysGraffitiReferenceDialog(referenceDefault);
   return true;
}
