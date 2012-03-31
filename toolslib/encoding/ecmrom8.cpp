/*
* $Id: ecmrom8.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for HP Roman8.
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"
#include "roman8.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------EncodingModule_ROM8::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_ROM8::maxMbSize() const {
   return 1;
}

/*-----------------------------------------EncodingModule_ROM8::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_ROM8::unicodeLength(
   unsigned char const *pchSource,
   int iSizeSource
) {
    return iSizeSource;
}

/*----------------------------------------EncodingModule_ROM8::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_ROM8::target2unicode(
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

/*----------------------------------------EncodingModule_ROM8::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_ROM8::unicode2target(
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
         /* Handle characters inside Roman-8 range */
         unsigned char chCurrent = tonative[ucSource];
         if (chCurrent == MULTIBYTE_SUBSTITUTE &&
             ucSource != MULTIBYTE_SUBSTITUTE) {  /* U+001A itself valid */
            --pUcSource;                             /* backup */
            rc = INVALID;        /* invalid. */
            break;
         }
         *pchTarget++ = chCurrent;
      }else if (ucSource == 0x0160) {
         /*
         | "latin capital letter s with hacek" which doesn't
         | fall within a single character range and is therefore not in
         | the standard conversion table.
         */
         *pchTarget++ = 0xeb;
      }else if (ucSource == 0x0161) {
         /*
         | "latin small letter s with hacek" which doesn't
         | fall within a single character range and is therefore not in
         | the standard conversion table.
         */
         *pchTarget++ = 0xec;
      }else if (ucSource == 0x0178) {
         *pchTarget++ = 0xee;         /* latin capital letter y with diaeresis */
      }else if (ucSource == 0x0192) {
         *pchTarget++ = 0xbe;         /* latin small letter f with hook */
      }else if (ucSource == 0x0300) {
         *pchTarget++ = 0xa9;         /* non-spacing grave */
      }else if (ucSource == 0x0301) {
         *pchTarget++ = 0xa8;         /* non-spacing acute */
      }else if (ucSource == 0x0302) {
         *pchTarget++ = 0xaa;         /* non-spacing circumflex */
      }else if (ucSource == 0x0303) {
         *pchTarget++ = 0xac;         /* non-spacing tilde */
      }else if (ucSource == 0x0308) {
         *pchTarget++ = 0xab;         /* non-spacing diaeresis */
      }else if (ucSource == 0x20a4) {
         *pchTarget++ = 0xaf;         /* lira sign */
      }else if (ucSource == 0x25a0) {
         *pchTarget++ = 0xfc;         /* black square */
      }else if (ucSource == 0xffff) {
         *pchTarget++ = 0xff;         /* not a character */
      }else {
         --pUcSource;                             /* backup */
         rc = INVALID;        /* invalid. */
         break;
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
