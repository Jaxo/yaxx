
#include "Common.h"

#include "ArgsForm.h"

#include "IRexxApp.h"

Boolean CArgsForm::OnOpen(EventPtr pEvent, Boolean& bHandled)
{
	// DDX will pass values of member variables into controls
	DoDataExchange(DDX_DataToControl);
	
	ShowObject(ArgsArgsField);
	//SetInsPtPosition(0);
	//DrawField(ArgsArgsField);
	SetFocus(ArgsArgsField);		
	
	bHandled = false;
	return false;
}

// OK button handler
Boolean CArgsForm::OnOK(EventPtr pEvent, Boolean& bHandled)
{
	// DDX will pass values of controls into member variables
	DoDataExchange(DDX_ControlToData);

	CloseForm(true);
	return true;
}

// Cancel button handler
Boolean CArgsForm::OnCancel(EventPtr pEvent, Boolean& bHandled)
{
	CloseForm(false);
	return false;
}
