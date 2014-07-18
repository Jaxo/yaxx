/* $Id: IRexxApp.cpp,v 1.97 2002/12/13 21:23:05 jlatone Exp $ */

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
#include "PalmConsole.h"
#include "PalmFileStream.h"
#include "PalmDbmStream.h"
#include "BeamerStream.h"
#include "Interpreter.h"
#include "miscutil.h"

SysAppInfoPtr SysGetAppInfo(
   SysAppInfoPtr *rootAppPP,
   SysAppInfoPtr *actionCodeAppPP)
   SYS_TRAP(sysTrapSysGetAppInfo);

#include <assert.h>

/* -- LOCAL GLOBALS -- */
// These used to be part ofthe app class, but
// now that we're handling launch codes that
// (1) are sysAppLaunchFlagSubCall and
// (2) are not sysAppLaunchFlagNewGlobals
// then we have to take these off the stack and
// put them in the global area.
// They proably could have been made app statics,
// but this is effectively the same.  Anyway,
// since they were well encapsulated, I only
// had to cut&paste them here, though they
// should be renamed as g_'s.
Rexx * m_rexx;
StringBuffer * m_args;
bool m_isScriptDirty;
UInt16 m_scriptInsertPosition, m_scriptSrcollPosition;
UInt16 m_scriptSelect1Position, m_scriptSelect2Position;
UInt16 m_fromForm;
Int16 m_loadRow;
CDatabase * m_scriptIndexDB;
CDatabase * m_memoDB;
CDatabase * m_pedit32DB;
CDatabase * m_rexxDB;
UInt16 m_memoCategory;
UInt16 m_pedit32Category;
UInt16 m_rexxCategory;
bool m_isRexxRunning;
bool m_isScriptIndexDBInitialized;
BeamerStreamBuf * g_pBeamerStreamBuf;

// Trying to overload too much into this; it is working,
// but it's fragile because (1) the different type of streams
// this represents have different semantics, (2) trying to
// use a file stream as a memory-mapped file really doesn't work.
// So, we use it in a very special way here to get it to work.
// Caveat emptor to anyone who touches m_script!
iostream * m_script;
inline static void m_script__ctor(unsigned long opflags) // instance ctor!
{
   if (m_script) { delete m_script; }
   m_script = new PalmFileStream("rxscript", opflags, 0, CREATORID, 'data');
   return;
/*
   if (!m_script) {
      m_script = new PalmFileStream("script", opflags, 0, CREATORID, 'data');
      m_script->rdbuf()->pubsetbuf(0, 200);  //<<<JAL TODO should base this on mem avail
      return;
   }
   (dynamic_cast<PalmFileStreamBuf *>(m_script->rdbuf()))->close();
   (dynamic_cast<PalmFileStreamBuf *>(m_script->rdbuf()))->open(
      "script", opflags, 0, CREATORID, 'data');
*/
   return;
}

/* ------------------- */

/* -- EX-INLINES -- */
// Inlines tend to bloat the code,
// and increased code size results in the infamous 16-bit ref link error,
// so we try to avoid them

CDatabase * CIRexxApp::databaseOfScripts() {
   return m_scriptIndexDB;
}
UInt16 CIRexxApp::numberOfScripts() {
   return (m_scriptIndexDB) ? m_scriptIndexDB->NumRecords() : 0;
}
bool CIRexxApp::isScriptEmpty() {
   //jal-- return (dynamic_cast<MemStream *>(m_script)->pcount() == 0);
   m_script->seekp(0, ios::end);
   return (m_script->tellp() == 0);
}
bool CIRexxApp::isScriptDirty() {
   return m_isScriptDirty;
}
bool CIRexxApp::isScriptIndexDBInitialized() {
   return m_isScriptIndexDBInitialized;
}
void CIRexxApp::setLoadPosition(Int16 row) {
   if (row >= 0) {
      m_loadRow = row;
   }
}
Int16 CIRexxApp::getLoadPosition() {
   return m_loadRow;
}
void CIRexxApp::BALR(UInt16 r15, UInt16 r14) {  // ;-)
   m_fromForm = r15;
   CForm::GotoForm(r14);
}
void CIRexxApp::RET() {
   if (m_fromForm == 0) {
      GotoLauncher();
   } else {
      CForm::GotoForm(m_fromForm);
   }
}
bool CIRexxApp::isRexxletPauseWhenDone() {
   return ((RexxAppLaunchRexxletParamType *)m_pCommandPBP)->iflags.pauseWhenDone;
}
void CIRexxApp::titleOfScript(Int16 index, CString & title)
{
   ScriptRecord sr;
   CDBStream dbs(m_scriptIndexDB->QueryRecord(index));   
   dbs >> sr.m_title;
   title += sr.m_title;
   return;
}
void CIRexxApp::dbIndicatorOfScript(Int16 index, CString & dbi)
{
   ScriptRecord sr;
   CDBStream dbs(m_scriptIndexDB->QueryRecord(index));   
   dbs >> sr.m_title;
   dbs >> sr.m_dbi;
   dbi += sr.m_dbi;
   return;
}
void CIRexxApp::doAbout()
{
   char info[40];
   CForm frmAbout(AboutForm);
   CLabel(frmAbout, AboutVersionLabel).CopyLabel(Interpreter::getYaxxVersion());
 	StrPrintF(info, getRexxAppMessage(msgRexxInfo), (unsigned long)queryMemory());
   CLabel(frmAbout, AboutInfoLabel).CopyLabel(info);
   frmAbout.DoDialog();
   return;
}

