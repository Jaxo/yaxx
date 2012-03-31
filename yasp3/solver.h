/*
* $Id: solver.h,v 1.5 2002-04-01 10:21:21 pgr Exp $
*/

#if !defined SOLVER_HEADER && defined __cplusplus
#define SOLVER_HEADER

/*---------+
| Includes |
+---------*/
#include "../toolslib/ucstring.h"
#include "../toolslib/mbstring.h"
#include "../toolslib/tplistix.h"
#include "../toolslib/tpset.h"
#include "../yasp3lib/yasp3.h"
#include "../yasp3lib/entity/entsolve.h"

class Entity;
class UnicodeComposer;

/*---------------------------------------------------------------------Solver-+
|                                                                             |
+----------------------------------------------------------------------------*/
class Solver : public EntitySolverBase {
public:
   /*---------------+
   | Public Methods |
   +---------------*/
   Solver(
      UnicodeString const & ucsPrimaryPathArg,
      UnicodeString const & ucsDtdPathArg,
      Yasp3 & yspArg
   ) :
      mbsPrimaryPath(ucsPrimaryPathArg, encLocale),
      mbsDtdPath(ucsDtdPathArg, encLocale),
      ysp(yspArg)
   {}

   // resolution of EntitySolverBase pure virtuals
   istream * getExternalProducer(Entity const & ent);
   ostream * getClone(Entity const & ent) const;
   EncodingModule::Id inqEncoding(Entity const & ent) const;
   UnicodeString const & inqAbsolutePath(Entity const & ent) const;

private:
   istream * Solver::resolve(
      UnicodeString path,
      UnicodeString const & basePath
   );
   Encoder encDefault;
   Encoder encLocale;
   Yasp3 & ysp;
   UnicodeString hack;
   MultibyteString mbsPrimaryPath;
   MultibyteString mbsDtdPath;
};

#endif /* SOLVER_HEADER =====================================================*/
