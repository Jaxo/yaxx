/* $Id: EditRexxletForm.cpp,v 1.2 2002/06/30 22:10:01 jlatone Exp $ */

#include "Common.h"

#include "EditRexxletForm.h"

#include "IRexxApp.h"

/*---------------------------------------------------CEditRexxletForm::OnOpen-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditRexxletForm::OnOpen(EventPtr pEvent, Boolean & bHandled)
{
   m_memo.Attach(RexxletEditEditField, RexxletEditEditScrollBar);

   DrawForm();
   SetFocus(RexxletEditEditField);
   ShowObject(RexxletEditDoneButton);

   bHandled = true;
   return true;
}

/*---------------------------------------------------CEditRexxletForm::OnDone-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditRexxletForm::OnDone(EventPtr pEvent, Boolean& bHandled)
{
	CIRexxApp* pApp = (CIRexxApp *)CIRexxApp::GetInstance();
	pApp->setScript(m_memo.GetTextPtr(), m_memo.GetTextLength());
   CloseForm(true);               
   return true;
}

/*-------------------------------------------------CEditRexxletForm::OnCancel-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean CEditRexxletForm::OnCancel(EventPtr pEvent, Boolean& bHandled)
{
   CloseForm(false);               
   return false;
}

/*===========================================================================*/