/*----------------------------------------------CIRexxApp::PreSystemEventHook-+
| Called everywhere!  And, that's what's good about it because it will give   |
| us a change to handle exceptional events like AppStop that can cause        |
| problems depending on the context, e.g., pause on return in a Rexxlet.      |
+----------------------------------------------------------------------------*/
/*Boolean CIRexxApp::PreSystemEventHook(EventType *pEvent)
{
   return false;
   //bool isRexxlet = (m_instanceLaunchCode == rexxAppLaunchRexxlet);
   //switch (pEvent->eType) {
   //case appStopEvent:
   //   return false;
   //default:                    SAVING IT FOR A RAINY DAY
   //   return false;
   //}
}
*/

/*----------------------------------------------------CIRexxApp::RunEventLoop-+
| Override RunEventLoop because we also want to run the event loop under a    |
| "normal" launch (SysUIAppSwitch) that has our own launch code.              |
+----------------------------------------------------------------------------*/
Boolean CIRexxApp::RunEventLoop() {
   return 
      (m_wCommand == sysAppLaunchCmdNormalLaunch) || 
      (m_wCommand == rexxAppLaunchRexxapp) || 
      (m_wCommand == sysAppLaunchCmdGoTo && (m_wLaunchFlags & sysAppLaunchFlagNewGlobals));
}

/*----------------------------------------CIRexxApp::RegisterForNotifications-+
| Register for interest in the pen stroke event (5.0's version of a hack)     |
+----------------------------------------------------------------------------*/
void CIRexxApp::RegisterForNotifications()
{
   Err err;
   UInt16 cardNo;
   LocalID dbID;

   SysCurAppDatabase(&cardNo, &dbID);
   err = SysNotifyRegister(cardNo, dbID,
      sysNotifyProcessPenStrokeEvent, (SysNotifyProcPtr)0,
      sysNotifyNormalPriority, 0);
   return;
}

/*---------------------------------------------------------HandleNotification-+
| Handle the pen stroke event (5.0's version of a hack)                       |
+----------------------------------------------------------------------------*/
void CIRexxApp::HandleNotification(SysNotifyParamType * np)
{
   SysNotifyPenStrokeType * ps = (SysNotifyPenStrokeType *)np->notifyDetailsP;
   UInt32 version = ps->version;
   bool handled = handleProcessSoftKeyStroke(&ps->startPt, &ps->endPt);

   //np->handled = handled;
   return;
}

