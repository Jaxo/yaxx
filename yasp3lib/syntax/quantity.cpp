/*
* $Id: quantity.cpp,v 1.1 2002-01-17 06:57:44 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include "quantity.h"

/*--------------------------------------------------------------------Quantity-+
| Constructor from scratch (i.e Ref Concrete Syntax)                           |
+-----------------------------------------------------------------------------*/
Quantity::Quantity()
{
   qntytab[IX_ATTCNT]    =  ATTCNT_RCS;
   qntytab[IX_ATTSPLEN]  =  ATTSPLEN_RCS;
   qntytab[IX_BSEQLEN]   =  BSEQLEN_RCS;
   qntytab[IX_DTAGLEN]   =  DTAGLEN_RCS;
   qntytab[IX_DTEMPLEN]  =  DTEMPLEN_RCS;
   qntytab[IX_ENTLVL]    =  ENTLVL_RCS;
   qntytab[IX_GRPCNT]    =  GRPCNT_RCS;
   qntytab[IX_GRPGTCNT]  =  GRPGTCNT_RCS;
   qntytab[IX_GRPLVL]    =  GRPLVL_RCS;
   qntytab[IX_LITLEN]    =  LITLEN_RCS;
   qntytab[IX_NAMELEN]   =  NAMELEN_RCS;
   qntytab[IX_NORMSEP]   =  NORMSEP_RCS;
   qntytab[IX_PILEN]     =  PILEN_RCS;
   qntytab[IX_TAGLEN]    =  TAGLEN_RCS;
   qntytab[IX_TAGLVL]    =  TAGLVL_RCS;
}

/*-------------------------------------------------------------------------set-+
| Set new quantity values                                                      |
+-----------------------------------------------------------------------------*/
bool Quantity::set(e_Ix ix, int value)
{
   int min = 1;
   int max = 1;

   /*-----------------------+
   | limit each quantity to |
   | a specific min/max     |
   +-----------------------*/
   switch (ix) {
   case IX_ATTSPLEN:
      max = ATTSPLEN_MAX;
      break;
   case IX_ATTCNT:
      max = ATTCNT_MAX;
      break;
   case IX_LITLEN:     // Check compatibility with BUFSIZE!
      min = NAMELEN_MAX;
      max = LITLEN_MAX;
      break;
   case IX_NAMELEN:
      max = NAMELEN_MAX;
      break;
   case IX_NORMSEP:
      max = NORMSEP_MAX;
      break;
   case IX_PILEN:
      max = PILEN_MAX;
      break;
   case IX_TAGLEN:
      max = TAGLEN_MAX;
      break;
   case IX_GRPCNT:
      max = GRPCNT_MAX;
      break;
   case IX_ENTLVL:
      max = ENTLVL_MAX;
      break;
   case IX_GRPGTCNT:
      max = GRPGTCNT_MAX;
      break;
   case IX_TAGLVL:
      max = TAGLVL_MAX;
      break;
   case IX_BSEQLEN:    // Check compatibility with BUFSIZE!
   case IX_DTAGLEN:
   case IX_DTEMPLEN:
   case IX_GRPLVL:
   default:
      min = max = qntytab[ix]; // i.e.: cannot change those...
      break;
   }
   if ((value < min) || (value > max)) return false;

   qntytab[ix] = value;
   return true;
}
/*===========================================================================*/
