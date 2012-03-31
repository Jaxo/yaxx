/*
* $Id: ecmc.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for ISO 8859-1.
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------EncodingModule_C::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_C::maxMbSize() const {
   return 1;
}

/*--------------------------------------------EncodingModule_C::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_C::unicodeLength(
   unsigned char const *pchSource,
   int iSizeSource
) {
   return iSizeSource;
}

/*-------------------------------------------EncodingModule_C::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_C::target2unicode(
   unsigned char const ** ppchSource,
   unsigned int iSizeSource,
   UCS_2 ** ppUcTarget,
   unsigned int iSizeTarget
) {
   RC rc = OK;
   unsigned char const * pchSource = *ppchSource;
   unsigned char const * const pchSourceEnd = pchSource + iSizeSource;
   UCS_2 * pchTarget = *ppUcTarget;
   UCS_2 const * const pchTargetEnd = pchTarget + iSizeTarget;

   while (pchSource < pchSourceEnd) {
      if (pchTarget >= pchTargetEnd) {
         rc = TOOSMALL;
         break;
      }
      *pchTarget++ = (UCS_2)(*pchSource++);
   }
   *ppchSource = pchSource;
   *ppUcTarget = pchTarget;
   return rc;
}

/*-------------------------------------------EncodingModule_C::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_C::unicode2target(
   UCS_2 const ** ppUcSource,
   unsigned int iSizeSource,
   unsigned char ** ppchTarget,
   unsigned int iSizeTarget
) {
   RC rc = OK;
   UCS_2 const * pchSource = *ppUcSource;
   UCS_2 const * const pchSourceEnd = pchSource + iSizeSource;
   unsigned char * pchTarget = *ppchTarget;
   unsigned char * const pchTargetEnd = pchTarget + iSizeTarget;

   while (pchSource < pchSourceEnd) {
      if (pchTarget >= pchTargetEnd) {
         rc = TOOSMALL;
         break;
      }else if (*pchSource > 0xFF) {
         rc = INVALID;
         break;
      }
      *pchTarget++ = (unsigned char)(*pchSource++);
   }
   *ppUcSource = pchSource;
   *ppchTarget = pchTarget;
   return rc;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