/*----------------------------------------------------CIRexxApp::InitInstance-+
| Called when application is invoked                                          |
+----------------------------------------------------------------------------*/
Err CIRexxApp::InitInstance()
{
   // true instance vars
   //none

   // really global vars
   // if we didn't get a new global segment for this instance,
   // then don't re-initialize the existing ones!
   if (m_wLaunchFlags & sysAppLaunchFlagNewGlobals) {
      m_rexx = 0;
      m_memoDB = 0;
      m_pedit32DB = 0;
      m_rexxDB = 0;
      m_scriptIndexDB = 0;
      m_memoCategory = dmRecNumCategories;
      m_pedit32Category = dmRecNumCategories;
      m_rexxCategory = dmRecNumCategories;
      m_script = 0;
      m_args = 0;
      m_isScriptDirty = false;
      m_scriptInsertPosition = m_scriptSrcollPosition =
      m_scriptSelect1Position = m_scriptSelect2Position = 0;
      m_fromForm = EditForm;
      m_loadRow = 0;
      m_isRexxRunning = false;
      m_isScriptIndexDBInitialized = false;
      g_pBeamerStreamBuf = 0;
   }

   // If Rexx is running, then this implies that we were launched
   // without a new set of globals and with sysAppLaunchFlagSubCall.
   // So, there are certain things we're not going to be able to do,
   // like a GoTo command because we cannot re-enter Rexx
   // while it's running unless we kill the running Rexx.
   // I'm not sure exactly why we cannot re-enter it,
   // but I do know it doesn't work and can look into
   // this later--I think it's a console issue.
   if (m_isRexxRunning) {  // handled below for individual commands
                           // but left here just for comments
   }

   // NOTE:  LOOK OUT BELOW...do not do anything that will cause a
   //        cross-segment call in non-global launches.  Hopefully,
   //        CW9 will allow cross-segment calls under non-normal launches.
   //        It's kind of a ridiculuous issue.  I.e., CW will allow you
   //        to set up globals (a5) for expanded mode, but there doesn't
   //        seem to be a way to initialize what else the app needs,
   //        e.g., the cross-segment jump table.

   UInt32 romVersion;
   switch(m_wCommand) {

   case sysAppLaunchCmdNormalLaunch:   // irexx ide
// enable this for easier testing of notify/rexxets on palm os 5
// the reason this is easier is that the way to cause this to
// happen on os 5 is to reset the device, which is a pain
// when testing (see call to RegisterForNotifications below)
// STATUS:  NOT YET WORKING.  NEED TO SEE PALM OS 5 STARTUP CODE
// TO SEE WHY REXX CANNOT FIND IT'S RESOURCES WHEN SELF-LAUNCHED
// UNDER THE NOTIFICATION HANDLER.  PERHAPS, CW_SETUP IS
// NOT DOING THE RIGHT THING SINCE IT'S A SUBCALL?
// LAUNCH CODES INFO I DISCOVERED FOR FUTURE REF:
//
// 32 when stroke is made (32 is undocumented)
//    32 = 0x20 = UNDOCUMENTED
// 166 when self-launched
//    166 = a6
//    x6 = 0x04|0x02 = sysAppLaunchFlagNewStack|sysAppLaunchFlagNewGlobals
//    Ax = 0x80|0x20 = sysAppLaunchFlagDataRelocated|UNDOCUMENTED
//
// SO, STILL USING SEPARATE NOHACK VERSON OF REXXLET LAUNCHER.
//RegisterForNotifications(); 
      return CPalmStApp<CIRexxApp>::InitInstance();

   case rexxAppNoop:                   // noop
      return errNone;
   case rexxAppLaunchRexxlet:          // rexxlet (sublaunch)
      return RexxletLaunch();
   case rexxAppLaunchRexxapp:          // rexxapp (uiappswitch)
      return RexxappLaunch();

   case sysAppLaunchCmdFind:
      return FindLaunch((FindParamsPtr)m_pCommandPBP);

   case sysAppLaunchCmdGoTo:
      // We cannot yet use Find when a rexxlet is running,
      // so considering that here is moot for now,
      // but if we could then we should not allow it.
      //<<<JAL Does this handle a waiting-on-return rexxlet console?
      if (m_isRexxRunning) {
         FrmCustomAlert(ErrorAlert, getRexxAppMessage(msgRexxIsRunning), 0, 0);
         return errNone;
      }
      // If we have new globals, then we weren't running (!sysAppLaunchFlagSubCall),
      // so start up as if in a normal launch and automatically run the script.
      // I know it's not obvious, but InitInstance will handle this
      // launch code as well and end up invoking GotoLaunch().
      // I could have placed GotoLaunch() here directly,
      // but might as well stick with POL.
      if (m_wLaunchFlags & sysAppLaunchFlagNewGlobals) {
         return CPalmStApp<CIRexxApp>::InitInstance();
      }
      // Finally, if we were running (sysAppLaunchFlagSubCall),
      // and the Rexx interpreter wasn't (!m_isRexxRunning),
      // then just load the script the user wanted and BALR.
      copyScriptFromFindResult((GoToParamsPtr)m_pCommandPBP);
      BALR(EditForm, ConsoleForm);
      return errNone;

   // The following is for Palm OS 5, and is not working correctly.
   //
   // http://www.palmos.com/dev/support/docs/recipes/notifications.html
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
   case sysAppLaunchCmdSyncNotify: // sent when our app is installed
      FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
      if (romVersion >= sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0)) {
//         RegisterForNotifications();
      }
      //<<<JAL TODO: why do we register now?
      // don't we only want to register if rexx is running?
      // we cannot receive data if rexx isn't.
      //Cannot make cross-segment calls and possibly calls that require globals
      //during non-normal launch codes.
      //BeamerStreamBuf::registerData();
      //we know we're at least palm os 3.0, so no need to check again
      ExgRegisterData(CREATORID, exgRegExtensionID, "REX");
      break;
      
   case sysAppLaunchCmdNotify:
      FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
      if (romVersion >= sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0)) {
         HandleNotification((SysNotifyParamType *)m_pCommandPBP);
      }
      break;
      
   // Exchange Manager
   case sysAppLaunchCmdExgAskUser:
      ((ExgAskParamPtr)m_pCommandPBP)->result = 
         (g_pBeamerStreamBuf && g_pBeamerStreamBuf->isReceiving())? exgAskOk : exgAskCancel;
      break;
   case sysAppLaunchCmdExgReceiveData:
      if (g_pBeamerStreamBuf && g_pBeamerStreamBuf->isReceiving()) {
         return g_pBeamerStreamBuf->receiveData((ExgSocketPtr)m_pCommandPBP);
      }
      break;

   default:
      return errNone;
   }
   return errNone;
}

