/* $Id: IRexxApp2.cpp,v 1.13 2003/01/22 09:17:48 pgr Exp $ */

#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif

#include "Common.h"
#include "IRexxApp.h"
#include "ArgsForm.h"
#include "RexxletURLForm.h"
#include "EditRexxletForm.h"
#include "ConsoleRexxletForm.h"
#include "Rexxlet.h"
#include "Rexxapp.h"
#include "RexxletForms.h"
#include "BeamerStream.h"

#include <assert.h>

#include <typeinfo>

//<<<JAL TODO:  NOTE NOTE NOTE
//              THE IO_ OUTPUT TYPES OF THE LAUNCHERS HAVE NOT BEEN TESTED.
//              I DON'T KNOW IF IO_NULL AND IO_FILESTREAM WORK!!!

extern Rexx * m_rexx;
extern StringBuffer * m_args;
extern bool m_isRexxRunning;
extern BeamerStreamBuf * g_pBeamerStreamBuf;

extern iostream * m_script;

/* ------------------- */

#pragma pcrelconstdata on
static char const baseURL[] = "file://";
static char const dmScheme[] = "dm";
static char const fileScheme[] = "file";
static char const consoleScheme[] = "console";
static char const clipboardScheme[] = "clipboard";
static char const pathInput[] = "in";
static char const pathOutput[] = "out";
static char const pathError[] = "err";
static char const defaultScriptName[] = "<script>";
static char const defaultRexxletName[] = "<rexxlet>";
#pragma pcrelconstdata reset

