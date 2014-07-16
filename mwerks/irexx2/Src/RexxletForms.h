#ifndef __RexxletForms_H__
#define __RexxletForms_H__

/*----------------------------------------------------------CRexxletAlertForm-+
| A generic "modal" form for messages to be used with Rexxlets.               |
+----------------------------------------------------------------------------*/
class CRexxletAlertForm : public CModalForm
{
public:
   static void popup(char const * message);

   Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
   Boolean OnOK(EventPtr pEvent, Boolean & bHandled);

   CString m_message;

   BEGIN_DDX_MAP()
      DDX_TEXT(RexxletAlertMessageField, m_message)
   END_DDX_MAP()

   BEGIN_EVENT_MAP(CModalForm)
      EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
      EVENT_MAP_COMMAND_ENTRY(RexxletAlertOKButton, OnOK)
   END_EVENT_MAP()
};

/*----------------------------------------------------------CRexxletPauseForm-+
|                                                                             |
+----------------------------------------------------------------------------*/
class CRexxletPauseForm : public CModalForm
{
public:
   static void popup(int rc);

private:
   int m_rc;
   CRexxletPauseForm(int rc);
   Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
   Boolean OnDone(EventPtr pEvent, Boolean & bHandled);

   BEGIN_EVENT_MAP(CModalForm)
      EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
      EVENT_MAP_COMMAND_ENTRY(RexxletPauseDoneButton, OnDone)
   END_EVENT_MAP()
};

#endif
