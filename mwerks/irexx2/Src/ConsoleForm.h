/* $Id: ConsoleForm.h,v 1.18 2002/11/13 21:06:20 jlatone Exp $ */

#ifndef __ConsoleForm_H__
#define __ConsoleForm_H__

#include "ConsoleFilter.h"

/*------------------------------------------------------- class CConsoleForm -+
|                                                                             |
+----------------------------------------------------------------------------*/
class CConsoleForm : public CFormFilter, public ConsoleFilter
{
public:
   Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
   Boolean OnAbout(EventPtr pEvent, Boolean & bHandled);
   Boolean OnExit(EventPtr pEvent, Boolean & bHandled);
   Boolean OnDone(EventPtr pEvent, Boolean & bHandled);
   Boolean OnUndo(EventPtr pEvent, Boolean & bHandled);
   Boolean OnCut(EventPtr pEvent, Boolean & bHandled);
   Boolean OnCopy(EventPtr pEvent, Boolean & bHandled);
   Boolean OnPaste(EventPtr pEvent, Boolean & bHandled);
   Boolean OnSelectAll(EventPtr pEvent, Boolean & bHandled);
   Boolean OnKeyboard(EventPtr pEvent, Boolean & bHandled);
   Boolean OnGraffitiHelp(EventPtr pEvent, Boolean & bHandled);

   BEGIN_EVENT_MAP(CFormFilter)
      EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
      EVENT_MAP_COMMAND_ENTRY(ConsoleDoneButton, OnDone)
      EVENT_MAP_MENU_ENTRY(HelpAboutRexx, OnAbout)
      EVENT_MAP_MENU_ENTRY(ConsoleToolsExit, OnExit)
      EVENT_MAP_MENU_ENTRY(EditUndo, OnUndo)
      EVENT_MAP_MENU_ENTRY(EditCut, OnCut)
      EVENT_MAP_MENU_ENTRY(EditCopy, OnCopy)
      EVENT_MAP_MENU_ENTRY(EditPaste, OnPaste)
      EVENT_MAP_MENU_ENTRY(EditSelectAll, OnSelectAll)
      EVENT_MAP_MENU_ENTRY(EditKeyboard, OnKeyboard)
      EVENT_MAP_MENU_ENTRY(EditGraffitiHelp, OnGraffitiHelp)
      EVENT_MAP_MEMO_ENTRY(m_memo)
   END_EVENT_MAP()

};

#endif
/*===========================================================================*/
