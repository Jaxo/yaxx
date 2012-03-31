/* $Id: Routines.cpp,v 1.69 2002-12-10 20:13:52 jlatone Exp $ */

#include "Exception.h"
#include "Routines.h"

#ifdef __MWERKS_NEEDS_REALLOC_FIX__
#define realloc reallocFix4Mwerks
extern void *reallocFix4Mwerks(void *ptr, size_t size);
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-----------------------------------------------------------Routine::Routine-+
| Constructor for the main routine                                            |
+----------------------------------------------------------------------------*/
Routine::Routine(
   RexxString & strArgs,
   int & raisedConds,
   int & seed,
   UnicodeComposer & erh
) :
   m_erh(erh),
   m_args(strArgs),
   m_raisedConds(raisedConds),
   m_seed(seed)
{
   m_ct = CT_COMMAND;              // call type...
   m_base = -1;                    // top of stack
   m_top = -1;                     // top - arguments
   m_posCur = CodeBuffer::Start;
   m_posEnd = CodeBuffer::Start;
   m_isProcedure = false;
   m_signals = 0;
   m_callons = 0;
   m_strError   = STR_ERROR;
   m_strHalt    = STR_HALT;
   m_strNoValue = STR_NOVALUE;
   m_strNotReady= STR_NOTREADY;
   m_strSyntax  = STR_SYNTAX;
   m_strFailure = STR_FAILURE;
   m_strLostDigits = STR_LOSTDIGITS;
}

/*----------------------------------------------------Routine::getElapsedTime-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::getElapsedTime(StopWatch const & nowClock, ostream & result) {
   if (!m_stopClock.isSet()) {
      m_stopClock = nowClock;
   }
   m_stopClock.formatElapsedTime(nowClock, result);
}

/*------------------------------------------------Routines::raisePendingConds-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routines::raisePendingConds()
{
   Signaled sig;
   if (m_pendingConds & SIG_ERROR) {
      sig = SIG_ERROR;
   }else if (m_pendingConds & SIG_FAILURE) {
      sig = SIG_FAILURE;
   }else if (m_pendingConds & SIG_HALT) {
      sig = SIG_HALT;
   }else if (m_pendingConds & SIG_NOTREADY) {
      sig = SIG_NOTREADY;
   }else {
      return;
   }
   m_pendingConds &= ~sig;
   throw RecoverableException(sig, false);
}

/*-----------------------------------------------------Routines::endInterpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routines::endInterpret(
   CodeBuffer & cb,
   int & top
) {

   /* fix ip and stack */
   cb.jumpTo(m_posCur);
   
//////////////////////////////////////////////////////////////
// <<<JAL TODO See the JAL comment in Interpreter.cpp in loadFile().
//             In brief, this truncate() will truncate code that was
//			   loaded from within a routine/interpret.
//   cb.truncate(m_posEnd);
//////////////////////////////////////////////////////////////

   top = m_base;

   // inherit back
   m_aRtne[m_ixCur-1].m_env = m_env;

   // the trick is to avoid to copy back SIG_DISABLED.
   if (0 == (m_signals & SIG_DISABLED)) {
      m_aRtne[m_ixCur-1].m_signals = m_signals;
   }
   restore();
}

/*--------------------------------------------------Routine::isSignalDisabled-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Routine::isSignalDisabled() const {
   return ((m_signals & SIG_DISABLED) != 0);
}

/*----------------------------------------------Routine::canRecoverFromSyntax-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Routine::canRecoverFromSyntax() const {
   // for SYNTAX, DISABLED means: don't exit, nor take any user-def
   return ((m_signals & (SIG_SYNTAX | SIG_DISABLED)) != 0);
}

/*-------------------------------------------------------------Routine::raise-+
| DO NOT USE for SIG_SYNTAX!  (use: m_erh << ECE__ERROR)                      |
+----------------------------------------------------------------------------*/
void Routine::raise(Signaled sig) {
   if ((m_signals & sig) && (!(m_signals & SIG_DISABLED))) {
      throw RecoverableException(sig, true);
   }else if (m_callons & sig) {
      m_raisedConds |= sig;   // wait 'til end of clause
   }
}

