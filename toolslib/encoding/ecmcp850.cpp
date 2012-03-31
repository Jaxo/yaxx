/*
* $Id: ecmcp850.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for CP850
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"
#include "cp850.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------EncodingModule_CP850::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_CP850::maxMbSize() const {
   return 1;
}

/*----------------------------------------EncodingModule_CP850::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_CP850::unicodeLength(
   unsigned char const *pchText,
   int iSizeText
) {
   return iSizeText;
}

/*---------------------------------------EncodingModule_CP850::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_CP850::target2unicode(
   unsigned char const ** ppchSource,
   unsigned int iSizeSource,
   UCS_2 ** ppUcTarget,
   unsigned int iSizeTarget
) {
   unsigned char const * pchSource = *ppchSource;
   unsigned char const * const pchSourceEnd = pchSource + iSizeSource;
   UCS_2 * pUcTarget = *ppUcTarget;
   UCS_2 * const pUcTargetEnd = pUcTarget + iSizeTarget;

   while ((pchSource < pchSourceEnd) && (pUcTarget < pUcTargetEnd)) {
      *pUcTarget++ = tounicode[*pchSource++];
   }
   *ppchSource = pchSource;
   *ppUcTarget = pUcTarget;
   if (pchSource == pchSourceEnd) {
      return OK;
   }
   return TOOSMALL;
}

/*---------------------------------------EncodingModule_CP850::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_CP850::unicode2target(
   UCS_2 const ** ppUcSource,
   unsigned int iSizeSource,
   unsigned char ** ppchTarget,
   unsigned int iSizeTarget
) {
   RC rc = TOOSMALL;
   UCS_2 const * pUcSource = *ppUcSource;
   UCS_2 const * const pUcSourceEnd = pUcSource + iSizeSource;
   unsigned char * pchTarget = *ppchTarget;
   unsigned char * const pchTargetEnd = pchTarget + iSizeTarget;

   while ((pUcSource < pUcSourceEnd) && (pchTarget < pchTargetEnd)) {
      UCS_2 ucSource = *pUcSource++;
      if (ucSource <= 0xFF) {
         /* Handle characters inside Code Page 850 range */
         unsigned char chCurrent = tonative[ucSource];
         if (chCurrent != MULTIBYTE_SUBSTITUTE  ||
             ucSource == MULTIBYTE_SUBSTITUTE) {  /* U+001A itself valid */
            *pchTarget++ = chCurrent;
            continue;
         }
      }else if ((ucSource >= 0x2500) && (ucSource <= 0x25A0)) {
         /*
         | Use a conversion table to handle characters from the range
         | 0x2500 through 0x25A0.
         */
         unsigned char chCurrent = graphictonative[ucSource - 0x2500];
         if (chCurrent != MULTIBYTE_SUBSTITUTE) {
            *pchTarget++ = chCurrent;
            continue;
         }
      }else if (ucSource == 0x0131) {
         /*
         | Special case LATIN SMALL LETTER DOTLESS I
         | which doesn't fall within a single character range and is
         | therefore not in the graphictonative conversion table.
         */
         *pchTarget++ = 0xd5;
         continue;
      }else if (ucSource == 0x0192) {
         /*
         | Special case LATIN SMALL LETTER F WITH HOOK
         | which doesn't fall within a single character range and is
         | therefore not in the graphictonative conversion table.
         */
         *pchTarget++ = 0x9f;
         continue;
      }else if (ucSource == 0x2017) {
         /*
         | Special case DOUBLE LOW LINE
         | which doesn't fall within a single character range and is
         | therefore not in the graphictonative conversion table.
         */
         *pchTarget++ = 0xf2;
         continue;
      }
      --pUcSource;                             /* backup */
      rc = INVALID;        /* invalid. */
      break;
   }
   *ppUcSource = pUcSource;
   *ppchTarget = pchTarget;
   if (pUcSource == pUcSourceEnd) {
      return OK;
   }
   return rc;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
