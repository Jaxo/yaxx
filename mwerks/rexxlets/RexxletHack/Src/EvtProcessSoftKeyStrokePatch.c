
#include <PalmOS.h>

/* all code from here to end of file should use no global variables */
#pragma warn_a5_access on

#include "RexxletHack.h"
#include "Rexxlet.h"

// See http://www.palmgear.com/software/showsoftware.cfm?sid=65362320020515050013&prodID=8774.
// That's the best launch extension.  If we could partner with it,
// then we wouldn't have to do *any* of this.  It's exactly
// what I want: defining a wide array of custom strokes
// corresponding to launching Rexxlets in different ways.
// This is a start towards doing the same,
// and completing it will probably take a couple weeks.

// How does this get bound to the trap?
// It's quite easy, if you know the structure of a code resource, PRC!
// The hard part is getting the tools you use (e.g., CW or PRC-TOOLS)
// to do it for you.  See the "68K Target" panel in the
// target settings for the particular trap's patch.

// This interface has to correspond to the Palm OS API
// that, in turn, corresponds to the system trap.
// In this case, it's sysTrapEvtProcessSoftKeyStroke and EvtProcessSoftKeyStroke.
typedef Err (*thisInterface)(PointType *, PointType *);

extern "C" {

// NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO GLOBALS NO

static Int16 
myStroke(PointType * start, PointType * end, RexxletHackPrefs * prefs)
{
   int i;
   UInt32 romVersion = 0;
   UInt16 n;
   PenBtnInfoPtr buttons;
   enum StrokeEnum from, to;
   enum StrokeEnum stroke;

   // The only way of doing things for older devices(?),
   // otherwise we should use the newer Palm APIs.
   RectangleType rApps; // = {{  0, 161}, { 24,  32}};
   RectangleType rMenu; // = {{  0, 191}, { 24,  32}};
   RectangleType rCalc; // = {{136, 161}, { 24,  32}};
   RectangleType rFind; // = {{136, 191}, { 24,  32}};
   RectangleType rGrff; // = {{ 27, 161}, {107,  60}};
   RectangleType rGrfA; // = {{ 27, 161}, { 70,  60}};
   RectangleType rGrfN; // = {{100, 161}, { 34,  60}};
   RectangleType rScrn; // = {{  0,   0}, {160, 160}};
   
   RctSetRectangle(&rApps,   0, 161,  24,  32);
   RctSetRectangle(&rMenu,   0, 191,  24,  32);
   RctSetRectangle(&rCalc, 136, 161,  24,  32);
   RctSetRectangle(&rFind, 136, 191,  24,  32);
   RctSetRectangle(&rGrff,  27, 161, 107,  60);
   RctSetRectangle(&rGrfA,  27, 161,  70,  60);
   RctSetRectangle(&rGrfN, 100, 161,  34,  60);
   RctSetRectangle(&rScrn,   0,   0, 160, 160);
   
   RectangleType * prApps = (RectangleType *)&rApps;
   RectangleType * prMenu = (RectangleType *)&rMenu;
   RectangleType * prCalc = (RectangleType *)&rCalc;
   RectangleType * prFind = (RectangleType *)&rFind;
   RectangleType * prGrff = (RectangleType *)&rGrff;
   RectangleType * prGrfA = (RectangleType *)&rGrfA;
   RectangleType * prGrfN = (RectangleType *)&rGrfN;
   RectangleType * prScrn = (RectangleType *)&rScrn;

   buttons = (PenBtnInfoPtr)EvtGetPenBtnList(&n);
   for (i = 0; i < n; ++i) {
		if (buttons[i].asciiCode == launchChr) prApps = &buttons[i].boundsR; else
      if (buttons[i].asciiCode == menuChr) prMenu = &buttons[i].boundsR; else
      if (buttons[i].asciiCode == calcChr) prCalc = &buttons[i].boundsR; else
      if (buttons[i].asciiCode == findChr) prFind = &buttons[i].boundsR;
   }

	if (RctPtInRectangle(start->x, start->y, prApps)) from = STROKEFROM_APPS; else
	if (RctPtInRectangle(start->x, start->y, prMenu)) from = STROKEFROM_MENU; else
	if (RctPtInRectangle(start->x, start->y, prCalc)) from = STROKEFROM_CALC; else
	if (RctPtInRectangle(start->x, start->y, prFind)) from = STROKEFROM_FIND;
	else return -1;

/*
	err = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion >= sysMakeROMVersion(3, 5, 0,sysROMStageRelease, 0)) {
		const SilkscreenAreaType * silkscreen = EvtGetSilkscreenAreaList(&n);
	   for (i = 0; i < n; ++i) {
	   	if (silkscreen[i].areaType == silkscreenRectGraffiti &&
	         silkscreen[i].index == alphaGraffitiSilkscreenArea) {
	         prGrfA = (RectangleType *)&silkscreen[i].bounds;
	      } else
	      if (silkscreen[i].areaType == silkscreenRectGraffiti &&
	         silkscreen[i].index == numericGraffitiSilkscreenArea) {
	         prGrfN = (RectangleType *)&silkscreen[i].bounds;
	      }
	   }
	   //<<<JAL TODO get extent of a & n for whole graffiti area
	   prGrff->left = (prGrfA->left < prGrfN->left)? prGrfA->left : prGrfN->left;
	   prGrff->top = (prGrfA->top < prGrfN->top)? prGrfA->top : prGrfN->top;
	}
*/
   
	if (RctPtInRectangle(end->x, end->y, prApps)) to = STROKETO_APPS; else
	if (RctPtInRectangle(end->x, end->y, prMenu)) to = STROKETO_MENU; else
	if (RctPtInRectangle(end->x, end->y, prCalc)) to = STROKETO_CALC; else
	if (RctPtInRectangle(end->x, end->y, prFind)) to = STROKETO_FIND; else
	if (RctPtInRectangle(end->x, end->y, prGrff)) to = STROKETO_GRAF; else
	if (RctPtInRectangle(end->x, end->y, prScrn)) to = STROKETO_SCRN;
	else return -1;

	stroke = (enum StrokeEnum)(from | to);
	for (i = 0; i < RexxletHackPrefs__numberOfStrokes(prefs); ++i) {
		if (stroke == RexxletHackPrefs__stroke(prefs, i)) {
			if (RexxletHackPrefs__strokeIsEnabled(prefs, i)) {
				return i;
			}
			return -1;
		}
	}
	return -1;
}

FieldType * MyFrmGetActiveField()
{
	Err err;
	UInt16 focus;
	FormType * activeForm;
	if (
		(activeForm = FrmGetActiveForm()) == 0 ||
		(focus = FrmGetFocus(activeForm)) == noFocus) {
		return 0;
	}
	FieldType * activeField = 0;
	switch (FrmGetObjectType(activeForm, focus)) {
	case frmFieldObj:
		UInt32 romVersion = 0;
		err = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
		activeField = (romVersion >= sysMakeROMVersion(4, 0, 0, sysROMStageRelease, 0))
			? FrmGetActiveField(0) : (FieldType *)FrmGetObjectPtr(activeForm, focus);
		break;
	case frmTableObj:
		TableType * activeTable = (TableType *)FrmGetObjectPtr(activeForm, focus);
		activeField = TblGetCurrentField(activeTable);
		break;
	default:
		break;
	}
	/* this might not work, eg., expense.
	    perhaps if the field is new, it has no text handle.
	if (activeField) {
		if (FldGetTextHandle(activeField) == 0) { return 0; }
		FieldAttrType a;
		FldGetAttributes(activeField, &a);
		if (!(a.usable && a.editable)) { return 0; }
	}
	*/
	return activeField;
}

bool handleProcessSoftKeyStroke(PointType * start, PointType * end)
{
   Err err;
   UInt32 romVersion;
   RexxletHackPrefs * prefs = 0;
   
   // For romVersion-specific behavior.
   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
   if (romVersion < sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0)) {
      goto passthru;
   }

   // Avoid re-entering, only because there's typically not enough
   // stack space to handle this.  If we did our own stack, then maybe...
   UInt32 isRexxletRunning = 0;
   err = FtrGet(CREATORID, FEATURE_ISREXXLETRUNNING, &isRexxletRunning);
   if (err == errNone && isRexxletRunning == 1) { goto passthru; }
   
   // Set prefs and check to see if this is our custom penstroke.
   // If not, pass it thru.
   prefs = RexxletHackPrefs__ctor();
   Int16 strokeIndex = myStroke(start, end, prefs);
   if (strokeIndex == -1) { goto passthru; }
   StrokeDescriptor * stroke = RexxletHackPrefs__strokeDescriptor(prefs, strokeIndex);

   // Set up the Rexxlet launch code's param type.
   struct RexxAppLaunchRexxletParamType r;
   MemSet((void *)&r, sizeof(struct RexxAppLaunchRexxletParamType), 0);
   r.iflags.pauseWhenDone = stroke->flags.pauseWhenDone;
   r.iflags.copyAndPaste = stroke->flags.copyAndPaste;  
   r.iflags.defaultInput = stroke->flags.defaultInput;
   r.iflags.defaultOutput = stroke->flags.defaultOutput;
   r.iflags.defaultError = stroke->flags.defaultError;  
   r.script = RexxletHackPrefs__url(prefs, strokeIndex + 1);
   // allow a null script...it will pop up the url dialog to prompt for one
   //   if (r.script == 0 || StrLen(r.script) == 0) {
   //      goto passthru;
   //   }
   r.iflags.args = stroke->flags.args;
   if (stroke->flags.args == RexxAppLaunchRexxletParamType::ARGS_SELECTION) {
      FieldType * activeField = MyFrmGetActiveField();
	   if (activeField) {
	      UInt16 startPosition, endPosition;
	      FldGetSelection(activeField, &startPosition, &endPosition);
	      Int32 len = endPosition - startPosition;
	      if (len > 0) {
	         char * p = FldGetTextPtr(activeField);
	         if (p) {
	            r.args = (char *)MemPtrNew(len + 1);
	            if (r.args) {
	               MemMove(r.args, &p[startPosition], len);
	               r.args[len] = '\0';
	            }
	         }
	      }
	   }
   }

   // no more literal scripts...could be done if we really wanted, e.g.,
   // not in the configuration (we don't want to store scripts in prefs),
   // but instead to allow the user to use the current selection as the script,
   // much in the same way we allow the clipboard, e.g., selection:   
   // r.script = "say \"This is the identity Rexxlet! Enter something to return, and press Enter when done:\"; parse pull x; return x";
   // r.script = "say \"Please enter numbers to add and press Enter when done:\"; parse pull n; r=0; do i=1 to words(n); if datatype(word(n,i))=\"NUM\" then r=r+word(n,i); end; return r";
   // r.script = "parse arg s; say s; return s";  // PGR idea:  use currently selected text!
   
   if (romVersion < sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0)) {
      EvtFlushNextPenStroke();
   }
   
   // Remember that a rexxlet is active because we're not
   // going to allow then to be "stacked" for now...not enough stack space!
	FtrSet(CREATORID, FEATURE_ISREXXLETRUNNING, 1);
   Rexxlet(r);
	FtrSet(CREATORID, FEATURE_ISREXXLETRUNNING, 0);
		
	if (r.args && stroke->flags.args == RexxAppLaunchRexxletParamType::ARGS_SELECTION) {
	   MemPtrFree(r.args);
	}
	if (prefs) { RexxletHackPrefs__dtor(prefs); }
	return true;
	
passthru:   
	if (prefs) { RexxletHackPrefs__dtor(prefs); }
	return false;
}

Err myEvtProcessSoftKeyStroke(PointType * start, PointType * end)
{
   // Get the old trap address set by the extensions manager,
   // e.g., HackMaster, X-Master, or TealMaster.
   // See http://www.daggerware.com/hackapi.htm and
   // http://www.linkesoft.com/english/xmaster/developer.html
   // for the system extension ("hack") protocol.
   UInt32 value;
   thisInterface oldTrap = 
      (FtrGet(CREATORID, TRAPID_PROCESSSOFTKEYSTROKE, &value) == errNone)? (thisInterface)value : 0;
   
   if (!handleProcessSoftKeyStroke(start, end)) { goto passthru; }
   return errNone;

passthru:
   return ((oldTrap) ? oldTrap(start, end) : errNone);
}

}