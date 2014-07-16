/* $Id: ConsoleFilter.cpp,v 1.25 2002/12/13 14:42:18 jlatone Exp $ */

#include "ConsoleFilter.h"
#include "IRexxApp.h"

/*-----------------------------------------------ConsoleFilter::ConsoleFilter-+
|                                                                             |
+----------------------------------------------------------------------------*/
ConsoleFilter::ConsoleFilter()
{
   m_sysCtxt = new SystemContext(
      "file://",
      PalmFileSchemeHandler(),
      PalmDbmSchemeHandler(),
      SerialSchemeHandler(),
      BeamerSchemeHandler(),
      NetSchemeHandler(),
      MidiSchemeHandler(),
      MySchemeHandler(this)
   );
   registerDecoder(new ClearDecoder(*this));
   registerDecoder(new BELDecoder(*this));
}

/*-------------------------------------------ConsoleFilter::initFilterConsole-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::initFilterConsole(
   FormPtr pForm,
   CEventFilter * pFilter,
   UInt16 fieldID,
   UInt16 scrollID,
   UInt16 rcValueID,
   UInt16 doneID
) {
   m_startPos = 0;
   m_pForm = pForm;
   m_fieldID = fieldID;
   m_pField = (FieldPtr)FrmGetObjectPtr(
      pForm,
      FrmGetObjectIndex(pForm, fieldID)
   );
   m_scrollID = scrollID;
   m_doneID = doneID;
   m_rcValueID = rcValueID;
   m_memo.Attach(m_fieldID, m_scrollID);
   pFilter->RegisterFilter(this);  //<<<JAL If we ever want to unwind this,
                                   //       then this may be a good place to start.
                                   //       POL might expect the Form and FormFilter to
                                   //       be different objects...here, they're the same.
                                   //       I suspect this is the root of the core dump problem.
}

/*----------------------------------------ConsoleFilter::showInitialDisplay()-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::showInitialDisplay() 
{
   FrmDrawForm(m_pForm);
   FrmSetFocus(m_pForm, FrmGetObjectIndex(m_pForm, m_fieldID));
   FrmHideObject(m_pForm, FrmGetObjectIndex(m_pForm, m_doneID));
   FrmHideObject(m_pForm, FrmGetObjectIndex(m_pForm, m_rcValueID));
}

/*--------------------------------------------ConsoleFilter::showFinalDisplay-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::showFinalDisplay(int rc)
{
   formatRcField(rc, m_pForm, m_rcValueID, 7);
   FrmShowObject(m_pForm, FrmGetObjectIndex(m_pForm, m_rcValueID));
   FrmShowObject(m_pForm, FrmGetObjectIndex(m_pForm, m_doneID));
}

/*----------------------------------------------ConsoleFilter::PreHandleEvent-+
|                                                                             |
+----------------------------------------------------------------------------*/
Boolean ConsoleFilter::PreHandleEvent(EventPtr pEvent, Boolean & bResult)
{
   switch (pEvent->eType) {
   case appStopEvent:
      throw AppStopEventException();

   case frmCloseEvent:
      delete m_sysCtxt;
      return false;

   case keyDownEvent:
      if (
         !isOpened()
         || (
            !TxtCharIsHardKey(
               pEvent->data.keyDown.modifiers,
               pEvent->data.keyDown.chr
            ) &&
            EvtKeydownIsVirtual(pEvent) && (
               (pEvent->data.keyDown.chr == vchrPageUp) ||
               (pEvent->data.keyDown.chr == vchrPageDown)
            )
         ) || (
            TxtGlueCharIsVirtual(
               pEvent->data.keyDown.modifiers,
               pEvent->data.keyDown.chr
            )
         )
      ) {
         return false;
      }else {
         char ch = (char)pEvent->data.keyDown.chr;
         if (ch < ' ') {
            switch (ch) {
            case '\b':
            case '\r':
            case '\f':
               break;
            case '\n':
                m_memo.SetInsPtPosition(m_memo.GetTextLength());
                break;
            default:
               return false;
            }
         }
         if (
            ((ch == '\b') && (m_memo.GetInsPtPosition() <= m_startPos)) ||
            (!isInEditableArea())
         ) {
            bResult = true;
            return true;
         }else {
            signalNewChars(&ch, 1);
            return false;
         }
      }

   default:
      return false;
   }
}

