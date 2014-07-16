/* $Id: IRexx.cpp,v 1.19 2002/12/13 21:23:05 jlatone Exp $ */

#include "Common.h"
#include "IRexxApp.h"
#include "RexxletForms.h"

//>>>PGR: All of these statics are highly unsafe!  FIXME
static char s_msg[180];
static char s_kwd[12];
static char s_bltin[12];
static char s_dayMonth[12];

// Entry point
//DEFAULT_STARTER(CIRexxApp)
// Had to override this MACRO because this change is part of allowing POL
// to support expanded-mode apps, i.e., calling _CW_SetupExpandedMode().
UInt32 PilotMain(UInt16 wCommand, MemPtr pCommandPBP, UInt16 wLaunchFlags)
{

   if (!(wLaunchFlags & sysAppLaunchFlagNewGlobals)) {
      // If you want to use global variables that are in the expanded data
      // section (virtual tables, exceptions tables), your app should call the
      // function _CW_SetupExpandedMode after it determines that its a launch
      // code it wants to handle.
      // Note, even calling _CW_SetupExpandedMode won't load additional segments.
      // You can get your expanded mode data, but you can't make cross-segment
      // calls.  This is because the segment jump table lives in A5-storage.
      // THIS MEANS EVERYTHING WE USE MUST BE IN THE SAME SEGMENT AS THIS MODULE!
      _CW_SetupExpandedMode();
   }
   
   Err err;
   CIRexxApp palmApp;
   palmApp.m_wCommand = wCommand;
   palmApp.m_pCommandPBP = pCommandPBP;
   palmApp.m_wLaunchFlags = wLaunchFlags;
   try {
      err = palmApp.AppStart();
      if (err == 0)
      {
         palmApp.EventLoop();
         err = palmApp.AppStop();
      }
   }catch (CException & e) {
      e.GetMessage(s_msg, sizeof(s_msg));
      CRexxletAlertForm::popup(*s_msg? s_msg : getRexxAppMessage(msgUserUnkownException));
   }catch (...) {
      CRexxletAlertForm::popup(getRexxAppMessage(msgUserUnkownException));
   }
   return err;
}

char const * getRexxAppMessage(int id) {
   SysStringByIndex(
      RexxAppMessagesStringList,
      id,
      s_msg,
      sizeof s_msg
   );
   return s_msg;
}

char const * getRexxMessage(int id) {
   SysStringByIndex(
      RexxMessagesStringList,
      id,
      s_msg,
      sizeof s_msg
   );
   return s_msg;
}

char const * getYasp3Message(int id) {
   SysStringByIndex(
      Yasp3MessagesStringList,
      id,
      s_msg,
      sizeof s_msg
   );
   return s_msg;
}

char const * getKeywordRsc(int id) {
   SysStringByIndex(
      RexxKeywordsStringList,
      id,
      s_kwd,
      sizeof s_kwd
   );
   return s_kwd;
}

char const * getBuiltInNameRsc(int id) {
   SysStringByIndex(
      RexxBuiltInNamesStringList,
      id,
      s_bltin,
      sizeof s_bltin
   );
   return s_bltin;
}

extern char const * getDayNameRsc(int day) {
   SysStringByIndex(
      DayNamesStringList,
      day,
      s_dayMonth,
      sizeof s_dayMonth
   );
   return s_dayMonth;
}

extern char const * getMonthNameRsc(int month) {
   SysStringByIndex(
      MonthNamesStringList,
      month,
      s_dayMonth,
      sizeof s_dayMonth
   );
   return s_dayMonth;
}

