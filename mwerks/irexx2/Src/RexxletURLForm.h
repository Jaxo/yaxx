#ifndef __RexxletURLForm_H__
#define __RexxletURLForm_H__

class CRexxletURLForm : public CModalForm
{
public:
	// Constructor
	CRexxletURLForm() : CModalForm(RexxletURLForm) { }

	// Command handlers
	Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
	Boolean OnOK(EventPtr pEvent, Boolean & bHandled);
	Boolean OnCancel(EventPtr pEvent, Boolean & bHandled);

	// Event map
	BEGIN_EVENT_MAP(CModalForm)
		EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
		EVENT_MAP_COMMAND_ENTRY(RexxletURLOKButton, OnOK)
		EVENT_MAP_COMMAND_ENTRY(RexxletURLCancelButton, OnCancel)
	END_EVENT_MAP()

	// Dynamic data echange map
	BEGIN_DDX_MAP()
		DDX_TEXT(RexxletURLURLField, m_strUrlField)
	END_DDX_MAP()

	// Data variables
	CString m_strUrlField;
};

#endif
