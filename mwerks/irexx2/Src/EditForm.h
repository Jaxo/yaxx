#ifndef __EditForm_H__
#define __EditForm_H__

class CEditForm : public CForm
{
public:
	// Constructor
	CEditForm() { m_pInstance = this; }

	// Command handlers
	Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
	Boolean OnExit(EventPtr pEvent, Boolean & bHandled);
	Boolean OnAbout(EventPtr pEvent, Boolean & bHandled);
	Boolean OnRun(EventPtr pEvent, Boolean & bHandled);
	Boolean OnRunWithArgs(EventPtr pEvent, Boolean & bHandled);
	Boolean OnLoad(EventPtr pEvent, Boolean & bHandled);
	Boolean OnClear(EventPtr pEvent, Boolean & bHandled);
    Boolean OnUndo(EventPtr pEvent, Boolean & bHandled);
    Boolean OnCut(EventPtr pEvent, Boolean & bHandled);
    Boolean OnCopy(EventPtr pEvent, Boolean & bHandled);
    Boolean OnPaste(EventPtr pEvent, Boolean & bHandled);
    Boolean OnSelectAll(EventPtr pEvent, Boolean & bHandled);
    Boolean OnKeyboard(EventPtr pEvent, Boolean & bHandled);
    Boolean OnGraffitiHelp(EventPtr pEvent, Boolean & bHandled);

	// Event map
	BEGIN_EVENT_MAP(CForm)
		EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
		EVENT_MAP_MENU_ENTRY(EditToolsRun, OnRun)
		EVENT_MAP_MENU_ENTRY(EditToolsRunwithargs, OnRunWithArgs)
		EVENT_MAP_MENU_ENTRY(EditToolsLoad, OnLoad)
		EVENT_MAP_MENU_ENTRY(EditToolsClear, OnClear)
		EVENT_MAP_MENU_ENTRY(EditToolsExit, OnExit)
		EVENT_MAP_MENU_ENTRY(HelpAboutRexx, OnAbout)
      EVENT_MAP_MENU_ENTRY(EditUndo, OnUndo)
      EVENT_MAP_MENU_ENTRY(EditCut, OnCut)
      EVENT_MAP_MENU_ENTRY(EditCopy, OnCopy)
      EVENT_MAP_MENU_ENTRY(EditPaste, OnPaste)
      EVENT_MAP_MENU_ENTRY(EditSelectAll, OnSelectAll)
      EVENT_MAP_MENU_ENTRY(EditKeyboard, OnKeyboard)
      EVENT_MAP_MENU_ENTRY(EditGraffitiHelp, OnGraffitiHelp)
		EVENT_MAP_COMMAND_ENTRY(EditRunButton, OnRun)
		EVENT_MAP_COMMAND_ENTRY(EditRunWithArgsButton, OnRunWithArgs)
		EVENT_MAP_COMMAND_ENTRY(EditLoadButton, OnLoad)
		EVENT_MAP_COMMAND_ENTRY(EditClearButton, OnClear)
		EVENT_MAP_COMMAND_ENTRY(EditDoneButton, OnExit)
		EVENT_MAP_MEMO_ENTRY(m_memo)
	END_EVENT_MAP()

	// Post event map
	BEGIN_POST_EVENT_MAP(CForm)
	END_POST_EVENT_MAP()

	CMemo & GetMemo() { return m_memo; }
	static CEditForm* GetInstance() { return m_pInstance; }

protected:
	CMemo m_memo;
	static CEditForm* m_pInstance;
private:
	void setScriptAndSaveScriptPositions();
};

#endif
