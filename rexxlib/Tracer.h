/* $Id: Tracer.h,v 1.39 2002-10-12 09:47:55 pgr Exp $ */

#ifndef COM_JAXO_YAXX_TRACER_H_INCLUDED
#define COM_JAXO_YAXX_TRACER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "CodeBuffer.h"
#include "../toolslib/uccompos.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class RecoverableException;
class RexxString;

/*------------------------------------------------------------ class Location +
|                                                                             |
+----------------------------------------------------------------------------*/
class Location {
public:
   int m_lineno;           // line number
   int m_nesting;          // nesting level
   RexxString m_path;      // current file path
   RexxString m_contents;  // contents
};

/*------------------------------------------------------------- class Tracer -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Tracer {
public:
   Tracer(Location const & loc, UnicodeComposer & erh);
   ~Tracer();

   bool isMyselfInteractive() const;
   bool isTracingClausesOrLabels() const;
   bool isTracingClauses() const;
   bool isTracingResults() const;
   bool isTracingIntermediates() const;
   bool mustPause();

   RexxString getTraceSetting() const;
   bool setTraceSetting(RexxString & value, bool denySkipCount);

   void startInterpret(bool isInteractiveTrace);
   void endInterpret();
   void startRoutine();
   void endRoutine();

   void traceResults(RexxString ** ppStr, int iMax) const;
   void traceMonadicOp(RexxString const & value) const;
   void traceDyadicOp(RexxString const & value) const;
   void traceAssign(RexxString const & value) const;
   void traceVariable(RexxString const & value) const;
   void traceFunction(RexxString const & value) const;
   void traceLiteral(RexxString const & value) const;
   void tracePlaceholder(RexxString const & value) const;
   void traceCompound(RexxString const & strStem, RexxString const & strTail) const;
   void traceNotLabelClause();
   void traceClause();
   void traceBadRc(int rc) const;
   void traceInteractiveError(RecoverableException const & e);

   static void traceError(
      UnicodeComposer::Message const & msg,
      Location const & loc
   );
   static void traceWarning(UnicodeComposer::Message const & msg);
private:
   class Setting {
   public:
      Setting();
      bool isInteractive() const;
      bool isTracingBadRc() const;
      bool isTracingClausesOrLabels() const;
      bool isTracingClauses() const;
      bool isTracingResults() const;
      bool isTracingIntermediates() const;
      bool set(RexxString & pVal, int & skipClauseCount, bool denySkipCount);
      RexxString get() const;

   private:
      enum {
         _BADRC = 1,
         OFF           = 0,                 // Nothing is traced
         NORMAL        = (1 << 1) + _BADRC, // Host: Failure host
         FAILURE       = (2 << 1) + _BADRC, // (same as NORMAL)
         ERROR         = (3 << 1) + _BADRC, // Host: Failure + Error
         COMMANDS      = (4 << 1) + _BADRC, // Host: Failure + Error + Exec
         LABELS        = (5 << 1) ,         // Label
         ALL           = (6 << 1) ,         // Clauses: Exec
         RESULTS       = (7 << 1) + _BADRC, // Clauses: Exec + Results
         INTERMEDIATES = (8 << 1) + _BADRC  // Clauses: Exec + Results + Itmdtes
      } m_setval;
      bool m_isInteractive;
   };

   enum { INC = 20 };
   UnicodeComposer & m_erh;
   Location const & m_loc;
   bool m_isClauseShown;
   int m_levelInteractive;
   Setting m_setting;
   int m_size;
   int m_skipPauseCount;
   int m_ix;
   void * m_buf;

   void traceFormat(char c, RexxString const & value) const;
   static void showSourceLine(Location const & loc, ostream & out);
   void makeRoom();
};

/* -- INLINES -- */
inline Tracer::Setting::Setting() {
   m_setval = NORMAL;
   m_isInteractive = false;
}

inline bool Tracer::Setting::isInteractive() const {
   return m_isInteractive;
}

inline bool Tracer::Setting::isTracingBadRc() const {
   return (m_setval & _BADRC);
}

inline bool Tracer::Setting::isTracingClausesOrLabels() const {
   return (m_setval >= LABELS);
}

inline bool Tracer::Setting::isTracingClauses() const {
   return (m_setval >= ALL);
}

inline bool Tracer::Setting::isTracingResults() const {
   return m_setval >= RESULTS;
}

inline bool Tracer::Setting::isTracingIntermediates() const {
   return m_setval >= INTERMEDIATES;
}

inline void Tracer::traceMonadicOp(RexxString const & value) const {
   traceFormat('P', value);
}
inline void Tracer::traceDyadicOp(RexxString const & value) const {
   traceFormat('O', value);
}
inline void Tracer::traceAssign(RexxString const & value) const {
   traceFormat('=', value);
}
inline void Tracer::traceVariable(RexxString const & value) const {
   traceFormat('V', value);
}
inline void Tracer::traceFunction(RexxString const & value) const {
   traceFormat('F', value);
}
inline void Tracer::traceLiteral(RexxString const & value) const {
   traceFormat('L', value);
}
inline void Tracer::tracePlaceholder(RexxString const & value) const {
   traceFormat('.', value);
}

inline void Tracer::startInterpret(bool isInteractiveTrace) {
   if ((isInteractiveTrace) || (m_levelInteractive)) {
      ++m_levelInteractive;
      if (isInteractiveTrace) m_skipPauseCount = 0; // stop b4 clause
   }
}

inline void Tracer::endInterpret() {
   if (m_levelInteractive) --m_levelInteractive;
}

inline void Tracer::startRoutine() {
   if (m_ix >= m_size) makeRoom();
   ((Setting *)m_buf)[m_ix++] = m_setting;
}

inline void Tracer::endRoutine() {
   m_setting = ((Setting *)m_buf)[--m_ix];
}

inline bool Tracer::isTracingIntermediates() const {
   return (m_setting.isTracingIntermediates());
}

inline bool Tracer::isTracingResults() const {
   return (m_setting.isTracingResults());
}

inline bool Tracer::isMyselfInteractive() const {
   return ((m_levelInteractive==0) && m_setting.isInteractive());
}

inline bool Tracer::isTracingClausesOrLabels() const {
   return (m_setting.isTracingClausesOrLabels());
}

inline bool Tracer::isTracingClauses() const {
   return (m_setting.isTracingClauses());
}

inline bool Tracer::mustPause() {
   if (isMyselfInteractive() && (m_skipPauseCount == 0)) {
      m_skipPauseCount = -1;
      return true;
   }else {
      return false;
   }
}

inline void Tracer::traceNotLabelClause() {
   if (isTracingClauses()) traceClause();  // only if tracing clauses
}

inline RexxString Tracer::getTraceSetting() const {
   return m_setting.get();   // delegate
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
