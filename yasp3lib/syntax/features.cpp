/*
* $Id: features.cpp,v 1.1 2002-01-17 06:57:44 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include "features.h"

/*--------------------------------------------------------------------Features-+
| RCS features (?)                                                             |
+-----------------------------------------------------------------------------*/
Features::Features()
{
   bFormal   = 0;
   bConcur   = 0;
   bExplicit = 0;
   bImplicit = 0;
   bSimple   = 0;
   bSubdoc   = 0;
   bRank     = 0;
   bDatatag  = 0;
   bOmittag  = 1;
   bShorttag = 1;
   bEmptynrm = 0;
   bKeeprsre = 0;

   iMaxSubdoc   = 0;
   iMaxConcur   = 0;
   iMaxSimple   = 0;
   iMaxExplicit = 0;
}

/*-------------------------------------------------------------------------set-+
| Set a feature                                                                |
|                                                                              |
| iNumber is 0 if the feature has to be disabled                               |
+-----------------------------------------------------------------------------*/
Features::e_ReplaceCode Features::set(
   ReservedName::e_Ix ixFeatureName, unsigned int iNumber
)
{
   switch (ixFeatureName) {
   case ReservedName::IX_FORMAL:
      if (iNumber) return NOT_SUPPORTED;    // not yet, but will for EONS
      if (iNumber) bFormal =  1; else bFormal =  0;
      return OK;
   case ReservedName::IX_CONCUR:
      if (iNumber) return NOT_SUPPORTED;    // no one wants it
      if (iNumber) bConcur =  1; else bConcur =  0;
      if (iNumber > MAX_CONCUR) {
         iMaxConcur = MAX_CONCUR;
         return SET_TO_MAXIMUM;
      }
      iMaxConcur = iNumber;
      return OK;
   case ReservedName::IX_EXPLICIT:          // may be...
      if (iNumber) return NOT_SUPPORTED;
      if (iNumber) bExplicit =  1; else bExplicit =  0;
      if (iNumber > MAX_SUBDOC) {
         iMaxExplicit = MAX_EXPLICIT;
         return SET_TO_MAXIMUM;
      }
      iMaxExplicit = iNumber;
      return OK;
   case ReservedName::IX_IMPLICIT:          // may be...
      if (iNumber) return NOT_SUPPORTED;
      if (iNumber) bImplicit =  1; else bImplicit =  0;
      return OK;
   case ReservedName::IX_SIMPLE:            // may be...
      if (iNumber) return NOT_SUPPORTED;
      if (iNumber) bSimple =  1; else bSimple =  0;
      if (iNumber > MAX_SUBDOC) {
         iMaxSimple = MAX_SIMPLE;
         return SET_TO_MAXIMUM;
      }
      iMaxSimple = iNumber;
      return OK;
   case ReservedName::IX_SUBDOC:            // will be some day
      if (iNumber) return NOT_SUPPORTED;
      if (iNumber) bSubdoc  =  1; else bSubdoc  =  0;
      if (iNumber > MAX_SUBDOC) {
         iMaxSubdoc = MAX_SUBDOC;
         return SET_TO_MAXIMUM;
      }
      iMaxSubdoc = iNumber;
      return OK;
   case ReservedName::IX_RANK:             // beurrkkk... no one wants it
      if (iNumber) return NOT_SUPPORTED;
      if (iNumber) bRank =  1; else bRank =  0;
      return OK;
   case ReservedName::IX_DATATAG:          // beurrkkk... no one wants it
      if (iNumber) return NOT_SUPPORTED;
      if (iNumber) bDatatag =  1; else bDatatag =  0;
      return OK;
   case ReservedName::IX_OMITTAG:          // yes.
      if (iNumber) bOmittag =  1; else bOmittag =  0;
      return OK;
   case ReservedName::IX_SHORTTAG:         // yes.
      if (iNumber) bShorttag =  1; else bShorttag =  0;
      return OK;
   case ReservedName::IX_EMPTYNRM:         // yes.
      if (iNumber) bEmptynrm =  1; else bEmptynrm =  0;
      return OK;
   case ReservedName::IX_KEEPRSRE:         // yes.
      if (iNumber) bKeeprsre =  1; else bKeeprsre =  0;
      return OK;
   default:
      return NOT_A_FEATURE;
   }
}

/*------------------------------------------------------------------------code-+
| See appendix G                                                               |
+-----------------------------------------------------------------------------*/
unsigned int Features::code() const
{
   return
      (bFormal?     1 : 0) +
      (bConcur?     2 : 0) +
      (bExplicit?   4 : 0) +
      (bImplicit?   8 : 0) +
      (bSimple?    16 : 0) +
      (bSubdoc?    32 : 0) +
      (bRank?      64 : 0) +
      (bDatatag?  128 : 0) +
      (bOmittag?  256 : 0) +
      (bShorttag? 512 : 0);
}
/*============================================================================*/
