/*
* $Id: solver.cpp,v 1.5 2002-04-01 10:21:21 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include <iostream.h>
#include <fstream.h>
#include "../reslib/HtmlDtd.h"
#include "../yasp3lib/entity/entity.h"
#include "solver.h"

/*------------------------------------------------------------Solver::resolve-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream * Solver::resolve(
   UnicodeString path,
   UnicodeString const & basePath
) {
   if (path.good()) {
      UCS_2 const * p0 = basePath;
      for (UCS_2 const * p = p0 + basePath.length(); p > p0; --p) {
         if ((*p == (UCS_2)'/') || (*p == (UCS_2)'\\')) {
            path = basePath(0, p-basePath+1) + path; // insert the directory
            break;
         }
      }
      return  new ifstream(
         MultibyteString(path, encLocale).inqString(),
         ios::in | ios::binary | ios::nocreate
      );
   }
   return 0;
}

/*------------------------------------------------Solver::getExternalProducer-+
| Effects:                                                                    |
|    Get an appropriate istream to read an external entity.                   |
| Returns:                                                                    |
|    Pointer to the stream or 0                                               |
+----------------------------------------------------------------------------*/
istream * Solver::getExternalProducer(Entity const & ent)
{
   if (ent.isSpecialDoc()) {
      return new ifstream(
         mbsPrimaryPath.inqString(),
         ios::in | ios::binary | ios::nocreate
      );
   }else if (ent.isSpecialDtd() && mbsDtdPath.good()) {
      return new ifstream(
         mbsDtdPath.inqString(),
         ios::in | ios::binary | ios::nocreate
      );
   }else {
      istream * is = resolve(
         ent.inqExternId().inqSystemId(),
         mbsPrimaryPath
      );

      if ((!is || !is->good()) && ent.isSpecialDtd() /* and it is HTML */) {
         ysp << YSP__HTML;
         return new MemStream(
            getHtmlDtd(), getHtmlDtdLength(), MemStream::Constant
            /*
            | If you want to test the minimal DTD, use:
            | "<ELEMENT HTML O O ANY>", 22, MemStream::Constant
            */
         );
      }
      return is;
   }
}

/*----------------------------------------------------Solver::inqAbsolutePath-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString const & Solver::inqAbsolutePath(Entity const & ent) const {
   if (ent.isSpecialDoc()) {
      ((Solver *)this)->hack = mbsPrimaryPath.inqString();
   }else if (ent.isSpecialDtd()) {
      if (mbsDtdPath.good()) {
         ((Solver *)this)->hack = mbsDtdPath.inqString();
      }else {
         ((Solver *)this)->hack = "[Internal HTML]";
      }
   }else {
      ExternId const & exid = ent.inqExternId();
      MultibyteString const mbsSystemId(exid.inqSystemId(), encLocale);
      ((Solver *)this)->hack = mbsSystemId.inqString();
   }
   return hack;
}

/*--------------------------------------------------------Solver::inqEncoding-+
| Effects:                                                                    |
|    Get the encoding attached to an entity.                                  |
| Note:                                                                       |
|   THIS IS NOT YET IMPLEMENTED.                                              |
+----------------------------------------------------------------------------*/
EncodingModule::Id Solver::inqEncoding(Entity const & ent) const
{
   if (ent.isSpecialDoc()) {
      return encDefault.inqEncoding();
   }
   assert (ent.isExternal());
   return EncodingModule::ISO_8859;  // not implemented.
}

/*-----------------------------------------------------------Solver::getClone-+
| Effects:                                                                    |
|    If we're cloning the input with PIs, set up output file.                 |
| Returns:                                                                    |
|    ostream pointer or 0                                                     |
| Note:                                                                       |
|   This can be called ONLY for an external entity!                           |
+----------------------------------------------------------------------------*/
ostream * Solver::getClone(Entity const & ent) const
{
   return 0;
}

/*===========================================================================*/