/*-------------------------------------------------------Routine::getCallType-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Routine::getCallType() const {
   switch (m_ct) {
   case CT_SUBROUTINE:
      return STR_SUBROUTINE;
   case CT_FUNCTION:
      return STR_FUNCTION;
   default:
      return STR_COMMAND;
   }
}

/*----------------------------------------------------Routine::getEnvironment-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Routine::getEnvironment() const {
   return m_env;
}

/*--------------------------------------------------------Routine::getAddress-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Routine::getAddress() const
{
   if (m_env.length()) {
      return m_env;
   }else {
      int len = 0;
      #if defined MWERKS_NEEDS_ADDR_FIX // getenv (shell)
         char * c_p = 0;
      #elif defined(_WIN32)
         char * c_p = getenv("COMSPEC");
      #else
         char * c_p = getenv("SHELL");
      #endif
      if (!c_p) return RexxString("SHELL");
      char * d_p = c_p + strlen(c_p);
      while (--d_p >= c_p) {
         switch (*d_p) {
         case '.':
            len = 0;
            continue;
         case '\\':
         case '/':
            c_p = d_p+1;
            break;
         default:
            ++len;
            break;
         }
      }
      return RexxString(c_p, len);
   }
}

/*-----------------------------------------------------------Routine::getForm-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Routine::getForm() const
{
   if (m_ctxt.isFormEngineering()) {
      return STR_ENGINEERING;
   }else {
      return STR_SCIENTIFIC;
   }
}

/*----------------------------------------------------Routine::setEnvironment-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setEnvironment(RexxString & value) {
   if (value.length() > 250) {
      m_erh << ECE__ERROR << _REX__29_1 << 250 << value << endm;
   }
   m_env = value;
}

/*--------------------------------------------------Routine::setDefaultDigits-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setDefaultDigits() {
   m_ctxt.digits = DecRexxContext::DEFAULT_PRECISION;
}

/*---------------------------------------------------------Routine::setDigits-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setDigits(int iDigitPrec)
{
   if (iDigitPrec <= 0) {
      m_erh << ECE__ERROR << _REX__26_5 << iDigitPrec << endm;
   }
   if (iDigitPrec <= m_ctxt.m_fuzz) {
      m_erh << ECE__ERROR << _REX__33_1 << iDigitPrec << m_ctxt.m_fuzz << endm;
   }
   if (iDigitPrec > DecRexxContext::MAX_PRECISION) {
      m_erh
         << ECE__ERROR << _REX__33_2
         << iDigitPrec << DecRexxContext::MAX_PRECISION << endm;
   }
   m_ctxt.digits = iDigitPrec;
}

/*-----------------------------------------------------------Routine::getSeed-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Routine::getSeed() const {
   return m_seed;
}

/*----------------------------------------------------Routine::setDefaultFuzz-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setDefaultFuzz() {
   m_ctxt.m_fuzz = DecRexxContext::DEFAULT_FUZZ;
}

/*-----------------------------------------------------------Routine::setFuzz-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setFuzz(int fuzz)
{
   if (fuzz < 0) {
      m_erh << ECE__ERROR << _REX__26_6 << fuzz << endm;
   }
   if ((unsigned int)fuzz >= m_ctxt.digits) {
      m_erh << ECE__ERROR << _REX__33_1 << m_ctxt.digits << fuzz << endm;
   }
   m_ctxt.m_fuzz = fuzz;
}

/*-----------------------------------------------------------Routine::setSeed-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setSeed(int seed) {
   m_seed = seed;
   srand((unsigned int)seed);
}

/*----------------------------------------------------Routine::setElapsedTime-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setElapsedTime(StopWatch const & nowClock, ostream & result) {
   getElapsedTime(nowClock, result);
   m_stopClock = nowClock;
}

/*-----------------------------------------------------------Routine::setForm-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setForm(RexxString & value)
{
   if (value == STR_ENGINEERING) {
      m_ctxt.setFormEngineering(true);
   }else if (value == STR_SCIENTIFIC) {
      m_ctxt.setFormEngineering(false);
   }else {
      m_erh << ECE__ERROR << _REX__25_11 << "SCIENTIFIC or ENGINEERING" << value << endm;
   }
}

/*-------------------------------------------------Routine::setDefaultTrapper-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setDefaultTrapper(RexxString const & key, bool isSignalStmt)
{
   Signaled sig;

   switch (key.charAt(0)) {
   case 'E':
      sig = SIG_ERROR;
      m_strError = STR_ERROR;
      break;
   case 'F':
      sig = SIG_FAILURE;
      m_strFailure = STR_FAILURE;
      break;
   case 'H':
      sig = SIG_HALT;
      m_strHalt = STR_HALT;
      break;
   case 'L':
      sig = SIG_LOSTDIGITS;
      m_strLostDigits = STR_LOSTDIGITS;
      break;
   case 'N':
      if (key.charAt(2)=='V') {
         sig = SIG_NOVALUE;
         m_strNoValue = STR_NOVALUE;
      } else {
         sig = SIG_NOTREADY;
         m_strNotReady = STR_NOTREADY;
      }
      break;
   case 'S':
      sig = SIG_SYNTAX;
      m_strSyntax = STR_SYNTAX;
      break;
   }
   if (isSignalStmt) {
      m_signals |= sig;
      m_callons &= ~sig;
   }else {
      m_callons |= sig;
      m_signals &= ~sig;
   }
}

/*--------------------------------------------------------Routine::setTrapper-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::setTrapper(
   RexxString const & key, RexxString & strName, bool isSignalStmt
) {
   Signaled sig;

   switch (key.charAt(0)) {
   case 'E':
      sig = SIG_ERROR;
      m_strError = strName;
      break;
   case 'F':
      sig = SIG_FAILURE;
      m_strFailure = strName;
      break;
   case 'H':
      sig = SIG_HALT;
      m_strHalt = strName;
      break;
   case 'L':
      sig = SIG_LOSTDIGITS;
      m_strLostDigits = strName;
      break;
   case 'N':
      if (key.charAt(2)=='V') {
         sig = SIG_NOVALUE;
         m_strNoValue = strName;
      } else {
         sig = SIG_NOTREADY;
         m_strNotReady = strName;
      }
      break;
   case 'S':
      sig = SIG_SYNTAX;
      m_strSyntax = strName;
      break;
   }
   if (isSignalStmt) {
      m_signals |= sig;
      m_callons &= ~sig;
   }else {
      m_callons |= sig;
      m_signals &= ~sig;
   }
}

/*------------------------------------------------------Routine::unsetTrapper-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routine::unsetTrapper(RexxString const & key, bool isSignalStmt)
{
   unsigned short & condSide = isSignalStmt? m_signals : m_callons;

   switch (key.charAt(0)) {
   case 'E':
      condSide &= ~SIG_ERROR;
      break;
   case 'F':
      condSide &= ~SIG_FAILURE;
      break;
   case 'H':
      condSide &= ~SIG_HALT;
      break;
   case 'L':
      condSide &= ~SIG_LOSTDIGITS;
      break;
   case 'N':
      if (key.charAt(2)=='V') {
         condSide &= ~SIG_NOVALUE;
      }else {
         condSide &= ~SIG_NOTREADY;
      }
      break;
   case 'S':
      condSide &= ~SIG_SYNTAX;
      break;
   }
}

/*---------------------------------------------------------Routine::getSignal-+
| Also, resets the signal, or callon                                          |
+----------------------------------------------------------------------------*/
RexxString & Routine::getSignal(Signaled sig)
{
   m_signals &= ~sig;
   m_callons &= ~sig;
   switch (sig) {
   case SIG_ERROR:
      return m_strError;
   case SIG_FAILURE:
      return m_strFailure;
   case SIG_HALT:
      return m_strHalt;
   case SIG_LOSTDIGITS:
      return m_strLostDigits;
   case SIG_NOVALUE:
      return m_strNoValue;
   case SIG_NOTREADY:
      return m_strNotReady;
   default: // case SIG_SYNTAX:
      return m_strSyntax;
   }
}

