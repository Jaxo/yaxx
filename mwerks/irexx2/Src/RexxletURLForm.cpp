
#include "Common.h"

#include "RexxletURLForm.h"

#include "IRexxApp.h"

Boolean CRexxletURLForm::OnOpen(EventPtr pEvent, Boolean& bHandled)
{
	// DDX will pass values of member variables into controls
	DoDataExchange(DDX_DataToControl);
	
	ShowObject(RexxletURLURLField);
	SetFocus(RexxletURLURLField);		
	
	bHandled = false;
	return false;
}

// OK button handler
Boolean CRexxletURLForm::OnOK(EventPtr pEvent, Boolean& bHandled)
{
	// DDX will pass values of controls into member variables
	DoDataExchange(DDX_ControlToData);

	CloseForm(true);
	return true;
}

// Cancel button handler
Boolean CRexxletURLForm::OnCancel(EventPtr pEvent, Boolean& bHandled)
{
	CloseForm(false);
	return false;
}
