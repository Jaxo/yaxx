 
#include <PalmOS.h>

/* all code from here to end of file should use no global variables */
#pragma warn_a5_access on

#if __ide_target("NoHack Debug") || __ide_target("NoHack Release")
#define PREFS_DEFINE_ONCE
#endif

#include "RexxletHack.h"
#include "RexxletHackRsc.h"

extern "C" {

// Indices of values of items in hidden lists on preferences dialog.
#define LISTFROMINDEX_APPS 		0
#define LISTFROMINDEX_MENU 		1
#define LISTFROMINDEX_CALC 		2
#define LISTFROMINDEX_FIND 		3

#define LISTTOINDEX_GRAF 			0
#define LISTTOINDEX_SCRN 			1

#define LISTARGSINDEX_NONE      	0
#define LISTARGSINDEX_CLIPBOARD	1
#define LISTARGSINDEX_POPUP     	2
#define LISTARGSINDEX_SELECTION 	3

static void RexxletHackPrefs__initForm(RexxletHackPrefs * self);
static void RexxletHackPrefs__updateForm(RexxletHackPrefs * self);
static void RexxletHackPrefs__updateDescriptor(RexxletHackPrefs * self);
static void RexxletHackPrefs__setUrls4Config(RexxletHackPrefs * self);
static void RexxletHackPrefs__save(RexxletHackPrefs * self);

static UInt16 myStroke(RexxletHackPrefs * prefs);

static void * GetObjectPtr(UInt16 objID);

static void PrefsReturn()
{
#if __ide_target("NoHack Debug") || __ide_target("NoHack Release")
	EventType stopEvent; 
	MemSet(&stopEvent, sizeof(stopEvent), 0);
	stopEvent.eType = appStopEvent;
	EvtAddEventToQueue(&stopEvent); // return to the os
#else
   FrmGotoForm(9000);              // return to extension mgr
#endif
	return;
}

Boolean PreferencesHandler(EventPtr event);
Boolean PreferencesHandler(EventPtr event)
{
	Err err;
  	Boolean handled = false;
   RexxletHackPrefs * prefs;
  
	// Get the saved or default preferences,
	// save the pointer to them in feature memory for re-entry, and
	// initialize the dialog.  Why do we do this with feature memory?
	// It's a technique for implementing a global variable, 
	// because I think we cannot even use globals in the pref dialog!
	// I'm not sure if this is really true or not--I know it is
	// true of the extension's code segment--but for the preferences,
	// which run under the control of the extension mgr's app,
	// it seems like we should be able to use a global.
	//<<<JAL This is something to look into...ask X-Manager developers.
   err = FtrGet(CREATORID, FEATURE_CACHEDPREFS, (UInt32 *)&prefs);
   if (err != errNone) { prefs = 0; }
   
   // Create dynamic prefs, 
   // allocate handles for the URLs to map them to the display,
   // initialize the view of the form,
   // and set the global (and draw the form and tell the OS we handled it).
   if (event->eType == frmOpenEvent) {
   	// assert(prefs == 0);
      FrmDrawForm(FrmGetActiveForm());
   	prefs = RexxletHackPrefs__ctor();
   	RexxletHackPrefs__setUrls4Config(prefs);
      RexxletHackPrefs__initForm(prefs);
      FtrSet(CREATORID, FEATURE_CACHEDPREFS, (UInt32)prefs);
      return true;
   }
   
	switch (event->eType) {
	
	// We're done, so destroy the dynamic prefs,
	// unregister the "global," and exit.
   case frmCloseEvent:
   case appStopEvent:
   	if (prefs) { 
   		FldSetTextHandle((FieldPtr)GetObjectPtr(PreferencesURLField), 0);
   		RexxletHackPrefs__dtor(prefs); 
   	}
      FtrUnregister(CREATORID, FEATURE_CACHEDPREFS);
      handled = false;
      break;
               
   case popSelectEvent:
   	// Every time the user selects a new from or to,
   	// update the form with values for the selected pen stroke.
   	if (event->data.popSelect.controlID == PreferencesFromPopTrigger) {
   		if (!prefs) { goto internalError; }
			RexxletHackPrefs__updateForm(prefs);
		} else
   	if (event->data.popSelect.controlID == PreferencesToPopTrigger) {
   		if (!prefs) { goto internalError; }
			RexxletHackPrefs__updateForm(prefs);
		}
		
   	// Every time the user sets a value for the selected pen stroke,
   	// keep the dynamic prefs in sync.
   	if (event->data.popSelect.controlID == PreferencesArgsPopTrigger) {
   		if (!prefs) { goto internalError; }
			RexxletHackPrefs__updateDescriptor(prefs);
		}
		
		// Let the default GUI behavior do the work.
		handled = false;
   	break;
   	
   case ctlSelectEvent:
   	// These handle the command buttons.
   	
   	// OK will save all the values (dynamic prefs) in the prefs db 
   	// and exit back to the extension manager's well-known form.
   	if (event->data.ctlSelect.controlID == PreferencesOKButton) {
   		if (!prefs) { goto internalError; }
   		RexxletHackPrefs__save(prefs);
      	PrefsReturn();
      } else
      
   	// List will list all the values for the user to see them all  
   	// in a single view in a single dialog.
   	if (event->data.ctlSelect.controlID == PreferencesListButton) {
   		// TODO
   		if (!prefs) { goto internalError; }
      } else
      
   	// Defaults will set all the values (dynamic prefs)   
   	// back to their factory settings.  
   	if (event->data.ctlSelect.controlID == PreferencesDefaultsButton) {
	   	if (prefs) { 
	   		FldSetTextHandle((FieldPtr)GetObjectPtr(PreferencesURLField), 0);
	   		RexxletHackPrefs__dtor(prefs); 
	   	}
	      FtrUnregister(CREATORID, FEATURE_CACHEDPREFS);
   		prefs = RexxletHackPrefs__ctor(false);
   		RexxletHackPrefs__setUrls4Config(prefs);
      	RexxletHackPrefs__initForm(prefs);
      	FtrSet(CREATORID, FEATURE_CACHEDPREFS, (UInt32)prefs);
			RexxletHackPrefs__updateForm(prefs);
      } else
      
   	// Cancel exits without saving.  
   	if (event->data.ctlSelect.controlID == PreferencesCancelButton) {
      	PrefsReturn();
      } else

   	// Every time the user sets a value for the selected pen stroke,
   	// keep the dynamic prefs in sync.
   	if (event->data.ctlSelect.controlID == PreferencesEnabledCheckbox) {
   		if (!prefs) { goto internalError; }
			RexxletHackPrefs__updateDescriptor(prefs);
      } else
   	if (event->data.ctlSelect.controlID == PreferencesCopyPasteCheckbox) {
   		if (!prefs) { goto internalError; }
			RexxletHackPrefs__updateDescriptor(prefs);
      } else
   	if (event->data.ctlSelect.controlID == PreferencesPauseCheckbox) {
   		if (!prefs) { goto internalError; }
			RexxletHackPrefs__updateDescriptor(prefs);
      } 

		// Let the default GUI behavior do the work.
      handled = false;
      break;

   default:
      break;
  	}
  	return handled;

internalError:
	SndPlaySystemSound(sndError);   
   FtrUnregister(CREATORID, FEATURE_CACHEDPREFS);
	FrmCustomAlert(ErrorAlert, "Internal error. Please reconfigure RexxletHack preferences.", 0, 0);
   PrefsReturn();
  	return false;
}

static void RexxletHackPrefs__initForm(RexxletHackPrefs * self)
{
   LstSetSelection((ListPtr)GetObjectPtr(PreferencesFromList), LISTFROMINDEX_APPS);
   CtlSetLabel((ControlPtr)GetObjectPtr(PreferencesFromPopTrigger),
   	LstGetSelectionText((ListPtr)GetObjectPtr(PreferencesFromList), LISTFROMINDEX_APPS));
   LstSetSelection((ListPtr)GetObjectPtr(PreferencesToList), LISTTOINDEX_GRAF);
   CtlSetLabel((ControlPtr)GetObjectPtr(PreferencesToPopTrigger),
   	LstGetSelectionText((ListPtr)GetObjectPtr(PreferencesToList), LISTTOINDEX_GRAF));
	RexxletHackPrefs__updateForm(self);
  	return;
}

static void RexxletHackPrefs__updateForm(RexxletHackPrefs * self)
{
	UInt16 strokeIndex = myStroke(self);
	FldSetTextHandle((FieldPtr)GetObjectPtr(PreferencesURLField), 0);
	FldSetTextHandle((FieldPtr)GetObjectPtr(PreferencesURLField), self->m_urls4Config[strokeIndex]);
	FldDrawField((FieldPtr)GetObjectPtr(PreferencesURLField));
	StrokeDescriptor * s = RexxletHackPrefs__strokeDescriptor(self, strokeIndex);
	CtlSetValue((ControlPtr)GetObjectPtr(PreferencesEnabledCheckbox), s->isEnabled);
	CtlSetValue((ControlPtr)GetObjectPtr(PreferencesPauseCheckbox), s->flags.pauseWhenDone);
	CtlSetValue((ControlPtr)GetObjectPtr(PreferencesCopyPasteCheckbox), s->flags.copyAndPaste);
	switch (s->flags.args) {
	case RexxAppLaunchRexxletParamType::ARGS_NONE:
	   LstSetSelection((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_NONE);
	   CtlSetLabel((ControlPtr)GetObjectPtr(PreferencesArgsPopTrigger),
	   	LstGetSelectionText((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_NONE));
		break;
	case RexxAppLaunchRexxletParamType::ARGS_CLIPBOARD:
	   LstSetSelection((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_CLIPBOARD);
	   CtlSetLabel((ControlPtr)GetObjectPtr(PreferencesArgsPopTrigger),
	   	LstGetSelectionText((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_CLIPBOARD));
		break;
	case RexxAppLaunchRexxletParamType::ARGS_POPUP:
	   LstSetSelection((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_POPUP);
	   CtlSetLabel((ControlPtr)GetObjectPtr(PreferencesArgsPopTrigger),
	   	LstGetSelectionText((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_POPUP));
		break;   			
	case RexxAppLaunchRexxletParamType::ARGS_SELECTION:
	   LstSetSelection((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_SELECTION);
	   CtlSetLabel((ControlPtr)GetObjectPtr(PreferencesArgsPopTrigger),
	   	LstGetSelectionText((ListPtr)GetObjectPtr(PreferencesArgsList), LISTARGSINDEX_SELECTION));
		break;   			
	}
  	return;
}

static void RexxletHackPrefs__updateDescriptor(RexxletHackPrefs * self)
{
	UInt16 strokeIndex = myStroke(self);
	StrokeDescriptor * s = RexxletHackPrefs__strokeDescriptor(self, strokeIndex);
	s->isEnabled = CtlGetValue((ControlPtr)GetObjectPtr(PreferencesEnabledCheckbox));
	s->flags.pauseWhenDone = CtlGetValue((ControlPtr)GetObjectPtr(PreferencesPauseCheckbox));
	s->flags.copyAndPaste = CtlGetValue((ControlPtr)GetObjectPtr(PreferencesCopyPasteCheckbox));
	switch (LstGetSelection((ListPtr)GetObjectPtr(PreferencesArgsList))) {
	case LISTARGSINDEX_NONE:
		s->flags.args = RexxAppLaunchRexxletParamType::ARGS_NONE;
		break;
	case LISTARGSINDEX_CLIPBOARD:
		s->flags.args = RexxAppLaunchRexxletParamType::ARGS_CLIPBOARD;
		break;
	case LISTARGSINDEX_POPUP:
		s->flags.args = RexxAppLaunchRexxletParamType::ARGS_POPUP;
		break;   			
	case LISTARGSINDEX_SELECTION:
		s->flags.args = RexxAppLaunchRexxletParamType::ARGS_SELECTION;
		break;   			
	}
	return;
}

static void RexxletHackPrefs__save(RexxletHackPrefs * self)
{
	PrefSetAppPreferences(
		CREATORID, PREFS_FLAGSIDv1, CurrentPrefsVersion, 
		&self->m_descriptors, sizeof(StrokeDescriptors), true);
	
	char * url;
	UInt32 urlsSpace = 0;
	for (int i = 0; i < self->m_descriptors.numberOfStrokes; ++i) {
		url = (char *)MemHandleLock(self->m_urls4Config[i]);
		urlsSpace += StrLen(url) + 1;
		MemHandleUnlock(self->m_urls4Config[i]);
	}
	int written = 0;
	char * urls = (char *)MemPtrNew(urlsSpace);
	for (int i = 0; i < self->m_descriptors.numberOfStrokes; ++i) {
		url = (char *)MemHandleLock(self->m_urls4Config[i]);
		int size =  StrLen(url) + 1;
		MemMove(urls + written, url, size);
		written += size;
		MemHandleUnlock(self->m_urls4Config[i]);
	}
	PrefSetAppPreferences(
		CREATORID, PREFS_URLSIDv1, CurrentPrefsVersion, 
		urls, urlsSpace, true);
	MemPtrFree(urls);
	return;
}

static void RexxletHackPrefs__setUrls4Config(RexxletHackPrefs * self)
{
	self->m_urls4Config = (MemHandle *)MemPtrNew(sizeof(MemHandle) * self->m_descriptors.numberOfStrokes);
	for (int i = 0; i < self->m_descriptors.numberOfStrokes; ++i) {
		self->m_urls4Config[i] = MemHandleNew(MAXLENGTHURLv1);
		char * s1 = (char *)MemHandleLock(self->m_urls4Config[i]);
		char * s2 = RexxletHackPrefs__url(self, i + 1);
		StrCopy(s1, (s2)? s2 : "");
		MemHandleUnlock(self->m_urls4Config[i]);
	}
	return;
}

static UInt16 myStroke(RexxletHackPrefs * prefs)
{
	enum StrokeEnum from, to, stroke;
	
   Int16 indexFrom = LstGetSelection((ListPtr)GetObjectPtr(PreferencesFromList));
   Int16 indexTo = LstGetSelection((ListPtr)GetObjectPtr(PreferencesToList));
   
   switch (indexFrom) {
   case LISTFROMINDEX_APPS:
   	from = STROKEFROM_APPS;
   	break;
   case LISTFROMINDEX_MENU:
   	from = STROKEFROM_MENU;
   	break;
   case LISTFROMINDEX_CALC:
   	from = STROKEFROM_CALC;
   	break;
   case LISTFROMINDEX_FIND:
   	from = STROKEFROM_FIND;
   	break;   	
   }
   switch (indexTo) {
   case LISTTOINDEX_GRAF:
   	to = STROKETO_GRAF;
   	break;
   case LISTTOINDEX_SCRN:
   	to = STROKETO_SCRN;
   	break;
   }

	stroke = (enum StrokeEnum)(from | to);
	for (int i = 0; i < RexxletHackPrefs__numberOfStrokes(prefs); ++i) {
		if (stroke == RexxletHackPrefs__stroke(prefs, i)) {
			return i;
		}
	}
	return -1;
}

static void * GetObjectPtr(UInt16 objectID)
{
  	FormPtr frmP;
  
  	frmP = FrmGetActiveForm();
  	return (FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID)));
}

}