/*---------------------------------------------------------Routines::Routines-+
|                                                                             |
+----------------------------------------------------------------------------*/
Routines::Routines(
   RexxString & strArgs,
   UnicodeComposer & erh
) :
   Routine(strArgs, m_pendingConds, m_seed, erh)
{
   m_pendingConds = 0;
   m_seed = -1;
   m_ixCur = 0;
   m_aRtneSize = 0;
   m_aRtne = 0;
}

/*--------------------------------------------------------Routines::~Routines-+
|                                                                             |
+----------------------------------------------------------------------------*/
Routines::~Routines()
{
   if (!m_aRtne) return; // m_aRtne==0 when the interpreter's 
   							 // "system resources exhausted" (stack overflow)
   while (m_ixCur--) {
      (m_aRtne + m_ixCur)->~Routine();
   };
   free(m_aRtne);
}

/*-----------------------------------------------------Routines::startRoutine-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routines::startRoutine(
   CallType ct,
   CodeBuffer & cb,
   int top,
   RexxString ** stack,
   Arguments::PresenceBits presenceBits
) {
   save();
   m_base = m_top = top;          // don't change the stack top.
   m_args.~Arguments();
   new(&m_args) Arguments(stack, presenceBits, m_base);
   if (ct == CT_SUBROUTINE) {
      --m_base;
   }
   m_isProcedure = false;
   m_ct = ct;
   m_posCur = cb.getRunPos();
}

/*-------------------------------------------------------Routines::endRoutine-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routines::endRoutine(
   CodeBuffer & cb,
   int & top
) {
   /* fix ip and stack */
   cb.jumpTo(m_posCur);
   top = m_base;
   restore();
}

/*---------------------------------------------------Routines::startInterpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routines::startInterpret(
   CodeBuffer & cb,
   int top,
   int base,
   bool isInteractiveTrace
) {
   save();

   // set the new values
   m_isProcedure = false;
   m_ct = CT_INTERPRET;
   m_posCur = cb.getRunPos();
   m_posEnd = cb.getEndPos();
   m_top = top;
   m_base = base;
   if (isInteractiveTrace) {
      m_signals |= SIG_DISABLED;  // trap everything
   }
}

/*-------------------------------------------------------------Routines::save-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Routines::save()
{
   if (++m_ixCur > m_aRtneSize) {
      int oldsize = m_aRtneSize;
      m_aRtneSize += ROUTINE_INC;
      m_aRtne = (Routine*) realloc(m_aRtne, m_aRtneSize * sizeof (Routine));
   }
   new(m_aRtne+m_ixCur-1) Routine(*this);   // save current routine
}

/*----------------------------------------------------------Routines::restore-+
| IMPLEMENTATION: use slicing (by ARM 12.8)                                   |
+----------------------------------------------------------------------------*/
void Routines::restore()
{
   Routine * that = this;     // don't stress too much poor Microsoft
   that->~Routine();                        // clean-up current
   new(that) Routine(m_aRtne[--m_ixCur]);   // restore from save
   m_aRtne[m_ixCur].~Routine();             // clean-up save
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
