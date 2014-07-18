#include <PalmOS.h>

/* all code from here to end of file should use no global variables */
#pragma warn_a5_access on

#include "Rexxlet.h"

// NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO

// This is the Rexxlet launcher.
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

extern "C" {

// NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO

/* inline register routines */
static UInt32 GetA4(void) = 0x200C; /* move.l a4,d0 */
static inline void SetA4(UInt32 newValue) 
{
    asm { MOVE.L newValue, A4 } 
}		

static void copyAndPaste(const char * result)
{
	if (!(result && *result)) { return; }
	ClipboardAddItem(clipboardText, result, StrLen(result));
	
	// Should we even use this?
	//FieldType * activeField = MyFrmGetActiveField();
	// 18 Sep 2002
	// It appears that some apps, e.g., word, may or may not
	// use fields (since they offer wysiwyg editing not supported
	// with fields), but trying to FldPaste into them simply doesn't 
	// work.  So, there are only two options:  1. try to simulate
	// the invocation of their Paste command (have to obtain the
	// menu id of Paste in order to do this), or 2. simply 
	// process the clipboard contents as key strokes (note that
	// they cannot be pushed with EvtEnqueueKey because 
	// the event queue is not very large).

/*
extern UInt16 FrmGlueGetMenuBarID (const FormType *formP);
Get the menu set for the current form.
Then use the structures in Menu.h to interpret the menu resources.

#define MenuRscType							'MBAR'

MemHandle DmGetResource (MenuRscType, FrmGlueGetMenuBarID(FrmGetActiveForm()))
DmResID resID)
*/

   UInt16 l;
   EventType event;
   event.eType = keyDownEvent;
   event.data.keyDown.modifiers = 0;
   MemHandle h = ClipboardGetItem(clipboardText, &l);
   if (h && l) {
      char * p = (char *)MemHandleLock(h);
	   for (int i = 0; i < l; ++i) {
         event.data.keyDown.chr = (WChar)*p++;   
//         if (!SysHandleEvent(&event))
//            if (!MenuHandleEvent (0, &event, &err))
//               if (!ApplicationHandleEvent(&event))
                  FrmDispatchEvent(&event);	   
	      //EvtEnqueueKey((WChar)*p++, 0, 0); // NO! the queue is not big enough...
	   }
	   MemHandleUnlock(h);
	}
	
	return;
}						

Err Rexxlet(struct RexxAppLaunchRexxletParamType & r)
{
	Err err;
	LocalID id;
	UInt32 type;
	UInt32 creator;

	id = DmFindDatabase(0, "irexx");  // look for either the non-debug or
	if (id == 0) { goto notFound; }
	err = DmDatabaseInfo(0, id, 0, 0, 0, 0, 0, 0, 0, 0, 0, &type, &creator);
	if (err != errNone || !(creator == 'Jaxo' && type == 'appl')) { goto notFound; }

	UInt32 resultSysAppLaunch;
	FormActiveStateType formState;
		
	// save the current context
	FrmSaveActiveState(&formState);
	UInt32 a4 = GetA4(); 
	UInt32 a5 = SysSetA5(0);  // i think this is how to get the current value
	SysSetA5(a5);			// and then restore it after you clobbered it?
	
   //>>>JAL: FIXME we could create our own stack (r7) to run within?!

	
	// launch the rexxlet with a new stack (a7) and globals (a4/a5)
	err = SysAppLaunch(
		0, 
		id,
		sysAppLaunchFlagNewGlobals | sysAppLaunchFlagNewStack, 
		rexxAppLaunchRexxlet, //rexxAppNoop, 
		&r, &resultSysAppLaunch);	

	// restore the context
	SetA4(a4); 
	SysSetA5(a5); 	
	FrmRestoreActiveState(&formState);
	
	if (r.oflags.appLauncher) {
		//<<<FIXME JAL This doesn't seem to work.
		//             Have to find some related technique.
		// UPDATE:  I think I saw it work once, so maybe it does!
		EventType appEvent;
		appEvent.eType = keyDownEvent;
		appEvent.data.keyDown.chr = launchChr;
		appEvent.data.keyDown.modifiers = commandKeyMask;
		EvtAddEventToQueue(&appEvent);
		return errNone;
	}
		
	//<<<JAL FIXME refresh the form?  this does not work if there's a popup
	// How do i know if there's a popup?  Don't know.  The doc implies
	// that the formid may be invalid if there is no active form (e.g., a pup).
	// If I did know, then I could use WinSaveBits/WinRestoreBits to
	// save the whole screen.
	//FrmUpdateForm(FrmGetActiveFormID(), frmRedrawUpdateCode);
	//THIS DOES NOT SEEM TO BE THE RIGHT THING TO DO FOR CERTAIN APPS.
	//PERHAPS THE PROBLEM IS REALLY WITH THE LOAD FORM IN IREXX2!
	
	if (
		err == errNone &&	// SysAppLaunch itself was ok
		resultSysAppLaunch == 0 && // The Rexx app (not script) returned errNone
		r.rc == 0 && // The exit code (not RC) from interpret() was ok
		r.result // There exists a RESULT (i think always unless r.rc was not zero)
		) {
		if (r.iflags.copyAndPaste) { 
		   copyAndPaste((const char *)r.result); 
		}
	} else {
		//<<<JAL FIXME TODO indicate the error in a generic error box?
		// No, because the error should have been displayed to the user
		// in the console or in the rc.   
		SndPlaySystemSound(sndWarning);
	}
	if (r.result) {
	   MemPtrFree(r.result);
	}	
	return errNone;
		
notFound:
	// the rexx interpreter was not found
	//<<<JAL FIXME TODO do this in rexxlethack install, too
	// i think i need to manually get the form resource for the alert dialog
	//MemHandle h;
	//DmOpenRef dbhack = DmOpenDatabaseByTypeCreator('HACK' ???, CREATORID, dmModeReadOnly);
	//h = DmGet1Resource(type, id); 
	//DmCloseDatabase(dbhack);
	// -or- can i use SysFatalAlert?
	// -or- use WinDrawChars (have to figure out how to erase them after a timeout)
	// -or- use forget the visuals and just audio (-:
	//
	// THE best would be a WinDrawChars popup that stayed up for 2 secs.
	// See sample code for how to do this.
	// Probably need to draw a box, save/clear/restore under bits, etc.
	SndPlaySystemSound(sndError);
	return errNone;    
}

}
