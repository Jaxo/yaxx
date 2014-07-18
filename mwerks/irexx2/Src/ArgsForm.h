#ifndef __ArgsForm_H__
#define __ArgsForm_H__

class CArgsForm : public CModalForm
{
public:
	// Constructor
	CArgsForm() : CModalForm(ArgsForm) { }

	// Command handlers
	Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
	Boolean OnOK(EventPtr pEvent, Boolean & bHandled);
	Boolean OnCancel(EventPtr pEvent, Boolean & bHandled);

	// Event map
	BEGIN_EVENT_MAP(CModalForm)
		EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
		EVENT_MAP_COMMAND_ENTRY(ArgsRunButton, OnOK)
		EVENT_MAP_COMMAND_ENTRY(ArgsCancelButton, OnCancel)
	END_EVENT_MAP()

	// Dynamic data echange map
	BEGIN_DDX_MAP()
		DDX_TEXT(ArgsArgsField, m_strArgsField)
	END_DDX_MAP()

	// Data variables
	CString m_strArgsField;
};

#endif
