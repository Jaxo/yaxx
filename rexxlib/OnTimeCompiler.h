/* $Id: OnTimeCompiler.h,v 1.5 2002-05-21 18:01:33 pgr Exp $ */

#ifndef COM_JAXO_YAXX_ONTIMECOMPILER_H_INCLUDED
#define COM_JAXO_YAXX_ONTIMECOMPILER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Compiler.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*----------------------------------------------------- class OnTimeCompiler -+
| To interpret a string on the fly                                            |
+----------------------------------------------------------------------------*/
class OnTimeCompiler : public Compiler {
public:
   OnTimeCompiler(
      istream & input,
      CodeBuffer & cb,
      SymbolMap & mapSym,
      LabelMap & mapLabel,
      UnicodeComposer & erh
   );
private:
   void onNewClause();
   bool onNewLabel();
   void onEndOfFile(streampos endPos, int m_lineno);
};


#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