/*-------------------------------------------------------CIRexxApp::interpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::interpret(RexxString & result, int & rc)
{
   rc = 0;
   bool isRexxlet = (m_wCommand == rexxAppLaunchRexxlet);

   /*
   |  IMPORTANT!  Avoid using the stack, since this is called
   |              when invoking a Rexxlet, which is executed
   |              in the Rexxlet caller's stack space.
   */


   // optimally configure the underlying buffer for the CodeBuffer
   // based on the amount of memory we have at our disposal
   // (or ultimately a config param!)
   streambuf * sb;
   if (queryMemory() < 50000) {
      sb = new PalmFileStreamBuf();
   
      //The  safe way doesn't work,
      //so we'll just do it the unsafe way that does work.
      //dynamic_cast<PalmFileStreamBuf *>(sb)->open("rxcode");
      ((PalmFileStreamBuf *)sb)->open("rxcode");

      sb->pubseekpos(0);
      sb->pubsetbuf(0, 100);
   } else {
      sb = new MemStreamBuf();
   }
   // the m_script is assumed to be set to read from its beginning, e.g., seekg(0)
   char const * scriptName = (isRexxlet) ? defaultRexxletName :defaultScriptName;
   Rexx::Script * script = new Rexx::Script(*m_script, scriptName, sb);

   /* the args (must be ASCIIZ if cast/used as ASCIIZ, i.e., const char *) */
   m_args->append('\0');

   /* do it THIS SHOULD BE THE ONLY PLACE WE CALL INTERPRET! */
   try {
      m_isRexxRunning = true;
      rc = m_rexx->interpret(*script, *m_args, result);
   }catch (CException & e) { // POL
      if (isRexxlet) {
         static const int maxMessageLength = 50;
         char * msg = new char[maxMessageLength];
         msg[0] = '\0';
         e.GetMessage(msg, maxMessageLength);
         CRexxletAlertForm::popup(*msg? msg : getRexxAppMessage(msgUserUnkownException));
         delete [] msg;
      }else {
         e.Display();
      }
      rc = e.GetCode();

   }catch (std::bad_alloc &) { // C++ OutOfMemory
      if (isRexxlet) {
         CRexxletAlertForm::popup(getRexxAppMessage(msgOutOfMemory));
      }else {
         FrmCustomAlert(ErrorAlert, getRexxAppMessage(msgOutOfMemory), 0, 0);
      }
      rc = -1;

   }catch (AppStopEventException) {
      /*
      | We could potentially do other things here,
      | like suspend the running script to be able to
      | restart it later.  Can you say "continuations?"
      | Have to be careful, though, what gets done here
      | because we cannot cause any events to be dispatched
      | since AppStop may have started to remove out app already.
      */
      m_isRexxRunning = false;
      sb->pubseekpos(0); // don't forget to clean up before we throw up...
      delete sb;
      delete script;
      rc = -1;
      throw; // let the top-level caller handle it (be it rexx or rexxlet)

   }catch (std::exception & e) { // C++
      if (isRexxlet) {
         CRexxletAlertForm::popup(e.what());
      }else {
         FrmCustomAlert(ErrorAlert, e.what(), 0, 0);
      }
      rc = -1;

   }catch (FileInEmptyException) { // io
      if (isRexxlet) {
         CRexxletAlertForm::popup(getRexxAppMessage(msgRexxIOException));
      }else {
         FrmCustomAlert(ErrorAlert, getRexxAppMessage(msgRexxIOException), 0, 0);
      }
      rc = -1;

   }catch (FileOutFullException) { // io
      if (isRexxlet) {
         CRexxletAlertForm::popup(getRexxAppMessage(msgRexxIOException));
      }else {
         FrmCustomAlert(ErrorAlert, getRexxAppMessage(msgRexxIOException), 0, 0);
      }
      rc = -1;

   }catch (URI::MalformedException & e) { // url
      if (isRexxlet) {
         CRexxletAlertForm::popup(getRexxAppMessage(msgMalformedURL));
      }else {
         FrmCustomAlert(ErrorAlert, getRexxAppMessage(msgMalformedURL), 0, 0);
      }
      rc = -1;

   }catch (...) {                  // Unknown
      if (isRexxlet) {
         CRexxletAlertForm::popup(getRexxAppMessage(msgRexxUnkownException));
      }else {
         FrmCustomAlert(ErrorAlert, getRexxAppMessage(msgRexxUnkownException), 0, 0);
      }
      rc = -1;
   }

   /* WATCH OUT ABOVE FOR AppStopEventException
      because that's the only place where we exit before getting here! */
   /* clean up */
   m_isRexxRunning = false;
   sb->pubseekpos(0);
   delete sb;
   delete script;
   m_args->empty();

   if (isRexxlet) {  // Set result and RC  !YUCK! should move this to app
      int len = result.length();
      char * psz = (char *)(((RexxAppLaunchRexxletParamType *)m_pCommandPBP)->result = MemPtrNew(len+1));
      memcpy(psz, (char const *)result, len);
      psz[len] = '\0';
      MemPtrSetOwner(psz, 0);
      ((RexxAppLaunchRexxletParamType *)m_pCommandPBP)->rc = rc;
   }

   return;
}

