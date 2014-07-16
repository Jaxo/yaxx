#include <PalmOS.h>
#include "Rexxapp.h"

static const char * script = "dm:///MemoDB/Rexx/2";

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
   Err err;
   LocalID id;
   UInt32 type;
   UInt32 creator;
// EventType event;
   struct RexxAppLaunchRexxappParamType * r;  

    switch (cmd)
    {
    case sysAppLaunchCmdNormalLaunch:
      id = DmFindDatabase(0, "irexx");  
      err = DmDatabaseInfo(0, id, 0, 0, 0, 0, 0, 0, 0, 0, 0, &type, &creator);
      if (creator == 'Jaxo' && type == 'appl') {
         r = (RexxAppLaunchRexxappParamType *)MemPtrNew(sizeof(RexxAppLaunchRexxappParamType));
         MemPtrSetOwner(r, 0);
         r->iflags.defaultInput = RexxAppLaunchRexxappParamType::IO_CONSOLE;
         r->iflags.defaultOutput = RexxAppLaunchRexxappParamType::IO_CONSOLE;
         r->iflags.defaultError = RexxAppLaunchRexxappParamType::IO_CONSOLE;
         r->iflags.args = RexxAppLaunchRexxappParamType::ARGS_NONE;
         r->iflags.reservedForFutureUse = 0;
         r->script = (char *)MemPtrNew(100);
         MemPtrSetOwner(r->script, 0);
         StrCopy(r->script, script);
         r->args = 0;
         err = SysUIAppSwitch(0, id, rexxAppLaunchRexxapp, r);
      }
      /*
       do {
           EvtGetEvent(&event, evtWaitForever);
           if (!SysHandleEvent(&event))
               if (!MenuHandleEvent(0, &event, &err))
                    FrmDispatchEvent(&event);
       } while (event.eType != appStopEvent);
        break;
        */
    default:
        break;
    }

    return errNone;

}
