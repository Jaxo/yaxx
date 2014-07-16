/* $Id: EditRexxletForm.h,v 1.2 2002/06/30 22:10:01 jlatone Exp $ */

#ifndef __EditRexxletForm_H__
#define __EditRexxletForm_H__

/*--------------------------------------------------- class CEditRexxletForm -+
|                                                                             |
+----------------------------------------------------------------------------*/
class CEditRexxletForm : public CModalForm
{
public:
   Boolean OnOpen(EventPtr pEvent, Boolean & bHandled);
   Boolean OnDone(EventPtr pEvent, Boolean & bHandled);
   Boolean OnCancel(EventPtr pEvent, Boolean & bHandled);

   BEGIN_EVENT_MAP(CModalForm)
      EVENT_MAP_ENTRY(frmOpenEvent, OnOpen)
      EVENT_MAP_COMMAND_ENTRY(RexxletEditDoneButton, OnDone)
      EVENT_MAP_COMMAND_ENTRY(RexxletEditCancelButton, OnCancel)
      EVENT_MAP_MEMO_ENTRY(m_memo)
   END_EVENT_MAP()

protected:
   CMemo m_memo;
};

#endif
/*===========================================================================*/
