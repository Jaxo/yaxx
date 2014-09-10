/* $Id: Label.h,v 1.16 2002-05-14 15:15:33 pgr Exp $ */

#ifndef COM_JAXO_YAXX_LABEL_H_INCLUDED
#define COM_JAXO_YAXX_LABEL_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "RexxString.h"
#include "CodeBuffer.h"
#include "HashMap.h"
#include "BuiltIn.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class LabelMap;

/*-------------------------------------------------------------- class Label -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Label : public MapItem {
public:
   enum Type {
      LABEL,
      FUNCTION,
      BUILTIN,
      INTERNAL,
      EXTERNAL,
      SYSTEM
   };

   Label(RexxString const & name, Type type, CodePosition pos);
   void resolveOffset(CodePosition pos);
   bool isBuiltIn(bool bypassInternals) const  {
     return ((m_type == BUILTIN) || (bypassInternals && (m_func != 0)));
   }
   bool isExternal(bool bypassInternals = false) const {
     return (bypassInternals || (m_pos == CodeBuffer::Unknown));
   }
   bool isInternal() const {
     return (m_pos != CodeBuffer::Unknown);
   }
   CodePosition getPos() const        { return m_pos; }
   BuiltInFunction getBuiltInFunction() const { return m_func; }

private:
   enum Type m_type;           // function type
   BuiltInFunction m_func;     // builtin function
   CodePosition m_pos;         // pos in code
};

/*----------------------------------------------------------- class LabelMap -+
|                                                                             |
+----------------------------------------------------------------------------*/
class REXX_API LabelMap : private HashMap {
public:
   Label * put(RexxString const & name, Label::Type type, CodePosition pos) {
      return (Label *)HashMap::put(new Label(name, type, pos));
   }
   Label * get(RexxString const & name) {
      return (Label *)HashMap::get(name);
   }
   Label * access(RexxString const & name, Label::Type type, CodePosition pos);
   using HashMap::print;
};

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