/*---------------------------------------------------CIRexxApp::RexxletLaunch-+
| Rexxlet launch handler                                                      |
+----------------------------------------------------------------------------*/
Err CIRexxApp::RexxletLaunch()
{

   // Be careful about things that happen in here.
   // We're launched in the context of another running app, i.e.,
   // its stack, among other resources.
   // Also note that the pointers in the launch structure come from
   // the launching application.
   //<<<JAL TODO: What about exceptions?  What if one is thrown but not caught here?
   //             Maybe we need to 'try' everything, just in case.
   //             In fact, rexxlet launching apps are not too keen
   //             so we have to be careful how we exit (e.g., making
   //             sure the "current" form is reset by the modal forms!)

   int rc;
   SystemContext * context;
   RexxString result;
   CRexxletURLForm * frmUrl = 0;
   RexxAppLaunchRexxletParamType * rexxletLaunchParam = (RexxAppLaunchRexxletParamType *)m_pCommandPBP;
   bool isPauseNeeded = rexxletLaunchParam->iflags.pauseWhenDone;
   bool isStopped = false;
   const char * scriptUrl = rexxletLaunchParam->script;

   if (!scriptUrl || scriptUrl[0] == '\0' || scriptUrl[0] == '?') {
      frmUrl = new CRexxletURLForm();
      if (frmUrl->DoModal())
      {
         scriptUrl = frmUrl->m_strUrlField;
      } else {
         rexxletLaunchParam->rc = -1; //<<<JAL -1?
         goto m_return;
      }
   }

   // Be a little extra cautious since a null URL causes an assertion
   // in the URI parser.  A blank one does, too, but if the users
   // wants to screw themselves...
   if (!scriptUrl || scriptUrl[0] == '\0') {
      rexxletLaunchParam->rc = -1;
      goto m_return;
   }

   try {
      RegisteredURI::registerScheme(PalmFileSchemeHandler());
      RegisteredURI uriBase(baseURL);
      FreeURI uri(scriptUrl, uriBase);

      //<<<JAL TODO: we should handle clipboard:in just like the rest.
      // in order to do so, we need a clipboard stream and stream handler.
      // it should probably be the simplest one...possibly a subclass of memstream.
      // console:in should not be a stream, tho, because it has to be
      // an interactive texte editor window.

      // console:in
      if (

         strcmp(uri.getScheme(), consoleScheme) == 0 &&
         strcmp(uri.getPath(), pathInput) == 0) {
         //we don't really need this anymore,
         //though we would then have the side-effect of
         //overwriting the temp script
         //m_script = new MemStream();
         //dynamic_cast<MemStream &>(*m_script) << "say hi; return \"123\"";
         CEditRexxletForm * frmEdit = new CEditRexxletForm();
         bool isOK = frmEdit->DoModal(RexxletEditForm);
         delete frmEdit;
         if (!isOK) {
            rexxletLaunchParam->rc = -1; //<<<JAL -1?
            goto m_return;
         }
      } else

      // clipboard:in
      if (
         strcmp(uri.getScheme(), clipboardScheme) == 0 &&
         strcmp(uri.getPath(), pathInput) == 0) {
         MemHandle h;
         UInt16 l;
         char * p;
         //see m_script comment above
         //m_script = new MemStream();
         if (!(
            (h = ClipboardGetItem(clipboardText, &l)) == 0 || l == 0 ||
            (p = (char *)MemHandleLock(h)) == 0)) {
            //dynamic_cast<MemStream *>(m_script)->write(p, l);
            setScript(p, l);
            MemHandleUnlock(h);
         }

      // dm:// or file://
      } else {  // this is the way they all should be done...
         // i just need the scheme handler for the specified scheme.
         // file: has been registered earlier to be able to use
         // as a base uri (bending over backwards for the sake of the user)
         if (strcmp(uri.getScheme(), dmScheme) == 0) {
            RegisteredURI::registerScheme(PalmDbmSchemeHandler());
         }
         if (m_script) { delete m_script; }
         //>>>PGR: don't think we need to pass 'uriBase'
         m_script = RegisteredURI(scriptUrl, uriBase).getStream(ios::in);
      }
   }catch (URI::MalformedException & e) {
      CRexxletAlertForm::popup(getRexxAppMessage(msgMalformedURL));
      rexxletLaunchParam->rc = -1;
      goto m_return;
   }

   m_args = new StringBuffer();
   switch (rexxletLaunchParam->iflags.args) {
   case RexxAppLaunchRexxletParamType::ARGS_CLIPBOARD:
   {
      MemHandle h;
      UInt16 l;
      char * p;
      if ((h = ClipboardGetItem(clipboardText, &l)) == 0 || l == 0 ||
         (p = (char *)MemHandleLock(h)) == 0) {
         break;
      }
      m_args->append(p, l);
      MemHandleUnlock(h);
      break;
   }
   case RexxAppLaunchRexxletParamType::ARGS_POPUP:
   {
      CArgsForm * frmArgs = new CArgsForm();
      if (frmArgs->DoModal()) {  //<<<JAL TODO: check to see if this really is ok.
                                 // I initially thought these kind of popups caused
                                 // problems for the calling app.  e.g., Alerts do.
         m_args->append(
            frmArgs->m_strArgsField,
            frmArgs->m_strArgsField.GetLength()
         );
      }
      delete frmArgs;
      break;
   }
   case RexxAppLaunchRexxletParamType::ARGS_SELECTION:
      if (rexxletLaunchParam->args) {
         m_args->append(rexxletLaunchParam->args, StrLen(rexxletLaunchParam->args));
      }
      break;
   case RexxAppLaunchRexxletParamType::ARGS_NONE:
   default:
      break;
   }

   switch (rexxletLaunchParam->iflags.defaultOutput) {
   case RexxAppLaunchRexxletParamType::IO_NULL:
      m_rexx = new Rexx();
      g_pBeamerStreamBuf = new BeamerStreamBuf();
      context = new SystemContext(
         baseURL,
         PalmFileSchemeHandler(),
         PalmDbmSchemeHandler(),
         SerialSchemeHandler(),
         BeamerSchemeHandler(),
         NetSchemeHandler(),
         MidiSchemeHandler()
      );
      try {
         interpret(result, rc);
      } catch (AppStopEventException) {
         isStopped = true;
      }
      delete context;
      break;

   case RexxAppLaunchRexxletParamType::IO_CONSOLE:
      m_rexx = new Rexx();
      g_pBeamerStreamBuf = new BeamerStreamBuf();
      CConsoleRexxletForm * frmConsole = new CConsoleRexxletForm();
      /* NOTE interpret() is called from within the form/dialog code.
      try {
         interpret(result, rc);
      } catch (...) {
      }
      */
//      try { we should NEVER exit this type of form with an exception
//            otherwise the currently active form (the one owned
//            by the launching app) will not be restored
//            exceptions of interest should be caught within the form.
         switch (frmConsole->DoModal(RexxletConsoleForm)) {
         case 2: isStopped = true;      break;
         case 1: isPauseNeeded = false; break;  // pause already done
         }
//      } catch (AppStopEventException) {
//         isStopped = true;
//         frmConsole->CloseForm();
//      }
      rc = rexxletLaunchParam->rc;
      delete frmConsole;
      break;

   case RexxAppLaunchRexxletParamType::IO_FILESTREAM:
      m_rexx = new Rexx();
      g_pBeamerStreamBuf = new BeamerStreamBuf();
      context = new SystemContext(
         baseURL,
         PalmFileSchemeHandler(),
         PalmDbmSchemeHandler(),
         SerialSchemeHandler(),
         BeamerSchemeHandler(),
         NetSchemeHandler(),
         MidiSchemeHandler(),
         PalmFileRedirectorSchemeHandler("")  //>>>JAL: FIXME
      );
      try {
         interpret(result, rc);
      } catch (AppStopEventException) {
         isStopped = true;
      }
      delete context;
      break;

   default:
      // assert(!"yikes");  // need a rexxlet-friendly assert
      break;
   }

   // NOTE:
   // This is bad.
   // Cannot use result/rc here because the console sets them as a side-effect.
   // They should probably be moved to the app and exist in only 1 spot.

   // If there was an AppStopEvent,
   // then tell the launcher about it and exit (without pausing).
   if (isStopped) {
      rexxletLaunchParam->oflags.appLauncher = true;
   } else if (isPauseNeeded) {
      CRexxletPauseForm::popup(rc);  // see JAL comments popup()
   }

m_return:
   delete frmUrl;
   return errNone;
}

