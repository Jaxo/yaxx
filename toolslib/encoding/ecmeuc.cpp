/*
* $Id: ecmeuc.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for Japanese EUC.
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"
#include "euc2uc.h"
#include "uc2euc.h"


#define LE3(min, x, max)        (((min) <= (x)) && ((x) <= (max)))
#define isEUC_Zenkaku(b)        (((b) == 0x8F) || LE3(0xA1, (b), 0xFE))
#define isEUC_ZenkakuTail(b)    (LE3(0xA1, (b), 0xFE))
#define isEUC_Hankaku(b)        ((b) == 0x8E)
#define isEUC_HankakuTail(b)    (LE3(0xA1, (b), 0xDF)

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------EncodingModule_EUC::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_EUC::maxMbSize() const {
   return 2;
}

/*-----------------------------------------EncodingModule_EUC::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_EUC::unicodeLength(
   unsigned char const *pchText,
   int iSizeText
) {
   unsigned char const * const pchTextEnd = pchText + iSizeText;
   int iReduced = 0;

   while (pchText < pchTextEnd) {
      if (UCS_MBSTART == EUC2UC_SB[*pchText++]) {
         /* if (InRange(0x8E, 0x8F) || InRange(0xA1, 0xF4)) */
         if (pchText++ >= pchTextEnd) {
            break;
         }
         ++iReduced;
      }
   }
   return iSizeText - iReduced;
}

/*----------------------------------------EncodingModule_EUC::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_EUC::target2unicode(
   unsigned char const ** ppchSource,
   unsigned int iSizeSource,
   UCS_2 ** ppUcTarget,
   unsigned int iSizeTarget
) {
   RC rc = TOOSMALL;
   unsigned char const * pchSource = *ppchSource;
   unsigned char const * const pchSourceEnd = pchSource + iSizeSource;
   UCS_2 * pUcTarget = *ppUcTarget;
   UCS_2 * const pUcTargetEnd = pUcTarget + iSizeTarget;
   unsigned int low, high;
   int ix;
   UCS_2 ucCurrent;

   while ((pchSource < pchSourceEnd) && (pUcTarget < pUcTargetEnd)) {
      UCS_2 ucValue;
      high = *pchSource++;
      ucCurrent = EUC2UC_SB[high];
      if (ucCurrent != UCS_MBSTART) {
         *pUcTarget++ = ucCurrent;
         continue;
      }
      /* if (InRange(0x8E, 0x8F) || InRange(0xA1, 0xF4)) */
      if (pchSource >= pchSourceEnd) {          /* If no byte follows, */
         --pchSource;                           /* back to where started */
         rc = NOTWHOLE;                         /* Truncated. */
         break;
      }

      low = *pchSource++;

      /* Checks if 2nd half byte of half-width katakana is valid.
         Half-width katakana in EUC is two bytes */
      if (high == 0x8e && (low < 0xa1 || low > 0xdf)) {
         pchSource -= 2;                        /* Backup */
         rc = INVALID;                          /* Invalid. */
         break;
      }
      /* Checks if 2nd half byte of Multi-byte character is valid */
      if (low < 0xa1 || low > 0xfe) {
         pchSource -= 2;                        /* Backup */
         rc = INVALID;                          /* Invalid. */
         break;
      }
      if (high == 0x8e) {
         high = 0;                              /* -= 0x8e; */
      }else {
         high -= 0xa0;
      }
      ix = (high * 94) + low - 0xa1;
      if ((ix < 0) || (ix >= MBTBLSZ)) {        /* TEMP FIX: BUG IN TABLE ?? */
         pchSource -= 2;                        /* Backup */
         rc = INVALID;                          /* Invalid. */
         break;
      }
      ucValue = EUC2UC_DB[ix];
      if (ucValue == UCS_UNDEFINED) {
         pchSource -= 2;                        /* Backup */
         rc = INVALID;                          /* Invalid. */
         break;
      } else {
         *pUcTarget++ = ucValue;
      }
   }
   *ppchSource = pchSource;
   *ppUcTarget = pUcTarget;
   if (pchSource == pchSourceEnd) {
      return OK;
   }
   return rc;
}

/*-----------------------------------------EncodingModule_EUC::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_EUC::unicode2target(
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
      UCS_2 ucValue;
      if (*pUcSource == 0) {
         pUcSource++;
         *pchTarget++ = 0;
         continue;
      }
      ucValue = UC2EUC_DB[*pUcSource++];
      if (ucValue == UCS_UNDEFINED) {
         --pUcSource;
         rc = INVALID;
         break;
      }
      if (ucValue <= 0xFF) {
         *pchTarget++ = (unsigned char)ucValue;
      }else {
         if (pchTarget+1 >= pchTargetEnd) {     /* Yes: check overflow. */
            --pUcSource;
            break;
         }
         *pchTarget++ = (ucValue >> 8);
         *pchTarget++ = (unsigned char)ucValue;
      }
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