/*----------------------------------------------------CIRexxApp::ExitInstance-+
|                                                                             |
+----------------------------------------------------------------------------*/
Err CIRexxApp::ExitInstance()
{
   // If we didn't get new globals for this instance,
   // then don't delete the existing ones!
   if (!(m_wLaunchFlags & sysAppLaunchFlagNewGlobals)) {
      return errNone;
   }

   // NOTE:  The reason we have to check-before-delete is because
   //        because we can now be called under other launch codes, e.g.,
   //        Find, and we're not allowed to make cross-segment calls in such
   //        a launch.  The delete of m_args, even though it's 0,
   //        causes a cross-segment call!  YOWZA!  Why?
   //        Not sure.  Perhaps it has a virtual dtor?  Go figure...
   if (m_memoDB) delete m_memoDB;
   if (m_pedit32DB) delete m_pedit32DB;
   if (m_rexxDB) delete m_rexxDB;
   if (m_scriptIndexDB) {
      UInt16 card;
      LocalID id;
      Err err1 = m_scriptIndexDB->GetCardNo(card);
      Err err2 = m_scriptIndexDB->GetDbUniqueID(id);
      bool doDelete = (err1 == errNone && err2 == errNone);
      delete m_scriptIndexDB;
      if (doDelete) DmDeleteDatabase(card, id);
   }
   if (m_script) { /*save it*/m_script->seekp(0, ios::end); delete m_script; }
   if (m_args) delete m_args;
   if (m_rexx) delete m_rexx;
   if (g_pBeamerStreamBuf) delete g_pBeamerStreamBuf;
   return errNone;
}

/*------------------------------------------------------CIRexxApp::NewGlobals-+
| Have to re-initialize when we've been given new globals for a new UI launch |
+----------------------------------------------------------------------------*/
void CIRexxApp::NewGlobals()
{
   m_rexx = new Rexx();
   m_script__ctor(fileModeUpdate); // open the last script we were editing 
   //jal-- m_script = new MemStream();
   m_args = new StringBuffer();
   g_pBeamerStreamBuf = new BeamerStreamBuf();
   return;
}

/*------------------------------------------------CIRexxApp::NewScriptIndexDB-+
| Load the scripts DB                                                         |
+----------------------------------------------------------------------------*/
void CIRexxApp::NewScriptIndexDB()
{
   //<<<JAL since searching for many scripts can be slow,
   //       we draw a little indicator at the start.
   //       palm os does provide a set of progress functions,
   //       but their progress dialog is too much for something simple.
   Err err;
   RectangleType saveFrame;
   WinHandle saveWindow = 0;
   if (WinGetDrawWindow()) { // If we can't draw yet, forget it!
      const char * s = getRexxAppMessage(msgRexxIsStarting); // the text in the rectangle
      const int wFrame = 2; // the width of dialogFrame
      const int padding = 2; // padding for the text in the rectangle
      Int16 w = FntCharsWidth(s, StrLen(s)) + (padding * 2);
      Int16 h = FntCharHeight() + (padding * 2);
      RectangleType r;
      RctSetRectangle(&r, (160-w)/2, (160-h)/2, w, h);
      RctSetRectangle(&saveFrame,
         r.topLeft.x-wFrame, r.topLeft.y-wFrame, r.extent.x+(wFrame*2), r.extent.y+(wFrame*2));
      saveWindow = WinSaveBits(&saveFrame, &err);
      WinDrawRectangleFrame(dialogFrame, &r);
      WinDrawChars(s, StrLen(s), r.topLeft.x+padding, r.topLeft.y+padding);
   }
   initializeIndexDatabase();
   if (saveWindow) {
      WinRestoreBits(saveWindow, saveFrame.topLeft.x, saveFrame.topLeft.y);
   }
   return;
}

/*----------------------------------------------------CIRexxApp::NormalLaunch-+
| Normal launch handler, called when application is invoked                   |
+----------------------------------------------------------------------------*/
Err CIRexxApp::NormalLaunch()
{
   NewGlobals();
   NewScriptIndexDB();
   CForm::GotoForm((numberOfScripts() == 0) ? EditForm : LoaditForm);
   return errNone;
}