/*---------------------------------------------------CIRexxApp::RexxappLaunch-+
| Rexxapp launch handler                                                      |
+----------------------------------------------------------------------------*/
Err CIRexxApp::RexxappLaunch()
{
   int rc;
   SystemContext * context;
   RexxString result;
   CRexxletURLForm frmUrl;
   RexxAppLaunchRexxappParamType * rexxappLaunchParam = (RexxAppLaunchRexxappParamType *)m_pCommandPBP;
   const char * scriptUrl = rexxappLaunchParam->script;

   NewGlobals();

   if (!scriptUrl || scriptUrl[0] == '\0' || scriptUrl[0] == '?') {
      if (frmUrl.DoModal())
      {
         scriptUrl = frmUrl.m_strUrlField;
      } else {
         goto m_return;
      }
   }

   // Be a little extra cautious since a null URL causes an assertion
   // in the URI parser.  A blank one does, too, but if the users
   // wants to screw themselves...
   if (!scriptUrl || scriptUrl[0] == '\0') {
      goto m_return;
   }

   try {
      RegisteredURI::registerScheme(PalmFileSchemeHandler());
      RegisteredURI uriBase(baseURL);
      FreeURI uri(scriptUrl, uriBase);

      //<<<JAL TODO: we should handle clipboard:in just like the rest.
      // in order to do so, we need a clipboard stream and stream handler.
      // it should probably be the simplest one...possibly a subclass of memstream.
      // console:in should not be a stream, tho, because it has to be
      // an interactive texte editor window.

      // console:in
      if (
         strcmp(uri.getScheme(), consoleScheme) == 0 &&
         strcmp(uri.getPath(), pathInput) == 0) {
         //see m_script comment above
         //m_script = new MemStream();
         //dynamic_cast<MemStream &>(*m_script) << "say hi; return \"123\"";
         CEditRexxletForm * frmEdit = new CEditRexxletForm();
         bool isOK = frmEdit->DoModal(RexxletEditForm);
         delete frmEdit;
         if (!isOK) {
            goto m_return;
         }
      } else

      // clipboard:in
      if (
         strcmp(uri.getScheme(), clipboardScheme) == 0 &&
         strcmp(uri.getPath(), pathInput) == 0) {
         MemHandle h;
         UInt16 l;
         char * p;
         //see m_script comment above
         //m_script = new MemStream();
         if (!(
            (h = ClipboardGetItem(clipboardText, &l)) == 0 || l == 0 ||
            (p = (char *)MemHandleLock(h)) == 0)) {
            //dynamic_cast<MemStream *>(m_script)->write(p, l);
            setScript(p, l);
            MemHandleUnlock(h);
         }

      // dm:// or file://
      } else {  // this is the way they all should be done...
         // i just need the scheme handler for the specified scheme.
         // file: has been registered earlier to be able to use
         // as a base uri (bending over backwards for the sake of the user)
         if (strcmp(uri.getScheme(), dmScheme) == 0) {
            RegisteredURI::registerScheme(PalmDbmSchemeHandler());
         }
         if (m_script) { delete m_script; }
         //>>>PGR: don't think we need to pass uriBase
         m_script = RegisteredURI(scriptUrl, uriBase).getStream(ios::in);
      }
   }catch (URI::MalformedException & e) {
      FrmCustomAlert(ErrorAlert, getRexxAppMessage(msgMalformedURL), 0, 0);
      goto m_return;
   }

   switch (rexxappLaunchParam->iflags.args) {
   case RexxAppLaunchRexxappParamType::ARGS_CLIPBOARD:
   {
      MemHandle h;
      UInt16 l;
      char * p;
      if ((h = ClipboardGetItem(clipboardText, &l)) == 0 || l == 0 ||
         (p = (char *)MemHandleLock(h)) == 0) {
         break;
      }
      m_args->append(p, l);
      MemHandleUnlock(h);
      break;
   }
   case RexxAppLaunchRexxappParamType::ARGS_POPUP:
   {
      CArgsForm * frmArgs = new CArgsForm();
      if (frmArgs->DoModal()) {  //<<<JAL TODO: check to see if this really is ok.
                                 // I initially thought these kind of popups caused
                                 // problems for the calling app.  e.g., Alerts do.
         m_args->append(
            frmArgs->m_strArgsField,
            frmArgs->m_strArgsField.GetLength()
         );
      }
      delete frmArgs;
      break;
   }
   case RexxAppLaunchRexxappParamType::ARGS_SELECTION:
      if (rexxappLaunchParam->args) {
         m_args->append(rexxappLaunchParam->args, StrLen(rexxappLaunchParam->args));
      }
      break;
   case RexxAppLaunchRexxappParamType::ARGS_NONE:
   default:
      break;
   }

   switch (rexxappLaunchParam->iflags.defaultOutput) {
   case RexxAppLaunchRexxappParamType::IO_NULL:
      context = new SystemContext(
         baseURL,
         PalmFileSchemeHandler(),
         PalmDbmSchemeHandler(),
         SerialSchemeHandler(),
         BeamerSchemeHandler(),
         NetSchemeHandler(),
         MidiSchemeHandler()
      );
      try {
         interpret(result, rc);
      } catch (AppStopEventException) {
      }
      delete context;
      break;

   case RexxAppLaunchRexxappParamType::IO_CONSOLE:
      BALR(0, ConsoleForm);
      return errNone;

   case RexxAppLaunchRexxappParamType::IO_FILESTREAM:
      context = new SystemContext(
         baseURL,
         PalmFileSchemeHandler(),
         PalmDbmSchemeHandler(),
         SerialSchemeHandler(),
         BeamerSchemeHandler(),
         NetSchemeHandler(),
         MidiSchemeHandler(),
         PalmFileRedirectorSchemeHandler("")  //>>>JAL: FIXME
      );
      try {
         interpret(result, rc);
      } catch (AppStopEventException) {
      }
      delete context;
      break;

   default:
      // assert(!"yikes");
      break;
   }

m_return:
   GotoLauncher();
   return errNone;
}

