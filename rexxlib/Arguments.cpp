/* $Id: Arguments.cpp,v 1.36 2002-04-28 17:07:48 pgr Exp $ */

#include "Arguments.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-------------------------------------------------------Arguments::Arguments-+
| Constructor for the main program                                            |
|                                                                             |
| IMPLEMENTATION:                                                             |
|   m_r(strArgs)  e.g.: "m_r refers strArgs".  This is just for convenience.  |
|   The main program is not supposed to return anything? Hmmm...              |
|   Consider using the top top top of the stack to return what                |
|   the CT_PROGRAM returned (generally a numeric return code.)                |
|   Also: why is this strArgs the blank-sep "char ** argv"                    |
|   Isn't CT_PROGRAM like anything else?                                      |
|   For today (11/07/2001) PGR has too much on his plate for taking care.     |
+----------------------------------------------------------------------------*/
Arguments::Arguments(RexxString & strArgs) : m_r(strArgs)
{
   memset(m_a, 0, sizeof m_a);
   if (strArgs.length() > 0) {
      m_a[0] = &strArgs;
      m_n = 1;
   }else {
      m_n = 0;
   }
}

/*-------------------------------------------------------Arguments::Arguments-+
| Constructor for a call (subroutine, internal function, or built-in function)|
+----------------------------------------------------------------------------*/
Arguments::Arguments(
   RexxString ** stack,             // stack
   PresenceBits presenceBits,
   int & base                  // initially: top of the stack (updated)
) :
   m_r(init(stack, presenceBits, base)) {
}

/*------------------------------------------------------------Arguments::init-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString & Arguments::init(
   RexxString ** stack,            // stack
   PresenceBits presenceBits,
   int & base                  // initially: top of the stack (updated)
) {
   m_n = -1;
   memset(m_a, 0, sizeof m_a);
   for (
     int pos = Arguments::MAXARGS-1;
     presenceBits != 0;
     presenceBits >>= 1, --pos
   ) {
      if (presenceBits & 1) {
         m_a[pos] = stack[base--];
         if (pos > m_n) m_n = pos;
      }
   }
   ++m_n;
   return *stack[base];
}

/*----------------------------------------------------Arguments::catenateArgs-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Arguments::catenateArgs(
   RexxString const & cmd,
   RexxString ** stack,
   PresenceBits presenceBits,
   int & base                            // initially: top of the stack
) {
   StringBuffer res(cmd, cmd.length());
   int top = base;
   while (presenceBits != 0) {
      if (presenceBits & 1) --base;
      presenceBits >>= 1;
   }
   for (int i=base+1; i <= top; ++i) {   // stack[base] is the result
      res.append(' ');
      res .append(*stack[i], stack[i]->length());
   }
   return res;
}


#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
