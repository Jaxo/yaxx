/*
* $Id: ecmutf8.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*
* Encoding Conversion Module (ECM) for UTF-8.
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------EncodingModule_UTF8::maxMbSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_UTF8::maxMbSize() const {
   return 3;
}

/*-----------------------------------------EncodingModule_UTF8::unicodeLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int EncodingModule_UTF8::unicodeLength(
   unsigned char const *pchText,
   int iSizeText
) {
   #ifndef ECMUTF8__WANT_PRECIZE_UNICODELENGTH
   return iSizeText;

   #else
   unsigned char const * const pchTextEnd = pchText + iSizeText;
   int iReduced = 0;
   while (pchText < pchTextEnd) {
      int c = *pchText++;
      if (c < 0x80) {
         continue;
      }else if (c >= 0xC2) {
         if (pchText++ >= pchTextEnd) {
            break;
         }
         if (c < 0xE0) {           /* 2 bytes seq. */
            if ((*pchText & 0xC0) == 0x80) {
               ++iReduced;         /* one collapses */
               continue;
            }
         }else {                   /* 3 bytes? */
            if (pchText++ >= pchTextEnd) {
               break;
            }
            if (c == 0xE0) {
               if (((pchText[-1] & 0xE0) == 0xA0) &&
                  ((*pchText & 0xC0) == 0x80)
               ) {
                  iReduced += 2;   /* two collapse */
                  continue;
               }
            }else if (c < 0xF0) {
               if (((pchText[-1] & 0xC0) == 0x80) &&
                  ((*pchText & 0xC0) == 0x80)
               ) {
                  iReduced += 2;   /* two collapse */
                  continue;
               }
            }
            --pchText;             /* bad 3rd byte */
         }
         --pchText;                /* bad 2nd byte */
      }
   }
   return iSizeText - iReduced;
   #endif
}

/*----------------------------------------EncodingModule_UTF8::target2unicode-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_UTF8::target2unicode(
   unsigned char const ** ppchSource,
   unsigned int iSizeSource,
   UCS_2 ** ppUcTarget,
   unsigned int iSizeTarget
) {
   RC rc = OK;
   unsigned char const * pchSource = *ppchSource;
   unsigned char const * const pchSourceEnd = pchSource + iSizeSource;
   UCS_2 * pUcTarget = *ppUcTarget;
   UCS_2 * const pUcTargetEnd = pUcTarget + iSizeTarget;
   unsigned int c0, c, r;                       /* unsigned int for UCS-4 ! */

   while (pchSource < pchSourceEnd) {
      if (pUcTarget >= pUcTargetEnd) {
         rc = TOOSMALL;
         break;
      }
      if ((c0 = *pchSource++) < 0x80) {         /* If a 1 byte sequence, */
         *pUcTarget++ = (UCS_2)c0;              /* set the value */
         continue;                              /* success. */
      }
      if (pchSource >= pchSourceEnd) {          /* If no byte follows, */
         --pchSource;                           /* back to where started */
         rc = NOTWHOLE;                         /* Truncated. */
         break;
      }
      c = (unsigned int)(*pchSource++ ^ 0x80);  /* 2nd byte */
      if (c & 0xC0) {                           /* starts new sequence? */
         pchSource -= 2;                        /* Yes, backup */
         rc = INVALID;                          /* Invalid. */
         break;
      }
      r = (c0 << 6) | c;                        /* Get encoded value */
      if ((c0 & 0xE0) == 0xC0) {                /* 2 bytes sequence? */
         r &= 0x7FF;                            /* Yes.  Cut noise */
         if (r >= 0x80) {                       /* If conforming, */
            *pUcTarget++ = (UCS_2)r;            /* set the value */
            continue;                           /* Success. */
         }
         pchSource -= 2;                        /* Not conform: backup */
         rc = INVALID;                          /* Invalid. */
         break;
      }
      if (pchSource >= pchSourceEnd) {          /* If no byte follows, */
         pchSource -= 2;                        /* back to where started */
         rc = NOTWHOLE;                         /* Truncated. */
         break;
      }
      c = (unsigned int)(*pchSource++ ^ 0x80);  /* 3rd byte */
      if (c & 0xC0) {                           /* starts new sequence? */
         pchSource -= 3;                        /* Yes, backup */
         rc = INVALID;                          /* Invalid. */
         break;
      }
      r = (r << 6) | c;                         /* Get encoded value */
      if ((c0 & 0xF0) == 0xE0) {                /* 3 bytes sequence? */
         /*-- r &= 0xFFFF; -- useless: no noise to cut for UCS-2 -- */
         if (r >= 0x800) {                      /* If conforming? */
            *pUcTarget++ = (UCS_2)r;            /* set the value */
            continue;                           /* Success. */
         }                                      /* Not conform, */
      }                                         /* or > 3 for UCS-2 */
      pchSource -= 3;                           /* Backup */
      rc = INVALID;                             /* Invalid. */
      break;
   }
   *ppchSource = pchSource;
   *ppUcTarget = pUcTarget;
   return rc;
}

/*----------------------------------------EncodingModule_UTF8::unicode2target-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::RC EncodingModule_UTF8::unicode2target(
   UCS_2 const ** ppUcSource,
   unsigned int iSizeSource,
   unsigned char ** ppchTarget,
   unsigned int iSizeTarget
) {
   RC rc = OK;
   UCS_2 const * pUcSource = *ppUcSource;
   UCS_2 const * const pUcSourceEnd = pUcSource + iSizeSource;
   unsigned char * pchTarget = *ppchTarget;
   unsigned char * const pchTargetEnd = pchTarget + iSizeTarget;
   unsigned int r;                              /* unsigned int for UCS-4 ! */

   while (pUcSource < pUcSourceEnd) {
      if (pchTarget >= pchTargetEnd) {
         rc = TOOSMALL;
         break;
      }
      if (r=*pUcSource++, r < 0x80) {           /* 1 byte sequence? */
         *pchTarget++ = (unsigned char)r;       /* Yes: set the value */
      }else if (r < 0x800) {                    /* 2 bytes sequence? */
         if (pchTarget+1 >= pchTargetEnd) {     /* Yes: check overflow. */
            --pUcSource;
            break;
         }
         *pchTarget++ = (unsigned char) (0xC0 | (r >> 6));
         *pchTarget++ = (unsigned char) (0x80 | (r & 0x3F));
      }else {                                   /* 3 bytes sequence. */
         if (pchTarget+2 >= pchTargetEnd) {     /* Check overflow... */
            --pUcSource;
            break;
         }
         *pchTarget++ = (unsigned char) (0xE0 | (r >> 12));
         *pchTarget++ = (unsigned char) (0x80 | ((r >> 6) & 0x3F));
         *pchTarget++ = (unsigned char) (0x80 | (r & 0x3F));
      }
   }
   *ppUcSource = pUcSource;
   *ppchTarget = pchTarget;
   return rc;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