/*--------------------------------------------ConsoleFilter::isInEditableArea-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool ConsoleFilter::isInEditableArea() {
   return (m_memo.GetInsPtPosition() >= m_startPos);
}

/*------------------------------------------------------ConsoleFilter::refill-+
| Read the console (into a streambuf)                                         |
|                                                                             |
| Note: when characters are entered, signalNewChars must be called            |
|       (to eventually turn on the haveEnough boolean)                        |
|       or this loop will never ends!                                         |
+----------------------------------------------------------------------------*/
void ConsoleFilter::refill(streambuf & sb, bool volatile const & haveEnough)
{
// m_startPos = m_memo.GetInsPtPosition();
   m_startPos = m_memo.GetTextLength();
   while (!haveEnough) {
      EventType event;
      UInt16 key;
      EvtGetEvent(&event, evtWaitForever);

      if (
         (event.eType == keyDownEvent) &&
         TxtGlueCharIsVirtual(
            event.data.keyDown.modifiers, event.data.keyDown.chr
         ) && (
            key = event.data.keyDown.chr, (
               (key == vchrKeyboard) ||
               (key == vchrKeyboardAlpha) ||
               (key == vchrKeyboardNumeric)
            )
         )
      ) {
         readVirtualKeyboard(&event);

      }else if (!SysHandleEvent(&event)) {
         UInt16 error;
         if (event.eType == appStopEvent) {
            throw AppStopEventException();
         }
         if (!MenuHandleEvent(0, &event, &error)) {
            FrmDispatchEvent(&event);
         }
      }
   }
   int lenText = m_memo.GetTextLength() - m_startPos;
   if (lenText > 0) {
      sb.sputn(m_memo.GetTextPtr() + m_startPos, lenText);
   }
}

