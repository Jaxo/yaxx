
#include "Common.h"

#include "RexxletForms.h"

#include "ConsoleFilter.h"

/*STATIC----------------------------------------------------------------------+
| CANNOT HAVE REAL MODAL DIALOGS, e.g., FrmCustomAlert, as in:                |
|   MemStream pauseMessage;                                                   |
|   pauseMessage << "Press OK when ready (rc=" << rc << ")." << '\0';         |
|   FrmCustomAlert(PauseWhenDoneAlert, pauseMessage.str(), 0, 0);             |
|   pauseMessage.rdbuf()->freeze(0);                                          |
|                                                                             |
| They seem to cause a problem in the launching application on return.        |
| NOT SURE WHY.  I think the point is that we always need to run our own      |
| event loop because the launching apps event loop is still running,          |
| DON'T FORGET!                                                               |
| >>>JAL: TODO Better test to see if other modals                             |
|              might cause problems, like the                                 |
|              ones popped up during exceptions!                              |
|              or, can we use the args dialog                                 |
|              in a rexxlet?                                                  |
+----------------------------------------------------------------------------*/

void CRexxletPauseForm::popup(int rc) {
   CRexxletPauseForm pauseWhenDone(rc);
   SndPlaySystemSound(sndInfo);
   pauseWhenDone.DoModal(RexxletPauseForm);
}
CRexxletPauseForm::CRexxletPauseForm(int rc) : m_rc(rc) {}
Boolean CRexxletPauseForm::OnOpen(EventPtr pEvent, Boolean & bHandled)
{
   ConsoleFilter::formatRcField(
      m_rc, GetFormPtr(), RexxletPauseReturnCodeField, 11
   );
   bHandled = false;
   return false;
}
Boolean CRexxletPauseForm::OnDone(EventPtr pEvent, Boolean & bHandled)
{
   CloseForm(true);
   return true;
}


void CRexxletAlertForm::popup(char const * message) {
   CRexxletAlertForm * alert = new CRexxletAlertForm();
   alert->m_message = message;
   alert->DoModal(RexxletAlertForm);
   delete alert;
}
Boolean CRexxletAlertForm::OnOpen(EventPtr pEvent, Boolean & bHandled) {
   DoDataExchange(DDX_DataToControl);
   DrawForm();
   return true;
}
Boolean CRexxletAlertForm::OnOK(EventPtr pEvent, Boolean & bHandled) {
   CloseForm(true);
   return true;
}