/*------------------------------------------------------CIRexxApp::FindLaunch-+
|                                                                             |
+----------------------------------------------------------------------------*/
Err CIRexxApp::FindLaunch(FindParamsPtr pFindParams)
{
   //<<<JAL TODO: This is currently dependent on the Rexx category.
   //             I'm not sure if we'll ever need to change/add-to this,
   //             but if we do, then this code will have to be changed
   //             along with the GoTo command.
   Err err;
   LocalID dbID;
   UInt16 cardNo = 0;
   DmOpenRef dbP;
   DmSearchStateType searchState;
   UInt16 recordNum;
   MemHandle hRecord;
   UInt32 pos;
   UInt16 matchLength;
   Boolean match, full;
   RectangleType r;
   UInt32 type;
   UInt32 creator;

   // Open our database (should we search MemoPad and pedit, too?)
   // and do our Find.  We define the semantics of Find, so
   // instead of searching the whole records for the search string,
   // let's just search for scripts with the search string as their "name."
   if (FindDrawHeader(pFindParams, "Rexx Scripts")) {
      goto m_return;
   }
   if ((err = DmGetNextDatabaseByTypeCreator(
       true, &searchState, 'data', CREATORID, true, &cardNo, &dbID)) != errNone) {
      pFindParams->more = false;
      return errNone;
   }
   if ((err = DmDatabaseInfo(0, dbID, 0, 0, 0, 0, 0, 0, 0, 0, 0, &type, &creator)) != errNone ||
      (type != 'data' && creator != CREATORID)) {
      pFindParams->more = false;
      return errNone;
   }
   if ((dbP = DmOpenDatabase(cardNo, dbID, pFindParams->dbAccesMode)) == 0 || 
      DmGetAppInfoID(dbP) == 0) { /* if categories not initialized then CategoryGetName throws fatal error */ 
      pFindParams->more = false;
      return errNone;
   }
   UInt16 category;
   char categoryName[dmCategoryLength];
   for (category = 0; category < dmRecNumCategories; ++category) {
       CategoryGetName(dbP, category, categoryName);
       if (!StrCaselessCompare(categoryName, "REXX")) { break; }
   }
   if (category == dmRecNumCategories) { goto m_return; }
   // set it to dmAllCategories?

   UInt32 romVersion;
   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

   full = false;
   recordNum = pFindParams->recordNum;
   while (true) {

      // Because applications can take a long time to finish a Find when
      // the result may already be on the screen, or for other reasons,
      // users like to be able to stop the Find.  So, stop it if any event
      // is pending, i.e., if the user does something with the device.
      // Because actually checking if an event is pending slows the
      // search itself, just check it every so many records.
      if ((recordNum & 0x000f) == 0 && EvtSysEventAvail(true)) {
         pFindParams->more = true;
         break;
      }
      if (!(hRecord = DmQueryNextInCategory(dbP, &recordNum, category))) {
         pFindParams->more = false;
         break;
      }

      Char * p = (char *)MemHandleLock(hRecord);
      UInt32 isInternational;
      err = FtrGet(sysFtrCreator, sysFtrNumIntlMgr, &isInternational);
      if (err == errNone && isInternational) {
         match = TxtFindString(p, pFindParams->strToFind, &pos, &matchLength);
      } else {
         match = TxtGlueFindString(p, pFindParams->strToFind, &pos, &matchLength);
      }
      if (match) {
         // Add the match to the find paramter block.
         // If there is no room to display the match
         // then the following function will return true.
         full = FindSaveMatch(pFindParams, recordNum, (UInt16)pos, 0, 0, cardNo, dbID);
         if (!full) {
            // Get the bounds of the region where we will draw the results, and
            // display the title of the description neatly in that area.
            FindGetLineBounds(pFindParams, &r);
            Int16 x = r.topLeft.x + 1;
            Int16 y = r.topLeft.y;
            Int16 w = r.extent.x - 2;
            Char * cr = StrChr(p, linefeedChr);
            UInt16 titleLen = (cr == 0)? StrLen(p) : cr - p;
            Int16 fntWidthToOffset;
            if (romVersion >= sysMakeROMVersion(3, 1, 0, sysROMStageRelease, 0)) {
               fntWidthToOffset = FntWidthToOffset(p, titleLen, w, 0, 0);
            } else {
               fntWidthToOffset = FntGlueWidthToOffset(p, titleLen, w, 0, 0);
            }
            if (fntWidthToOffset == titleLen) {
               WinDrawChars(p, titleLen, x, y);
            } else {
               Int16 titleWidth;
               titleLen = FntWidthToOffset(p, titleLen, w - FntCharWidth(chrEllipsis), 0, &titleWidth);
               WinDrawChars(p, titleLen, x, y);
               WinDrawChar (chrEllipsis, x + titleWidth, y);
            }
            ++pFindParams->lineNumber;
         }
      }
      MemHandleUnlock(hRecord);
      if (full) { break; }
      ++recordNum;
   }

m_return:
   DmCloseDatabase(dbP);
   return errNone;
}

