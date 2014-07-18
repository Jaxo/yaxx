/* $Id: main.cpp,v 1.2 2002/09/17 08:48:24 pgr Exp $ */

#include "screen.h"
#include "testsRsc.h"
#include "TestDbmRecStream.h"
#include "TestDbmByteStream.h"
#include "TestFileStream.h"
#include "TestSerialStream.h"
#include "TestNetStream.h"
#include "BeamerStream.h"

extern void testBeamerStream();
extern void testTime();

#define ourMinVersion    sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

BeamerStreamBuf * g_pBeamerStreamBuf = 0;
static Boolean MainFormHandleEvent(EventType * eventP);

/*--------------------------------------------------------MainFormHandleEvent-+
|                                                                             |
+----------------------------------------------------------------------------*/
static Boolean MainFormHandleEvent(EventPtr event)
{
   FormPtr frm = FrmGetActiveForm();
   FieldPtr field = (FieldPtr)FrmGetObjectPtr(
      frm,
      FrmGetObjectIndex(frm, MainConsoleField)
   );
   FieldAttrType attr;

   FldGetAttributes(field, &attr);
   attr.hasScrollBar = true;
   FldSetAttributes(field, &attr);

   switch(event->eType) {

   case frmOpenEvent:
      FrmDrawForm(frm);
      return true;

   case menuEvent:
      switch (event->data.menu.itemID) {
      case MainSerial:
         TestSerialStream().run();
         break;
      case MainNetTcpIP:
         TestNetStream().run();
         break;
      case MainBeamer:
         testBeamerStream();
         break;
      case MainTime:
         testTime();
         break;
      case MainDbmByteStream:
         TestDbmByteStream().run();
         break;
      case MainDbmRecStream:
         TestDbmRecStream().run();
         break;
      case MainFileStream:
         TestFileStream().run();
         break;
      default:
         break;
      }
      return true;

   case fldChangedEvent:
      updateScroll();
      return false;

   case sclRepeatEvent:
      linesScroll(
         event->data.sclRepeat.newValue - event->data.sclRepeat.value,
         false
      );
      return false;

   case keyDownEvent:
      if (event->data.keyDown.chr == pageUpChr) {
         pageScroll(winUp);
      }else if (event->data.keyDown.chr == pageDownChr) {
         pageScroll(winDown);
      }
      return false;

   default:
      return false;
   }
}

/*-------------------------------------------------------------AppHandleEvent-+
|                                                                             |
+----------------------------------------------------------------------------*/
static Boolean AppHandleEvent(EventType * eventP)
{
   UInt16 formId;
   FormType * frmP;

   if (eventP->eType == frmLoadEvent) {
      /* Load the form resource. */
      formId = eventP->data.frmLoad.formID;
      frmP = FrmInitForm(formId);
      FrmSetActiveForm(frmP);

      /*
      | Set the event handler for the form.  The handler of the
      | currently active form is called by FrmHandleEvent each
      | time is receives an event.
      */
      switch (formId) {
      case MainForm:
         FrmSetEventHandler(frmP, MainFormHandleEvent);
         break;
      default:
         break;
      }
      return true;
   }
   return false;
}

/*-------------------------------------------------------------------AppStart-+
|                                                                             |
+----------------------------------------------------------------------------*/
static Err AppStart()
{
   g_pBeamerStreamBuf = new BeamerStreamBuf();
   return errNone;
}

/*--------------------------------------------------------------------AppStop-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void AppStop()
{
   FrmCloseAllForms();
   delete g_pBeamerStreamBuf;
}

/*---------------------------------------------------------------AppEventLoop-+
| Description:                                                                |
|    This routine is the event loop for the application.                      |
+----------------------------------------------------------------------------*/
static void AppEventLoop()
{
   UInt16 error;
   EventType event;

   do {
      EvtGetEvent(&event, evtWaitForever);
      if (
         !SysHandleEvent(&event) &&
         !MenuHandleEvent(0, &event, &error) &&
         !AppHandleEvent(&event)
       ) {
         FrmDispatchEvent(&event);
      }
   }while (event.eType != appStopEvent);
}

/* all code from here to end of file should use no global variables */
// #pragma warn_a5_access on

/*-------------------------------------------------------RomVersionCompatible-+
|                                                                             |
+----------------------------------------------------------------------------*/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32 romVersion;

    /* See if we're on in minimum required version of the ROM or later. */
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion) {
       if (
          (launchFlags &
          (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
          (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)
       ) {
          FrmAlert(RomIncompatibleAlert);

          /*
          | Palm OS versions before 2.0 will continuously relaunch this
          | app unless we switch to another safe one.
          */
          if (romVersion < kPalmOS20Version) {
             AppLaunchWithCommand(
                sysFileCDefaultApp,
                sysAppLaunchCmdNormalLaunch,
                0
             );
          }
       }
       return sysErrRomIncompatible;
    }
    return errNone;
}

/*------------------------------------------------------------------PilotMain-+
| Description:                                                                |
|    This is the main entry point for the application.                        |
|                                                                             |
| Parameters:                                                                 |
|   cmd          word value specifying the launch code.                       |
|   cmdPB        pointer to the structure associated with the launch code     |
|   launchFlags  word value providing extra information about the launch.     |
|                                                                             |
| Returns:                                                                    |
|   Result of launch, errNone if all went OK                                  |
+----------------------------------------------------------------------------*/
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    Err error = RomVersionCompatible (ourMinVersion, launchFlags);
    if (error) return error;

    switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
       error = AppStart();
       if (error) {
          return error;
       }
       FrmGotoForm(MainForm);
       AppEventLoop();
       AppStop();
       break;

   // Exchange Manager
   case sysAppLaunchCmdSystemReset:
   case sysAppLaunchCmdSyncNotify: // sent when our app is installed
      //<<<JAL TODO: why do we register now?
      // don't we only want to register if rexx is running?
      // we cannot receive data if rexx isn't.
      ExgRegisterData('TEST', exgRegExtensionID, "RXT");
      break;

   case sysAppLaunchCmdExgAskUser:
      if (
         ((launchFlags & sysAppLaunchFlagSubCall)) &&
         (g_pBeamerStreamBuf) &&
         (g_pBeamerStreamBuf->isReceiving())
      ) {
         ((ExgAskParamPtr)cmdPBP)->result = exgAskOk;
      }else {
         ((ExgAskParamPtr)cmdPBP)->result = exgAskCancel;
      }
      break;

   case sysAppLaunchCmdExgReceiveData:
      if (
         ((launchFlags & sysAppLaunchFlagSubCall)) &&
         (g_pBeamerStreamBuf->isReceiving())
      ) {
         return g_pBeamerStreamBuf->receiveData((ExgSocketPtr)cmdPBP);
      }
      break;

    default:
       break;
    }
    return errNone;
}

/* turn a5 warning off to prevent it being set off by C++
 * static initializer code generation */
// #pragma warn_a5_access reset

/*===========================================================================*/