UInt32 CIRexxApp::queryMemory(/*UInt32 * pTotalMemory, UInt32 * pDynamicMemory*/)
{
    char * x = (char *)MemPtrNew(1);
   UInt32 freeBytes, maxChunk;
   MemHeapFreeBytes(MemPtrHeapID(x), &freeBytes, &maxChunk);
   MemPtrFree(x);
   return freeBytes;
/*
   const long memoryBlockSize = 1024L;
   UInt32 heapFree, maxChunk;
   Int16 cards;
   UInt16 cardNo, heapID;
   UInt32 freeMemory = 0;
   UInt32 totalMemory = 0;
   UInt32 dynamicMemory = 0;

   // Iterate through each card to support devices with multiple cards.
   // Iterate through the RAM heaps on a card (excludes ROM).
   // Obtain the ID of the heap.
   // Check if the heap is dynamic or not.
   cards = MemNumCards();     
   for (cardNo = 0; cardNo < cards; cardNo++) {
      for (int i = 0; i < MemNumRAMHeaps(cardNo); ++i) {
         heapID = MemHeapID(cardNo, i);
         if (MemHeapDynamic(heapID)) {
            dynamicMemory += MemHeapSize(heapID);
         } else {
            totalMemory += MemHeapSize(heapID);
            MemHeapFreeBytes(heapID, &heapFree, &maxChunk);
            freeMemory += heapFree;
         }
      }
   }
   // Reduce the stats to KB and round the results.
   freeMemory  = freeMemory / memoryBlockSize;
   totalMemory = totalMemory  / memoryBlockSize;
   dynamicMemory = dynamicMemory / memoryBlockSize;
   if (pTotalMemory) *pTotalMemory = totalMemory;
   if (pDynamicMemory) *pDynamicMemory = dynamicMemory;
   return freeMemory;
   */
}  

/*==========================================================================*/