/*------------------------------------------------------CIRexxApp::GotoLaunch-+
| Goto launch handler, called when a Find record was selected                 |
+----------------------------------------------------------------------------*/
Err CIRexxApp::GotoLaunch(GoToParamsPtr pGoToParams)
{
   NewGlobals();
   // NewScriptIndexDB(); defer the loading of the script db for quicker/smarter startup
   copyScriptFromFindResult(pGoToParams);
   BALR(EditForm, ConsoleForm);
   return errNone;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*-----------------------------------------CIRexxApp::initializeIndexDatabase-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::initializeIndexDatabase()
{
   m_isScriptIndexDBInitialized = true;

   Err err;
   char categoryName[dmCategoryLength];

   // Open MemoPad's DB as a source for scripts.
   m_memoDB = new CDatabase(
      0, "MemoDB", sysFileCMemo, 'data', false, false, dmModeReadOnly
   );
   for (m_memoCategory = 0; m_memoCategory < dmRecNumCategories; ++m_memoCategory) {
       CategoryGetName(m_memoDB->GetDatabasePtr(), m_memoCategory, categoryName);
       if (!StrCaselessCompare(categoryName, "REXX")) { break; }
   }
   if (m_memoCategory == dmRecNumCategories ||
       m_memoDB->NumRecordsInCategory(m_memoCategory) == 0) {
       delete m_memoDB;
       m_memoDB = 0;
   }

   //<<<JAL TODO:  I think CategoryGetName() will core dump
   // if the db has no categories.  So, we should probably
   // check the db attrs before doing this.


   // Open pedit's DB as a source for scripts.
   // Since this DB may not exist (unlike MmeoPad's)
   // then Open() or CategoryGetName() may throw an exception
   // that we'll have to handle properly.
   try {
      m_pedit32DB = new CDatabase();
      err = m_pedit32DB->Open(0, "Memo32DB", dmModeReadOnly);
      for (m_pedit32Category = 0; m_pedit32Category < dmRecNumCategories; ++m_pedit32Category) {
          CategoryGetName(m_pedit32DB->GetDatabasePtr(), m_pedit32Category, categoryName);
          if (!StrCaselessCompare(categoryName, "REXX")) { break; }
      }
      if (m_pedit32Category == dmRecNumCategories ||
          m_pedit32DB->NumRecordsInCategory(m_pedit32Category) == 0) {
         delete m_pedit32DB;
         m_pedit32DB = 0;
      }
   } catch (...) {
       delete m_pedit32DB;
       m_pedit32DB = 0;
   }

   // Open our own DB as a source for scripts.
   try {
      m_rexxDB = new CDatabase();
      err = m_rexxDB->Open(0, "RexxDB", dmModeReadOnly);
      for (m_rexxCategory = 0; m_rexxCategory < dmRecNumCategories; ++m_rexxCategory) {
          CategoryGetName(m_rexxDB->GetDatabasePtr(), m_rexxCategory, categoryName);
          if (!StrCaselessCompare(categoryName, "REXX")) { break; }
      }
      if (m_rexxCategory == dmRecNumCategories ||
          m_rexxDB->NumRecordsInCategory(m_rexxCategory) == 0) {
         delete m_rexxDB;
         m_rexxDB = 0;
      }
   } catch (...) {
       delete m_rexxDB;
       m_rexxDB = 0;
   }

   if (m_memoDB == 0 && m_pedit32DB == 0 && m_rexxDB == 0) { return; }

   // Create an index and then index each database so that we can
   // serialize the index into a actual POL database for grid mapping.
   CArray<ScriptRecord *> records;

   addRecordsToIndex(records, m_pedit32DB, m_pedit32Category, "P");
   addRecordsToIndex(records, m_memoDB, m_memoCategory, "M");
   addRecordsToIndex(records, m_rexxDB, m_rexxCategory, "R");

   m_scriptIndexDB = new CDatabase(0, "RexxScriptIndexDB", CREATORID, 'temp');
   m_scriptIndexDB->RemoveAllRecords();
   for (int i = 0; i < records.GetCount(); ++i) {
      ScriptRecord * sr = records[i];
      CRecordStream rs(m_scriptIndexDB);
      rs << sr->m_title;  //<<<JAL TODO:  YUCK!  MOVE ACCESS TO ScriptRecord CLASS!
      rs << sr->m_dbi;
      rs << (UInt32)sr->m_db;
      rs << sr->m_indexes.GetCount();
      for (int j = 0; j < sr->m_indexes.GetCount(); ++j) {
         rs << sr->m_indexes[j] << sr->m_segments[j];
      }
   }
   for (int i = 0; i < records.GetCount(); ++i) {
      delete records[i];
   }
   return;
}

/*-----------------------------------------------CIRexxApp::addRecordsToIndex-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::addRecordsToIndex(CArray<ScriptRecord *> & records, CDatabase * db, UInt16 cat, char * dbi)
{
   UInt16 index = 0;
   UInt32 rowId;
   MemHandle hMem;
   Int16 recordPosition = 0;
   CHash<unsigned int, Int16> recordPositionBySegmentId;
   ScriptRecord * scriptRecord;

   if (db == 0) { return; }
   while ((hMem = db->QueryNextInCategory(index, cat))) {
      db->GetRecordUniqueID(index, rowId);
      UInt32 size = MemHandleSize(hMem);
      const char * pMem = (char *)MemHandleLock(hMem);
      /*
      | FORMAT:
      | %6s #segment.<no>#\ndate() time() <id>\nTitle: <tit>\nCategory: <cat>
      */
      char * scanner;
      if (                    // if it's segmented, combine the segments
         (scanner = StrStr(pMem, "#segment.")) &&
         (scanner = StrChr(scanner + 1, '#'))
      ) {
         char segmentNoAsString[5];
         memcpy(segmentNoAsString, scanner - 4, 4);
         segmentNoAsString[4] = '\0';
         int segmentNo = StrAToI(segmentNoAsString);
         ++scanner;
         int peditHeaderSize = size - (scanner - pMem);
         if (peditHeaderSize > ScriptRecord::MAX_PEDITHEADERLEN) {
            peditHeaderSize = ScriptRecord::MAX_PEDITHEADERLEN;
         }
         RexxString header(scanner, peditHeaderSize);
         RexxString segmentIdAsString;
         segmentIdAsString.wordAt(header, 3);
         unsigned int segmentId = hex2uint(
            (char const *)segmentIdAsString, segmentIdAsString.length()
         );
         int titWordNo = RexxString("Title:").wordpos(header, 1);
         int catWordNo = RexxString("Category:").wordpos(header, 1);
         RexxString title;
         title.subword(header, titWordNo + 1, catWordNo - (titWordNo + 1));
         // create the script record
         Int16 * psegmentedRecordPosition = (Int16 *)recordPositionBySegmentId.Lookup(segmentId);
         // if this segment has already been encountered, then create a chain of segments
         if (psegmentedRecordPosition) {
            ScriptRecord * sr = records[*psegmentedRecordPosition];
            sr->m_indexes.Insert(sr->m_indexes.GetCount(), index);
            sr->m_segments.Insert(sr->m_segments.GetCount(), segmentNo);
         // otherwise just add it
         }else {
            scriptRecord = new ScriptRecord(db, dbi, title, index, segmentNo);
            recordPositionBySegmentId.SetAt(segmentId, recordPosition);
            records.Insert(recordPosition++, scriptRecord);
         }
      }else { // otherwise just add it
         unsigned int i;
         for (i=0; pMem[i] && pMem[i] != linefeedChr && i < ScriptRecord::MAX_TITLELEN; ++i) {
            ;
         }
         char * t = new char[i + 1];
         memcpy(t, pMem, i);
         t[i] = '\0';
         RexxString title(t);
         scriptRecord = new ScriptRecord(db, dbi, title, index);
         delete [] t;
         records.Insert(recordPosition++, scriptRecord);
      }
      MemHandleUnlock(hMem);
      ++index;
   }
}

