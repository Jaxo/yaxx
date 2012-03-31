/* $Id: Label.cpp,v 1.24 2002-05-14 15:15:33 pgr Exp $ */

#include "Label.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*---------------------------------------------------------------Label::Label-+
|                                                                             |
+----------------------------------------------------------------------------*/
Label::Label(
   RexxString const & name,
   Type type,
   CodePosition pos
) :
   MapItem(name)
{
   m_type = type;
   m_func = 0;
   m_pos = pos;

   if (type == Label::FUNCTION) {   /* can it be a builtin? */
      m_func = BuiltIn::getFunction(key());
      if (m_func) m_type = Label::BUILTIN;
   }
}

/*-------------------------------------------------------Label::resolveOffset-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Label::resolveOffset(CodePosition pos)
{
   if (m_pos == CodeBuffer::Unknown) {
      /* label found in the code: it is an Internal, not a Builtin */
      if (m_type == BUILTIN) {
         m_type = INTERNAL;
      }
      m_pos = pos;
   }
}

/*-----------------------------------------------------------LabelMap::access-+
| access means: get it, or, if it doesn't exist, create it                    |
+----------------------------------------------------------------------------*/
Label * LabelMap::access(
   RexxString const & name,
   Label::Type type,
   CodePosition pos
) {
   Label * label = get(name);
   if (!label) {
      label = put(name, type, pos);
   }else if (pos != CodeBuffer::Unknown) {
      label->resolveOffset(pos);
   }
   return label;
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
