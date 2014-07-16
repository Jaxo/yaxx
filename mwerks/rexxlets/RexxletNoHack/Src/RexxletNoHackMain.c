
//
//TODO: Prevent it from running on older systems.
//      The normal launch supports this, but I don't think
//      the other launch codes do (and those are IMPORTANT).
//
//TODO: Should it be embedded in irexx?  If it is,
//      then we don't have to sublaunch, but we'll have to
//      set up as if we were sublaunched.
//
//TODO: Prefs!  Should we just put the prefs as irexx prefs
//      and have this read them from irexx?  This is moot if
//      we embed this in irexx.
// 

#include <PalmOS.h>

/* all code from here to end of file should use no global variables */
#pragma warn_a5_access on

#include "RexxletHackRsc.h"
#include "Rexxlet.h"

#define ourMinVersion    sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

extern "C" {

extern Boolean PreferencesHandler(EventPtr event);

//JAL
static void RegisterForNotifications()
{
   Err err;
   UInt16 cardNo;
   LocalID dbID;

   SysCurAppDatabase(&cardNo, &dbID);
   err = SysNotifyRegister(
      cardNo, dbID, 
      sysNotifyProcessPenStrokeEvent, (SysNotifyProcPtr)0, 
      sysNotifyNormalPriority, 0);
   return;
}

static void HandleNotification(SysNotifyParamType * np)
{
   SysNotifyPenStrokeType * ps = (SysNotifyPenStrokeType *)np->notifyDetailsP;
   UInt32 version = ps->version;
   bool handled = handleProcessSoftKeyStroke(&ps->startPt, &ps->endPt);
   //np->handled = handled;
   return;
}
//JAL

static Boolean AppHandleEvent(EventType * eventP)
{
    UInt16 formId;
    FormType * frmP;

    if (eventP->eType == frmLoadEvent)
    {
        /* Load the form resource. */
        formId = eventP->data.frmLoad.formID;
        frmP = FrmInitForm(formId);
        FrmSetActiveForm(frmP);

        /* Set the event handler for the form.  The handler of the
         * currently active form is called by FrmHandleEvent each
         * time is receives an event. */
        switch (formId)
        {
        case PreferencesForm:
            FrmSetEventHandler(frmP, PreferencesHandler);
            break;

        default:
            break;

        }
        return true;
    }

    return false;
}

static void AppEventLoop(void)
{
    UInt16 error;
    EventType event;

    do {
        /* change timeout if you need periodic nilEvents */
        EvtGetEvent(&event, evtWaitForever);

        if (! SysHandleEvent(&event))
        {
            if (! MenuHandleEvent(0, &event, &error))
            {
                if (! AppHandleEvent(&event))
                {
                    FrmDispatchEvent(&event);
                }
            }
        }
    } while (event.eType != appStopEvent);
}

static Err AppStart(void)
{
//    UInt16 prefsSize;

    /* Read the saved preferences / saved-state information. */
//    prefsSize = sizeof(RexxletNoHackPreferenceType);
//    if (PrefGetAppPreferences(
//        appFileCreator, appPrefID, &g_prefs, &prefsSize, true) != 
//        noPreferenceFound)
//    {
        /* FIXME: setup g_prefs with default values */
//    }
    
    return errNone;
}

static void AppStop(void)
{
    /* Write the saved preferences / saved-state information.  This
     * data will be saved during a HotSync backup. */
//    PrefSetAppPreferences(
//        appFileCreator, appPrefID, appPrefVersionNum, 
//        &g_prefs, sizeof(RexxletNoHackPreferenceType), true);
        
    /* Close all the open forms. */
    FrmCloseAllForms();
}


static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32 romVersion;

    /* See if we're on in minimum required version of the ROM or later. */
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion)
    {
        if ((launchFlags & 
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            FrmAlert(RomIncompatibleAlert);

            /* Palm OS versions before 2.0 will continuously relaunch this
             * app unless we switch to another safe one. */
            if (romVersion < kPalmOS20Version)
            {
                AppLaunchWithCommand(
                    sysFileCDefaultApp, 
                    sysAppLaunchCmdNormalLaunch, NULL);
            }
        }

        return sysErrRomIncompatible;
    }

    return errNone;
}

static UInt32 RexxletNoHackPalmMain(
    UInt16 cmd, 
    MemPtr cmdPBP, 
    UInt16 launchFlags)
{
    Err error;

    error = RomVersionCompatible (ourMinVersion, launchFlags);
    if (error) return (error);

    switch (cmd)
    {
    case sysAppLaunchCmdNormalLaunch:
        error = AppStart();
        if (error) 
            return error;

        /* start application by opening the main form
         * and then entering the main event loop */
        FrmGotoForm(PreferencesForm);
        AppEventLoop();
        
        AppStop();
        break;
        
   //JAL http://www.palmos.com/dev/support/docs/recipes/notifications.html
   /*
   Be very careful when registering for notifications on system resets. 
   If you have a bug in how you handle resets (like accessing global variables), 
   you may crash the Palm OS and force another reset, creating an infinite loop. 
   Many users won't know to do a no-notify reset so that they can delete your application.
      
   There is a special 'no-notify' reset (also referred to as a warm reset) 
   which you invoke by holding the UP button on the device and poking the 
   reset (i.e. it's like doing a hard reset except that you hold the UP 
   button instead of the power button).

   This type of reset will NOT send the reset message out to loaded 
   applications and it will NOT cause the loss of any data. At that point, 
   you can do a HotSync to make sure that all your current data is saved 
   before you proceed.

   When it comes back up, start removing applications--in an attempt to 
   find the culprit (i.e. remove say two applications, do a soft reset and 
   see if loops out again--if so, do another no-notify reset and try 
   removing more apps till you find it surviving a soft reset).
   */
   case sysAppLaunchCmdSystemReset:
   case sysAppLaunchCmdSyncNotify:  // sent when our app is installed
      RegisterForNotifications();
      break;
   case sysAppLaunchCmdNotify:
      HandleNotification((SysNotifyParamType *)cmdPBP);
      break;
   //JAL

    default:
        break;
    }

    return errNone;
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return RexxletNoHackPalmMain(cmd, cmdPBP, launchFlags);
}

}