/*-------------------------------------CIRexxApp::copyScriptFromIndexDatabase-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::copyScriptFromIndexDatabase(Int16 index)
{
   Int16 count;
   UInt32 dbptr;
   ScriptRecord sr;
   CDBStream dbs(m_scriptIndexDB->QueryRecord(index));   
   dbs >> sr.m_title;
   dbs >> sr.m_dbi;
   dbs >> dbptr;
   sr.m_db = (CDatabase *)dbptr;
   dbs >> count;
   sr.m_indexes.EnsureSize(count);
   sr.m_segments.EnsureSize(count);
   for (int i = 0; i < count; ++i) {
      Int16 value1, value2;
      dbs >> value1 >> value2;
      sr.m_indexes.Insert(i, value1);
      sr.m_segments.Insert(i, value2);
   }

   emptyScript();
   bool isSegmented = count > 1;
   for (Int16 i = 0; i < count; ++i) {
      MemHandle hMem = sr.m_db->QueryRecord(sr.m_indexes[i]);
      char * script = (char *)MemHandleLock(hMem);
      UInt32 size = MemHandleSize(hMem);
      if (isSegmented) {
         char * realScript = strstr(
            script, "!DO NOT TOUCH THIS LINE OR ABOVE!"
         );
         if (realScript) {
            realScript += 34;
            size -= (realScript - script);
            script = realScript;
         }
      }
      if (script[size - 1] == '\0') { --size; }
      appendScript(script, size);
      MemHandleUnlock(hMem);
   }
   return;
}

/*-------------------------------------------------------CIRexxApp::setScript-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::setScript(const char * string, UInt32 size)
{
   // create a new script to edit 
   emptyScript();
   m_script->write(string, size);  
   //--jal
   //MemStream * script = dynamic_cast<MemStream *>(m_script);
   //script->reset();
   //script->write(string, size);
   //--jal
   m_isScriptDirty = true;
   return;
}

/*----------------------------------------------------CIRexxApp::appendScript-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::appendScript(const char * string, UInt32 size)
{
   // so append to the current script (e.g., a segmented script)
   m_script__ctor(fileModeUpdate);
   m_script->seekp(0, ios::end);  
   m_script->write(string, size);  
   //jal-- MemStream * script = dynamic_cast<MemStream *>(m_script);
   //jal-- script->write(string, size);
   m_isScriptDirty = true;
   return;
}

/*------------------------------------------------------CIRexxApp::copyScript-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::copyScriptToMemo(CMemo & memo)
{
   // copy the current script to the field (3 copies!!!  YUCK!)
   //<<< JAL FIXME
   m_script__ctor(fileModeUpdate);   
   StringBuffer * sb = new StringBuffer(100); // temp to convert it to a char * for gui
   try {
   	  sb->append(*m_script->rdbuf()); // read it
   	  memo.Insert(*sb, sb->length()); // give it to the gui to copy
   	  
   // if the script is just too bug, or currupted to make ti appear too big,
   // an exception here (e.g., no more memory) can cripple the IDE.  so,
   // any exception here and we'll delete the saved script and try to go on.
   } catch (CException & e) { 
   	  delete sb;
   	  emptyScript();
   	  return;
   } catch (std::bad_alloc &) {
   	  delete sb;
   	  emptyScript();
   	  return;
   } catch (...) {
   	  delete sb;
   	  emptyScript();
   	  return;
   }
   delete sb; // delete the temp
   m_script__ctor(fileModeUpdate); // reset the script  
   //jal--
   //MemStream * script = dynamic_cast<MemStream *>(m_script);
   //int l = script->pcount();
   //char * str = script->str();
   //memo.Insert(str, l);
   //script->rdbuf()->freeze(0);
   //jal--
   return;
}

/*-----------------------------------------------------CIRexxApp::emptyScript-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::emptyScript()
{
   // create an empty script
   m_script__ctor(fileModeReadWrite);   
   //jal-- MemStream * script = dynamic_cast<MemStream *>(m_script);
   //jal-- script->reset();
   m_isScriptDirty = true;
   return;
}

/*---------------------------------------------------------CIRexxApp::setArgs-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::setArgs(const char * args, UInt32 size)
{
   m_args->empty();
   m_args->append(args, size);
   return;
}

/*----------------------------------------CIRexxApp::copyScriptFromFindResult-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::copyScriptFromFindResult(GoToParamsPtr pGoToParams)
{
   Err err;
   LocalID dbID;
   UInt16 cardNo = 0;
   DmOpenRef dbP;
   DmSearchStateType searchState;
   MemHandle hRecord;

   if ((err = DmGetNextDatabaseByTypeCreator(
       true, &searchState, 'data', CREATORID, true, &cardNo, &dbID)) != errNone) {
      return;
   }
   if ((dbP = DmOpenDatabase(cardNo, dbID, dmModeReadOnly)) == 0) {
      return;
   }
   if (!(hRecord = DmQueryRecord(dbP, pGoToParams->recordNum))) {
      DmCloseDatabase(dbP);
      return;
   }
   Char * p = (char *)MemHandleLock(hRecord);
   UInt32 size = MemHandleSize(hRecord);
   if (p[size - 1] == '\0') { --size; }
   emptyScript();
   appendScript(p, size);
   MemHandleUnlock(hRecord);
   DmCloseDatabase(dbP);
   return;
}

/*---------------------------------------------CIRexxApp::saveScriptPositions-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::saveScriptPositions(
   UInt16 insert,
   UInt16 scroll,
   UInt16 select1,
   UInt16 select2)
{
   m_isScriptDirty = false;
   m_scriptInsertPosition = insert;
   m_scriptSrcollPosition = scroll;
   m_scriptSelect1Position = select1;
   m_scriptSelect2Position = select2;
   return;
}

/*------------------------------------------CIRexxApp::restoreScriptPositions-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CIRexxApp::restoreScriptPositions(
   UInt16 & insert,
   UInt16 & scroll,
   UInt16 & select1,
   UInt16 & select2)
{
   if (!m_isScriptDirty) {
      insert = m_scriptInsertPosition;
      scroll = m_scriptSrcollPosition;
      select1 = m_scriptSelect1Position;
      select2 = m_scriptSelect2Position;
   } else {
      insert = 0;
      scroll = 0;
      select1 = select2 = 0;
   }
   return;
}

/*==========================================================================*/