/*-----------------------------------------------------ConsoleFilter::produce-+
| Write to the console                                                        |
+----------------------------------------------------------------------------*/
int ConsoleFilter::produce(char const * pchBuf, int n)
{
   if (pchBuf && (n > 0)) { //  && (FrmGetActiveFormID() == m_formID)) {
      UInt16 maxLen = FldGetMaxChars(m_pField);
      if (n > maxLen - 1) n = maxLen - 1; // ensure enough room

      // If not enough room, lop off the top.
      while (FldGetTextLength(m_pField) + n >= maxLen) {
         UInt16 pos;
         RectangleType clipR, savedClipR;

         // Clip off everything so that field package doesn't
         // do a redraw during FldDelete()
         WinGetClip(&savedClipR);
         clipR = savedClipR;
         clipR.extent.x = clipR.extent.y = 0;
         WinSetClip(&clipR);

         // Lop off the top
         pos = FldGetInsPtPosition(m_pField);
         FldDelete(m_pField, 0, maxLen / 4);
         FldSetInsPtPosition(m_pField, pos);

         // Restore clip region.
         WinSetClip(&savedClipR);
      }

      // Insert the new text
      process(pchBuf, n);
   }
   return n;
}
/*-----------------------------------------ConsoleFilter::readVirtualKeyboard-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::readVirtualKeyboard(EventType* pEvent)
{
   // get the palmconsole's text handle
   MemHandle savedTextHandle = FldGetTextHandle(m_pField);
   
   // Palm OS doesn't lazy alloaction of the text handle of a displayed field,
   // so if it's empty then we'll just allocate one for it.
   if (!savedTextHandle) {
      savedTextHandle = MemHandleNew(1);
      if (!savedTextHandle) { throw std::bad_alloc(); }
      *(char *)MemHandleLock(savedTextHandle) = '\0';
      MemHandleUnlock(savedTextHandle);
   }
// UInt16 savedScrollPosition = FldGetScrollPosition(m_pField);

   // replace it so that the vkbd uses a fresh one
   UInt16 vkbdInsPtPosition = FldGetInsPtPosition(m_pField) - m_startPos;
   char * savedChars = (char *)MemHandleLock(savedTextHandle);
   MemHandle vkbdTextHandle = MemHandleNew(1+strlen(savedChars)-m_startPos);
   if (!vkbdTextHandle) { throw std::bad_alloc(); }
   strcpy((char *)MemHandleLock(vkbdTextHandle), savedChars+m_startPos);
   MemHandleUnlock(vkbdTextHandle);
   FldSetTextHandle(m_pField, vkbdTextHandle);
   FldSetInsPtPosition(m_pField, vkbdInsPtPosition);
   savedChars[m_startPos] = '\0';
   MemHandleUnlock(savedTextHandle);

   // start the vkbd
   SysHandleEvent(pEvent);

   // restore the original text handle
   // and append the vkbd chars to it
   // (careful not to use tempChars after FldSetTextHandle)
   int vkbdLen = FldGetTextLength(m_pField);
   vkbdInsPtPosition = FldGetInsPtPosition(m_pField);
   if (vkbdLen) {
      char * vkbdChars = FldGetTextPtr(m_pField);
      char * tempChars = (char *)MemPtrNew(vkbdLen);
      if (!tempChars) { throw std::bad_alloc(); }
      MemMove(tempChars, vkbdChars, vkbdLen);
      signalNewChars(tempChars, vkbdLen);
      FldSetTextHandle(m_pField, savedTextHandle);
      FldSetInsPtPosition(m_pField, m_startPos);
      //>>>PGR: No! use "produce"
      FldInsert(m_pField, tempChars, vkbdLen);
      MemPtrFree(tempChars);
   }else {
      FldSetTextHandle(m_pField, savedTextHandle);
   }
   FldSetInsPtPosition(m_pField, m_startPos + vkbdInsPtPosition);
// FldSetScrollPosition(m_pField, savedScrollPosition);
   MemHandleFree(vkbdTextHandle);
// FldDrawField(m_pField);  >>>PGR: I don't think we need this.
}

/*------------------------------------------------------ConsoleFilter::output-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::output(char const * pchBuf, int len)
{
   // Inserting a '00'x sends the Palm into a tizzy!
   int i,j;
   const char null = 0x01;  // 'd8'x is a null symbol, non-prtintables print boxes
   for(i=j=0; i<len; ++i) {
      if (pchBuf[i]=='\0') {                    // if it's a '00'x then
         FldInsert(m_pField, &pchBuf[j], i-j);  // flush then
         FldInsert(m_pField, &null, 1);         // output something else
         j=i+1;
      }
   }
   FldInsert(m_pField, &pchBuf[j], i-j);
};

/*-------------------------------------------------------ConsoleFilter::clear-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::clear() {
   FldDelete(m_pField, 0, FldGetTextLength(m_pField));
   m_startPos = 0;
}

/*---------------------------------------------------------ConsoleFilter::bel-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::bel() {
   SndPlaySystemSound(sndInfo);
}

/*STATIC-----------------------------------------ConsoleFilter::formatRcField-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleFilter::formatRcField(
   int rc, FormPtr pForm, UInt16 fieldId, int maxSize
) {
   MemStream rcAsString;
   rcAsString << "rc=" << rc;
   if (rcAsString.rdbuf()->in_avail() > maxSize) {
      rcAsString.seekp(maxSize);
      rcAsString << "...";
   }
   CField returnCodeField(/*pForm,*/ fieldId);
   returnCodeField.Replace(
      rcAsString.str(), rcAsString.rdbuf()->in_avail()
   );
   rcAsString.rdbuf()->freeze(0);
   return;
}

/*===========================================================================*/
