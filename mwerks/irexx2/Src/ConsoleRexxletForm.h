/* $Id: ConsoleRexxletForm.h,v 1.14 2002/07/16 16:53:11 jlatone Exp $ */

#ifndef __ConsoleRexxletForm_H__
#define __ConsoleRexxletForm_H__

#include "ConsoleFilter.h"

/*------------------------------------------------ class CConsoleRexxletForm -+
|                                                                             |
+----------------------------------------------------------------------------*/
class CConsoleRexxletForm : public CModalFormFilter, public ConsoleFilter
{
public:
   CConsoleRexxletForm() : isInterpreterRunning(false) {}
   virtual Boolean PreSystemEventHook(EventType *pEvent);

   Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
   Boolean OnDone(EventPtr pEvent, Boolean & bHandled);

   BEGIN_EVENT_MAP(CModalFormFilter)
      EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
      EVENT_MAP_ENTRY(appStopEvent, OnAppStop)
      EVENT_MAP_COMMAND_ENTRY(RexxletConsoleDoneButton, OnDone)
      EVENT_MAP_MEMO_ENTRY(m_memo)
   END_EVENT_MAP()

private:
   bool open();   // virtual from PalmConsole
   bool isInterpreterRunning;  // see comment in .cpp code
};

#endif
/*===========================================================================*/
