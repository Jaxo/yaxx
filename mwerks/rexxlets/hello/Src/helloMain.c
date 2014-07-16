#include <PalmOS.h>

#include "Rexxlet.h"

// This is a minimal self-contained Rexxlet.
// It does not link in the Rexx interpreter
// because that would be prohibitive on the Palm.
// Instead, it uses Rexx as if it were an intrinsic part of the OS,
// sub-launching it to interpret a script and return a value.
// This is quite analogous to using it as a shared library on other systems,
// but closer to how AREXX was used on the Amiga...a more powerful model.
//
// Very generic, very simple, very extensible, very useful:
//
// 1. We could supply some standard generic Rexxlets.
//    E.g., a Rexxlet that invokes a referenced script and
//    takes its result and copies it to the clipboard and exits.
//    Peter Pircher's "simple adder" is an example of this,
//    albeit a canned one today since we cannot return values
//    from Rexx.interpret().
//
// 2. The few lines of code here could be added to any existing
//    Palm application and thus allow that Palm application to
//    use Rexx as a simple macro facility.  Yes, we would still
//    need to dela with issues like variable pools, signals, and
//    the many, many issues Rexx SAA had to deal with when
//    using Rexx as a macro language in some application,
//    but for doing something quick & simple & effective,
//    this is not bad.
//
// 3. A Rexx script that generates the binary code for the Rexxlet,
//    so that people could develop Rexxlets (like this one)
//    without needing a development environment.
//
// 4. I think there is something here that will lead to the
//    whole Palm API being available from within Rexx scripts.
//
// Issues:  return values, signals, variable pools, see Rexx SAA
//
//

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
   Err err;
   LocalID id;
   UInt32 type;
   UInt32 creator;
   UInt32 result;
// EventType event;
   struct RexxAppLaunchRexxletParamType r;  // do not initialize here.
      // careful not to do anything
      // before we're sure it's a sysAppLaunchCmdNormalLaunch

    switch (cmd)
    {
    case sysAppLaunchCmdNormalLaunch:

        // don't want to make it a class, to avoid c++ dependency
      r.flags.pauseWhenDone = false;
      r.flags.useConsole = true;
      r.flags.reservedForFutureUse = 0;
//    r.script = "return 'Bachi-bouzouk - Moules a gauffres'";
      r.script = "say \"Please enter numbers to add and press Enter when done:\"; parse pull n; r=0; do i=1 to words(n); if datatype(word(n,i))=\"NUM\" then r=r+word(n,i); end; return r";
//    r.script = "parse arg s; say s; say \"Press return to continue\"; parse pull pause; return 0";
      r.args = "hello";
      r.result = 0;

      id = DmFindDatabase(0, "irexx");  // look for either the non-debug or
      if (id == 0) { id = DmFindDatabase(0, "irexxD"); }  // debug versions
      err = DmDatabaseInfo(0, id, 0, 0, 0, 0, 0, 0, 0, 0, 0, &type, &creator);
      if (creator == 'Jaxo' && type == 'appl') {
         err = SysAppLaunch(
            0,
            id,
            sysAppLaunchFlagNewGlobals | sysAppLaunchFlagNewStack,
            rexxAppLaunchRexxlet,
            &r, &result);
      }
      // copy the result to the clipboard
      if (result == 0 && r.result) {
         ClipboardAddItem(clipboardText, r.result, StrLen(r.result));
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
