/* $Id: Tracer.cpp,v 1.56 2011-08-02 15:19:24 pgr Exp $ */

#include <iomanip>

#include "../toolslib/uccompos.h"
#include "../toolslib/SystemContext.h"
#include "RexxString.h"
#include "Exception.h"
#include "Tracer.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-------------------------------------------------------------Tracer::Tracer-+
|                                                                             |
+----------------------------------------------------------------------------*/
Tracer::Tracer(
   Location const & loc,
   UnicodeComposer & erh
) :
   m_erh(erh), m_loc(loc)
{
   m_levelInteractive = 0;
   m_size = 0;
   m_skipPauseCount = 0;
   m_isClauseShown = false;
   m_ix = 0;
   m_buf = 0;
}

/*------------------------------------------------------------Tracer::~Tracer-+
|                                                                             |
+----------------------------------------------------------------------------*/
Tracer::~Tracer() {
   delete [] (char *)m_buf;
}

/*-----------------------------------------------------------Tracer::makeRoom-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::makeRoom()
{
   char * old = (char *)m_buf;
   m_buf = new char [(m_size + INC) * sizeof m_setting];
   memcpy(m_buf, old, m_size * sizeof m_setting);
   delete [] old;
   m_size += INC;
}

/*--------------------------------------------------------Tracer::traceFormat-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceFormat(char c, RexxString const & value) const
{
   MemStream temp;
   temp
      << "       >" << c << ">  "
      << std::setw(1+(2*m_loc.m_nesting))
      << "\"" << value << '\"'
      << std::endl;
   SystemContext::cerr() << temp.rdbuf() << std::flush;
}

/*------------------------------------------------------Tracer::traceCompound-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceCompound(
   RexxString const & strStem, RexxString const & strTail) const
{
   MemStream temp;
   temp << "       >C>  " << std::setw(1+(2*m_loc.m_nesting)) << "\""
      << strStem << strTail << "\"" << std::endl;
   SystemContext::cerr() << temp.rdbuf() << std::flush;
}

/*-------------------------------------------------------Tracer::traceResults-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceResults(RexxString ** ppStr, int iMax) const {
   for (int i=0; i < iMax; ++i) {
      if (ppStr[i]->length()) traceFormat('>', *ppStr[i]);
   }
}

/*---------------------------------------------------------Tracer::traceBadRc-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceBadRc(int rc) const
{
   MemStream temp;
   if (m_levelInteractive) {                  // isAncestorInteractive
      temp << "       +++ Interactive trace.  RC(" << rc << ") +++" << std::endl;
   }else if (m_setting.isTracingBadRc()) {
      if (!m_setting.isTracingClauses()) {
         showSourceLine(m_loc, temp);
      }
      temp << "       +++ RC(" << rc << ") +++" << std::endl;
   }
   SystemContext::cerr() << temp.rdbuf() << std::flush;
}

/*--------------------------------------------------------Tracer::traceClause-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceClause()
{
   // zeroes the m_skipPauseCount that should be -1 (or 0 by a previous call)
   if (--m_skipPauseCount <= 0) {
      m_skipPauseCount = 0;   // pause at next clause
      m_isClauseShown = true;
   }
   if (m_isClauseShown) {
      MemStream temp;
      showSourceLine(m_loc, temp);
      SystemContext::cerr() << temp.rdbuf() << std::flush;
   }
}

/*----------------------------------------------------Tracer::setTraceSetting-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tracer::setTraceSetting(RexxString & value, bool denySkipCount)
{
   bool isInteractive = m_setting.isInteractive();

   if (
      (m_levelInteractive == 0) &&
      m_setting.isInteractive() &&
      !denySkipCount
   ) {
      /*
      | ignore trace instructions coming in the rexx source
      | "denySkipCount" tells it is a built-in (not an instruction)
      */
      return true;
   }
   if (m_setting.set(value, m_skipPauseCount, denySkipCount)) {
      if (m_skipPauseCount < 0) {
         m_skipPauseCount = -m_skipPauseCount;
         m_isClauseShown = false;
      }else {
         m_isClauseShown = true;
      }
      if (!isInteractive && m_setting.isInteractive()) {
         m_erh << ECE__INFO << _REX__0_3 << "       +++ " << endm;
      }
      return true;
   }else {
      return false;
   }
}

