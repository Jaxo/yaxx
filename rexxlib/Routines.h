/* $Id: Routines.h,v 1.41 2002-10-16 11:38:14 pgr Exp $ */

#ifndef COM_JAXO_YAXX_ROUTINE_H_INCLUDED
#define COM_JAXO_YAXX_ROUTINE_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Constants.h"
#include "Arguments.h"
#include "CodeBuffer.h"
#include "Exception.h"
#include "TimeClock.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

enum Signaled;
class Routine;
class Routines;

/*------------------------------------------------------------ class Routine -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Routine : private Constants {
   friend class Routines;
public:

   int getStackTop() const {
      return m_top;
   }
   bool isSignalDisabled() const;
   bool canRecoverFromSyntax() const;
   void raise(Signaled sig);
   RexxString getCallType() const;
   RexxString getEnvironment() const;
   RexxString getAddress() const;
   RexxString getForm() const;
   DecRexxContext const & getDecRexxContext() const;
   int getSeed() const;
   RexxString & getSignal(Signaled sig);
   void getElapsedTime(StopWatch const & nowClock, ostream & result);

   void setEnvironment(RexxString & value);
   void setDefaultDigits();
   void setDigits(int value);
   void setDefaultFuzz();
   void setFuzz(int value);
   void setSeed(int value);
   void setForm(RexxString & value);
   void setDefaultTrapper(RexxString const & key, bool isSignalStmt);
   void setTrapper(RexxString const & key, RexxString & strName, bool isSignalStmt);
   void unsetTrapper(RexxString const & key, bool isSignalStmt);
   void setElapsedTime(StopWatch const & nowClock, ostream & result);

public: // temporarly.
   CallType m_ct;            // call type...
   bool m_isProcedure;       // is a PROCEDURE (has a private variable map)
   Arguments m_args;         // arguments (on the stack)

private:
   UnicodeComposer & m_erh;  // Error handler (shared)
   CodePosition m_posCur;    // current position
   CodePosition m_posEnd;    // end position
   int m_top;                // stack after args
   int m_base;               // stack position
   int & m_raisedConds;      // call-on's raised (shared between all routines)
   int & m_seed;             // seed for random number generator (shared)
   StopWatch m_stopClock;    // to compute elapsed time
   DecRexxContext m_ctxt;
   unsigned short m_signals; // signaled conditions
   unsigned short m_callons; // call-on's conditions
   RexxString m_env;         // address environment
   RexxString m_strError;
   RexxString m_strFailure;
   RexxString m_strHalt;
   RexxString m_strLostDigits;
   RexxString m_strNoValue;
   RexxString m_strNotReady;
   RexxString m_strSyntax;

   Routine(RexxString &, int &, int &, UnicodeComposer &); // Main routine
};

/*----------------------------------------------------------- class Routines -+
| IMPLEMENTATION:                                                             |
|   Routines is not an IS-A Routine.  This derivation only serves the         |
|   purposes of the implementation, making Routines a cache of the            |
|   current Routine.                                                          |
+----------------------------------------------------------------------------*/
class Routines : public Routine {
public:
   Routines(RexxString & strArgs, UnicodeComposer & erh);
   ~Routines();

   int getDepth() {
      return m_ixCur;
   }

   bool hasPendingConds() {
      return m_pendingConds != 0;
   }
   void raisePendingConds();

   void startRoutine(
      CallType ct,
      CodeBuffer & cb,
      int top,
      RexxString ** stack,
      Arguments::PresenceBits presenceBits
   );
   void endRoutine(
      CodeBuffer & cb,
      int & top
   );
   void startInterpret(
      CodeBuffer & cb,
      int top,
      int base,
      bool disableSignal
   );
   void endInterpret(
      CodeBuffer & cb,
      int & top
   );

private:
   Routine * m_aRtne;       // routines list: subroutine & function array
   int m_ixCur;             // current routine index
   int m_aRtneSize;         // number of items in routines list
   int m_pendingConds;      // callons raised since last clause
   int m_seed;              // seed for random number generator
   enum { ROUTINE_INC = 10 };

   void save();
   void restore();
};

/* --- INLINES --- */
inline DecRexxContext const & Routine::getDecRexxContext() const {
   return m_ctxt;
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
