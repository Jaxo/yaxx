#ifndef __LoadForm_H__
#define __LoadForm_H__

class CMyDBGrid : public CDBGrid
{
public:
	virtual const Char * GetItemText(Int16 row, Int16 col);
private:
	bool isFirstSegment(UInt16 positionInCategory, int matchSegmentNo, const char * matchId);
};

// Load form class
class CLoadForm : public CForm
{
public:
	// Command handlers
	Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
	Boolean OnExit(EventPtr pEvent, Boolean & bHandled);
	Boolean OnAbout(EventPtr pEvent, Boolean & bHandled);
	Boolean OnRun(EventPtr pEvent, Boolean & bHandled);
	Boolean OnRunWithArgs(EventPtr pEvent, Boolean & bHandled);
	Boolean OnEdit(EventPtr pEvent, Boolean & bHandled);

	// Event map
	BEGIN_EVENT_MAP(CForm)
		EVENT_MAP_GRID_ENTRY(m_grid)
		EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
		EVENT_MAP_MENU_ENTRY(LoadToolsRun, OnRun)
		EVENT_MAP_MENU_ENTRY(LoadToolsRunwithargs, OnRunWithArgs)
		EVENT_MAP_MENU_ENTRY(LoadToolsEdit, OnEdit)
		EVENT_MAP_MENU_ENTRY(LoadToolsExit, OnExit)
		EVENT_MAP_MENU_ENTRY(HelpAboutRexx, OnAbout)
		EVENT_MAP_COMMAND_ENTRY(LoaditRunButton, OnRun)
		EVENT_MAP_COMMAND_ENTRY(LoaditRunWithArgsButton, OnRunWithArgs)
		EVENT_MAP_COMMAND_ENTRY(LoaditEditButton, OnEdit)
		EVENT_MAP_COMMAND_ENTRY(LoaditDoneButton, OnExit)
	END_EVENT_MAP()

	// Post-event map
	BEGIN_POST_EVENT_MAP(CForm)
		POST_EVENT_MAP_GRID_ENTRY(m_grid)
	END_POST_EVENT_MAP()

protected:
	// Grid variable
	CMyDBGrid m_grid;
	
private:
	bool checkSelectedDBRecord();
	void copySelectedDBRecord();
};

#endif
