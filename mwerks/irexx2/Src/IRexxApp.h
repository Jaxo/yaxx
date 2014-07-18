/* $Id: IRexxApp.h,v 1.56 2002/11/19 17:13:39 jlatone Exp $ */

#ifndef __IRexxApp_H__
#define __IRexxApp_H__

#include "assert.h"
#include "Common.h"

#include "EditForm.h"
#include "LoadForm.h"
#include "ConsoleForm.h"

#include "StringBuffer.h"
#include "PalmConsole.h"
#include "AppStopEventException.h"
#include "SystemContext.h"
#include "Rexx.h"

#include "Rexxlet.h"

extern char const * getRexxAppMessage(int id);
extern char const * getYasp3Message(int id);
extern char const * getRexxMessage(int id);
extern char const * getKeywordRsc(int id);
extern char const * getBuiltInNameRsc(int id);
extern char const * getDayNameRsc(int day);
extern char const * getMonthNameRsc(int month);

static const int msgUserUnkownException = 0;
static const int msgRexxUnkownException = 1;
static const int msgOutOfMemory = 2;
static const int msgMalformedURL = 3;
static const int msgRexxIsRunning = 4;
static const int msgRexxInfo = 5;
static const int msgRexxIsStarting = 6;
static const int msgRexxIOException = 7;

class ScriptRecord;

/*------------------------------------------------------------------CIRexxApp-+
| Application class                                                           |
+----------------------------------------------------------------------------*/
class CIRexxApp : public CPalmStApp<CIRexxApp> //CPalmApp
{
public:
   /*
   |  CIRexxApp() { }
   |
   |  IMPORTANT!  Do not declare/define a ctor!
   |              Access to it requires access to the
   |              global (A4/A5) space, which is not available
   |              in non-normal launches (e.g., find, reset, etc).
   |              !!!!! THIS IS NO LONGER TRUE !!!!!
   |              THIS IS FIXED WITH _CW_SetupExpandedMode().
   |              _CW_SetupExpandedMode() does not fix the issue
   |              of making cross-segment calls, tho.
   */
   
   // Overrides
   //virtual Boolean PreSystemEventHook(EventType *pEvent);
   UInt32 GetMinRomVersion() {
      return sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0);
   }
   Boolean RunEventLoop();
   virtual Err NormalLaunch();
   virtual Err FindLaunch(FindParamsPtr pFindParams);
   virtual Err GotoLaunch(GoToParamsPtr pGoToParams);
   Err InitInstance();
   Err ExitInstance();
   
   // for palm os 5
   void RegisterForNotifications();
   void HandleNotification(SysNotifyParamType * np);

private:
   void NewGlobals();  // used internal for different launches
public:
   void NewScriptIndexDB(); // used internal & external for different launches

public:

   // Form map
   BEGIN_FORM_MAP()
     FORM_MAP_ENTRY(EditForm, CEditForm)
     FORM_MAP_ENTRY(LoaditForm, CLoadForm)
     FORM_MAP_ENTRY(ConsoleForm, CConsoleForm)
   END_FORM_MAP()

   // the app
   CDatabase * databaseOfScripts();
   UInt16 numberOfScripts();
   void titleOfScript(Int16 index, CString & title);
   void dbIndicatorOfScript(Int16 index, CString & dbi);
   void copyScriptFromIndexDatabase(Int16 index);
   void copyScriptFromFindResult(GoToParamsPtr pGoToParams);
   void setScript(const char * string, UInt32 size);
   void appendScript(const char * string, UInt32 size);
   void emptyScript();
   void copyScriptToMemo(CMemo & memo);
   void setArgs(const char * args, UInt32 size);
   bool isScriptEmpty();
   bool isScriptDirty();
   void saveScriptPositions(
      UInt16 insert, UInt16 scroll, UInt16 select1, UInt16 select2
   );
   void restoreScriptPositions(
      UInt16 & insert, UInt16 & scroll, UInt16 & select1, UInt16 & select2
   );
   bool isScriptIndexDBInitialized();
   void setLoadPosition(Int16 row);
   Int16 getLoadPosition();
   void BALR(UInt16 from, UInt16 to);
   void RET();
   UInt32 queryMemory(/*UInt32 * pTotalMemory, UInt32 * pDynamicMemory*/);


public:
   Err RexxletLaunch();
   Err RexxappLaunch();
   void interpret(RexxString & result, int & rc);
   bool isRexxletPauseWhenDone();
   void doAbout();

private:
   void initializeIndexDatabase();
   void addRecordsToIndex(CArray<ScriptRecord *> & records, CDatabase * db, UInt16 cat, char * dbi);
};

inline void GotoLauncher()
{
   EventType event;
   event.eType = keyDownEvent;
   event.data.keyDown.chr = launchChr;
   event.data.keyDown.modifiers = commandKeyMask;
   EvtAddEventToQueue(&event);   
}

//<<<JAL TODO:  Clean this thing up.
//              It needs to be encapsulated properly.
/*---------------------------------------------------------------ScriptRecord-+
| Set up our own way of referencing the scripts in the MemoPad DB             |
| because we have to be smarter since we're supporting                        |
| pedit-segmented entries. Then, we'll turn this                              |
| into a CDatabase.                                                           |
+----------------------------------------------------------------------------*/
class ScriptRecord {
   friend class CIRexxApp;
public:
   enum {
      MAX_TITLELEN = 20,
      MAX_PEDITHEADERLEN = 200
   };
   ScriptRecord() { }
   ScriptRecord(CDatabase * db, char * dbi, RexxString & title, Int16 index, Int16 segmentNo = -1);

private:
   char m_title[MAX_TITLELEN];
   char m_dbi[2];
   CArray<Int16> m_indexes;
   CArray<Int16> m_segments;  //<<<JAL TODO:  should be sorting on these!
   CDatabase * m_db;
};

/* -- INLINES -- */
inline ScriptRecord::ScriptRecord(
   CDatabase * db, char * dbi, RexxString & title, Int16 index, Int16 segmentNo
) {
   m_dbi[0] = (dbi && *dbi)? *dbi : ' ';
   m_dbi[1] = '\0';
   if (title.length() > 0) {
      strncpy(m_title, title, MAX_TITLELEN);
      m_title[MAX_TITLELEN - 1] = '\0';
   } else {
      m_title[0] = '\0';
   }
   m_indexes.Insert(0, index);
   m_segments.Insert(0, segmentNo);
   m_db = db;
}

#endif
/*===========================================================================*/