/*----------------------------------------------Tracer::traceInteractiveError-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceInteractiveError(RecoverableException const & e) {
   m_erh << ECE__INFO << _REX__0_4 << "       +++ "
         << e.getRxMainCodeNo()
         << getMsgTemplate(e.getRxMainMsgId()) << endm;
}

/*STATIC---------------------------------------------------Tracer::traceError-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceError(
   UnicodeComposer::Message const & msg,
   Location const & loc
) {
   MemStream temp;
   showSourceLine(loc, temp);
   MsgTemplateId id = msg.inqStringId();
   int subCodeNo = ::getRxSubCodeNo(id);
   temp << "Error " << ::getRxMainCodeNo(id);
   if (subCodeNo !=0) {
      temp << '.' << subCodeNo;
   }
   temp << " in ";
   temp.write((char const *)loc.m_path, loc.m_path.length());
   temp << ", line " << loc.m_lineno << ": " << msg.stringize() << std::endl;
   SystemContext::cerr() << temp.rdbuf() << std::flush;
}

/*-------------------------------------------------------Tracer::traceWarning-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::traceWarning(UnicodeComposer::Message const & msg) {
   SystemContext::cerr() << msg.stringize() << std::endl << std::flush;
}

/*-----------------------------------------------------Tracer::showSourceLine-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Tracer::showSourceLine(Location const & loc, ostream & out)
{
   out
      << std::setw(6) << loc.m_lineno
      << " *-*"
      << std::setw(2 * loc.m_nesting)
      << "";
   out.write(loc.m_contents, loc.m_contents.length());
   out << std::endl;
}

/*-------------------------------------------------------Tracer::Setting::set-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Tracer::Setting::set(
   RexxString & val,
   int & skipPauseCount,
   bool denySkipCount
) {
   bool isInteractiveChange = false;
   char const * c_p = val;
   int newSkipPauseCount = 0;

   if (*c_p == '?') {
      isInteractiveChange = true;
      do {
         m_isInteractive = !m_isInteractive;
      }while (*++c_p == '?');
   }
   switch (*c_p) {
   case 'A':
      m_setval = ALL;
      break;
   case 'C':
      m_setval = COMMANDS;
      break;
   case 'E':
      m_setval = ERROR;
      break;
   case 'F':
      m_setval = FAILURE;
      break;
   case 'I':
      m_setval = INTERMEDIATES;
      break;
   case 'L':
      m_setval = LABELS;
      break;
   case 0:
      if (isInteractiveChange) break;
      // else, fall thru: default setting demanded.
   case 'N':
      m_setval = NORMAL;
      break;
   case 'O':
      m_setval = OFF;
      m_isInteractive = false;
      break;
   case 'R':
      m_setval = RESULTS;
      break;
   default:
      if (denySkipCount || !val.isInt(newSkipPauseCount)) {
         return false;
      }
      break;
   }
   skipPauseCount = newSkipPauseCount;
   return true;
}

/*-------------------------------------------------------Tracer::Setting::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Tracer::Setting::get() const
{
   char buf[4];
   char *c_p = buf;
   int i = 0;
   if (m_isInteractive) {
      *c_p++ = '?';
   }
   switch (m_setval) {
   case ALL:           *c_p++ = 'A';  break;
   case COMMANDS:      *c_p++ = 'C';  break;
   case ERROR:         *c_p++ = 'E';  break;
   case FAILURE:       *c_p++ = 'F';  break;
   case INTERMEDIATES: *c_p++ = 'I';  break;
   case LABELS:        *c_p++ = 'L';  break;
   case NORMAL:        *c_p++ = 'N';  break;
   case OFF:           *c_p++ = 'O';  break;
   case RESULTS:       *c_p++ = 'R';  break;
   default:            *c_p++ = 'O';  break;
   }
   return RexxString(buf, c_p - buf);
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
