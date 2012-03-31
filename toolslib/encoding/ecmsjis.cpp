/*
* $Id: ecmsjis.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for Japanese Shift-JIS.
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"
#include "sjis2uc.h"
#include "uc2sjis.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------EncodingModule_SJIS::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_SJIS::maxMbSize() const {
   return 2;
}

/*-----------------------------------------EncodingModule_SJIS::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_SJIS::unicodeLength(
   unsigned char const * pchText,
   int iSizeText
) {
   unsigned char const * const pchTextEnd = pchText + iSizeText;
   int iReduced = 0;

   while (pchText < pchTextEnd) {
      if (UCS_MBSTART == SJIS2UC_SB[*pchText++]) {
         if (pchText++ >= pchTextEnd) {
            break;
         }
         ++iReduced;
      }
   }
   return iSizeText - iReduced;
}

/*----------------------------------------EncodingModule_SJIS::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_SJIS::target2unicode(
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
   UCS_2 ucCurrent;

   while ((pchSource < pchSourceEnd) && (pUcTarget < pUcTargetEnd)) {
      UCS_2 ucValue;
      high = *pchSource++;
      ucCurrent = SJIS2UC_SB[high];
      if (ucCurrent != UCS_MBSTART) {
         *pUcTarget++ = ucCurrent;
         continue;
      }
      if (pchSource >= pchSourceEnd) {            /* If no byte follows, */
         --pchSource;                             /* back to where started */
         rc = NOTWHOLE;                           /* Truncated. */
         break;
      }
      low = *pchSource++;
      /* Checks if 2nd half byte of Multi-byte character is valid */
      if (low < 0x40 || low == 0x7f || low > 0xfc)
      {
         pchSource -= 2;                          /* Backup */
         rc = INVALID;                            /* Invalid. */
         break;
      }
      if (high <= 0x9f) {
         int ix;
         if (low <= 0x7f) {
            ix = (high * 0xbc) + low - 0x5efc;
         }else {
            ix = (high * 0xbc) + low - 0x5efd;
         }
         if (ix < 0) {
            pchSource -= 2;                       /* Backup */
            rc = INVALID;                         /* Invalid. */
            break;
         }
         ucValue = SJIS2UC_DB[ix];
      }else {
         int ix;
         if (low <= 0x7f) {
            ix = (high * 0xbc) + low - 0x8dfc;
         }else {
            ix = (high * 0xbc) + low - 0x8dfd;
         }
         if (ix >= MBTBLSZ) {                  /* Table Upper bound check */
            pchSource -= 2;                    /* Backup */
            rc = INVALID;                      /* Invalid. */
            break;
         }
         ucValue = SJIS2UC_DB[ix];
      }
      if (ucValue == UCS_UNDEFINED) {
         pchSource -= 2;                       /* Backup */
         rc = INVALID;                         /* Invalid. */
         break;
      }
      *pUcTarget++ = ucValue;
   }
   *ppchSource = pchSource;
   *ppUcTarget = pUcTarget;
   if (pchSource == pchSourceEnd) {
      return OK;
   }
   return rc;
}

/*----------------------------------------EncodingModule_SJIS::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_SJIS::unicode2target(
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
      ucValue = UC2SJIS_DB[*pUcSource++];
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
