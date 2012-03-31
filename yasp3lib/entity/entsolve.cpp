/*
* $Id: entsolve.cpp,v 1.2 2002-01-25 11:45:09 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include "entsolve.h"

/*-----------------------------------------EntitySolverBase::EntitySolverBase-+
| Virtual destructor                                                          |
+----------------------------------------------------------------------------*/
EntitySolverBase::~EntitySolverBase() {
  /* empty */
}

int EntitySolverBase::isOpenable(Entity const &) {
  return 1;
}

ostream * EntitySolverBase::getClone(Entity const & ent) const {
   return 0;
}

EncodingModule::Id EntitySolverBase::inqEncoding(Entity const & ent) const {
   return EncodingModule::ISO_8859;
}

/*===========================================================================*/
