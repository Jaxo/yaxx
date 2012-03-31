/*
* $Id: ecmucs2.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for UCS-2.
* This does a 1 to 1 conversion.
* The source "bytes" happen to be UCS-2.
*/

/*---------+
| Includes |
+---------*/
#include <string.h>
#include "ecm.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------EncodingModule_UCS2::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_UCS2::maxMbSize() const {
   return sizeof (UCS_2);
}

/*-----------------------------------------EncodingModule_UCS2::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_UCS2::unicodeLength(
   unsigned char const *pchText,
   int iSizeText
) {
   return (iSizeText/sizeof(UCS_2)) + ((iSizeText % sizeof(UCS_2))? 1 : 0);
}

/*----------------------------------------EncodingModule_UCS2::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_UCS2::target2unicode(
   unsigned char const ** ppchSource,
   unsigned int iSizeSource,
   UCS_2 ** ppUcTarget,
   unsigned int iSizeTarget
) {
   RC rc;
   char * pchTarget = (char *)(*ppUcTarget);
   unsigned int iByteSize = iSizeTarget * sizeof(UCS_2);

   if (iSizeSource > iByteSize) {
      rc = TOOSMALL;
   }else {
      int iRemain = iSizeSource % sizeof(UCS_2);
      if (iRemain) {       /* we have to round down */
         iByteSize = iSizeSource - iRemain;
         rc = NOTWHOLE;
      }else {
         rc = OK;
         iByteSize = iSizeSource;
      }
   }
   memcpy(pchTarget, *ppchSource, iByteSize);
   *ppUcTarget = (UCS_2 *)(pchTarget + iByteSize);
   *ppchSource += iByteSize;
   return rc;
}

/*----------------------------------------EncodingModule_UCS2::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_UCS2::unicode2target(
   UCS_2 const ** ppUcSource,
   unsigned int iSizeSource,
   unsigned char ** ppchTarget,
   unsigned int iSizeTarget
) {
   RC rc;
   char const * pchSource = (char const *)(*ppUcSource);
   unsigned int iByteSize = iSizeSource * sizeof(UCS_2);

   if (iByteSize > iSizeTarget) {
      iByteSize = iSizeTarget - (iSizeTarget % sizeof(UCS_2)); /* round */
      rc = TOOSMALL;
   }else {
      rc = OK;
   }
   memcpy(*ppchTarget, pchSource, iByteSize);
   *ppUcSource = (UCS_2 *)(pchSource + iByteSize);
   *ppchTarget += iByteSize;
   return rc;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
