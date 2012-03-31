/*
* $Id: entsolve.h,v 1.7 2011-07-29 10:26:39 pgr Exp $
*
* Entity Solver: maps an entity to an istream
*/

#if !defined ENTSOLVE_HEADER && defined __cplusplus
#define ENTSOLVE_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "../../toolslib/encoding/ecm.h"
// class istream;
// class ostream;
class Entity;
class UnicodeString;

/*-----------------------------------------------------------EntitySolverBase-+
| Pure abstract class that all Entity Solvers must derive from.               |
+----------------------------------------------------------------------------*/
class YASP3_API EntitySolverBase {
public:
   // given an entity, return 1 if you plan to open it - 0 if you don't
   // TEMP (PGR, 04/22/98): not a pure virtual: default is to return 1
   virtual int isOpenable(Entity const & ent);

   // given an entity, return a pointer to an istream or 0 if not found.
   // besides regular external entities, two special kinds of entities,
   // the document to parse itself, and the document type declaration.
   // these are recognizable by "ent.isSpecialDoc()" and
   // "ent.isSpecialDtd()".
   // - the document has no system id ("!ent.inqExternId().inqSystemId()".)
   // - the name of the DTD entity is the base element.  The base element
   //   can be changed using: Yasp3::setDoctype.
   virtual istream * getExternalProducer(Entity const & ent) = 0;

   // "clones" are ostream where to write modifications for a given
   // entity.  It is used, for example, to insert version changes PI's.
   virtual ostream * getClone(Entity const & ent) const;

   // the encoding of the entity can be found in bound data,
   // like mime headers, etc...  The Solver's answer (if not NULL)
   // takes precedence over other internal methods, such as
   // High Order Byte recognition.
   // Basic Unicode has 2 encoders: ucs2, and ucs4!
   virtual EncodingModule::Id inqEncoding(Entity const & ent) const;

   // this method is mainly used to furnish "readable" information
   // to the human beings.
   virtual UnicodeString const & inqAbsolutePath(Entity const & ent) const = 0;

   // as usual:
   virtual ~EntitySolverBase();

   // from there you are on your own....
};

#endif /* ENTSOLVE_HEADER ===================================================*/
