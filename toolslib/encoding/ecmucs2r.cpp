/*
* $Id: ecmucs2r.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for UCS-2 Reverted.
* The source "bytes" happen to be UCS-2, endians flipped.
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"

#define revertEndians(s) (((s<<8) | ((s>> 8) & 0x00ff)) & 0x0000ffff);

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------EncodingModule_UCS2R::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_UCS2R::maxMbSize() const {
   return sizeof (UCS_2);
}

/*----------------------------------------EncodingModule_UCS2R::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_UCS2R::unicodeLength(
   unsigned char const *pchText,
   int iSizeText
) {
   return (iSizeText/sizeof(UCS_2)) + ((iSizeText % sizeof(UCS_2))? 1 : 0);
}

/*---------------------------------------EncodingModule_UCS2R::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_UCS2R::target2unicode(
   unsigned char const ** ppchSource,
   unsigned int iSizeSource,
   UCS_2 ** ppUcTarget,
   unsigned int iSizeTarget
) {
   RC rc;
   UCS_2 s;
   UCS_2 const * pUcSource = (UCS_2 const *)(*ppchSource);
   UCS_2 * pUcTarget = *ppUcTarget;
   unsigned int iUcs2Size = iSizeSource / sizeof(UCS_2);

   if (iUcs2Size > iSizeTarget) {
      rc = TOOSMALL;
      iUcs2Size = iSizeTarget;
   }else {
      if (iSizeSource % sizeof(UCS_2)) {
         if (iUcs2Size == iSizeTarget) {
            rc = TOOSMALL;
         }else {
            rc = NOTWHOLE;
         }
      }else {
         rc = OK;
      }
   }
   for (s = *pUcSource; iUcs2Size--; s = *++pUcSource) {
      *pUcTarget++ = revertEndians(s);
   }
   *ppUcTarget = pUcTarget;
   *ppchSource = (unsigned char const *)pUcSource;
   return rc;
}

/*---------------------------------------EncodingModule_UCS2R::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_UCS2R::unicode2target(
   UCS_2 const ** ppUcSource,
   unsigned int iSizeSource,
   unsigned char ** ppchTarget,
   unsigned int iSizeTarget
) {
   RC rc;
   UCS_2 s;
   UCS_2 const * pUcSource = *ppUcSource;
   UCS_2 * pUcTarget = (UCS_2 *)(*ppchTarget);
   unsigned int iUcs2Size = iSizeTarget / sizeof(UCS_2);

   if (iSizeSource > iUcs2Size) {
      rc = TOOSMALL;
   }else {
      iUcs2Size = iSizeSource;
      rc = OK;
   }
   for (s = *pUcSource; iUcs2Size--; s = *++pUcSource) {
      *pUcTarget++ = revertEndians(s);
   }
   *ppchTarget = (unsigned char *)pUcTarget;
   *ppUcSource = pUcSource;
   return rc;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
