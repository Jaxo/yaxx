/*
* $Id: Encoder.cpp,v 1.4 2011-07-29 10:26:36 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <string.h>
#include <iostream>
#include <new>
#include "ucstring.h"
#include "mbstring.h"
#include "Encoder.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-----------------------------------------------------------Encoder::Encoder-+
| Copy constructor                                                            |
+----------------------------------------------------------------------------*/
Encoder::Encoder(Encoder const & source)
{
   if (source.pEcm) {
      pEcm = source.pEcm;
      ++pEcm->iReferenceCount;
   }else {
      pEcm = 0;
   }
}

/*---------------------------------------------------------Encoder::operator=-+
| Assignment operator                                                         |
+----------------------------------------------------------------------------*/
Encoder & Encoder::operator=(Encoder const & source)
{
   if (pEcm != source.pEcm) {
      this->~Encoder();
      new(this) Encoder(source);
   }
   return *this;
}

/*----------------------------------------------------Encoder::target2unicode-+
| Transform from target encoding to unicode                                   |
+----------------------------------------------------------------------------*/
void Encoder::target2unicode(
   char const *& pchSource, int iSizeSource,
   UCS_2 *& pUcTarget, int iSizeTarget
) const {
   char const * const pchSourceStart = pchSource;
   UCS_2 * const pUcTargetStart = pUcTarget;
   for (;;) {
      switch (
         pEcm->target2unicode(
            (unsigned char const **)&pchSource, iSizeSource,
            &pUcTarget, iSizeTarget
         )
      ) {
      case EncodingModule::OK:       // Conversion successful.
      case EncodingModule::TOOSMALL: // Output buffer too small.
      case EncodingModule::NOTWHOLE: // Partial multi-byte characters on input
         return;                     // stop here.
      case EncodingModule::INVALID:  // Invalid character in input.
         *pUcTarget++ = (UCS_2)'?';  // replace it...
         iSizeSource -= (++pchSource - pchSourceStart);
         iSizeTarget -= (pUcTarget - pUcTargetStart);
         /* and continue */
      }
   }
}

/*----------------------------------------------------Encoder::unicode2target-+
| Transform from unicode to target encoding                                   |
+----------------------------------------------------------------------------*/
void Encoder::unicode2target(
   UCS_2 const *& pUcSource, int iSizeSource,
   char *& pchTarget, int iSizeTarget
) const {
   UCS_2 const * const pUcSourceStart = pUcSource;
   char * const pchTargetStart = pchTarget;
   for (;;) {
      switch (
         pEcm->unicode2target(
            &pUcSource, iSizeSource,
            (unsigned char **)&pchTarget, iSizeTarget
         )
      ) {
      case EncodingModule::OK:       // Conversion successful.
      case EncodingModule::TOOSMALL: // Output buffer too small.
         return;
      case EncodingModule::INVALID:  // Invalid character in input.
         *pchTarget++ = '?';         // replace it...
         iSizeSource -= (++pUcSource - pUcSourceStart);
         iSizeTarget -= (pchTarget - pchTargetStart);
         /* and continue */
      }
   }
}

/*-----------------------------------------------------------Encoder::sputnMb-+
| Decode Unicode to MultiByte into a stream buffer                            |
+----------------------------------------------------------------------------*/
bool Encoder::sputnMb(streambuf * bp, UCS_2 const * pUc, int iLen) const
{
   if (pUc) {
      static int const iSizeMbBuf = 256;
      char achBuffer[iSizeMbBuf];
      UCS_2 const * const pUcEnd = pUc + iLen;

      for (;;) {
         char * pch = achBuffer;
         unicode2target(pUc, pUcEnd - pUc, pch, iSizeMbBuf);
         int iMbytes = pch - achBuffer;
         if (!iMbytes) return true;
         if (bp->sputn((char *)achBuffer, iMbytes) != iMbytes) return false;
      }
   }
   return true;
}

/*------------------------------------------------------------Encoder::mbSize-+
| Get the exact multibyte size of an UCstring                                 |
+----------------------------------------------------------------------------*/
int Encoder::mbSize(
   UCS_2 const * pUcStart, UCS_2 const * pUcEnd
) const {
   static int const iSizeMbBuf = 256;
   char achBuffer[iSizeMbBuf];

   int iLength = 0;
   for (;;) {
      char * pch = achBuffer;
      unicode2target(pUcStart, pUcEnd - pUcStart, pch, iSizeMbBuf);
      if (pch == achBuffer) return iLength;
      iLength += pch - achBuffer;
   }
}

int Encoder::mbSize(UCS_2 const * pUcSrc) const
{
   return mbSize(pUcSrc, pUcSrc + strlength(pUcSrc));
}

/*------------------------------------------------------------Encoder::ucSize-+
| Get the equivalent unicode size of an MbString - this is *not* a byte size. |
| The returned length is approximate.  It is at least enough big.             |
+----------------------------------------------------------------------------*/
int Encoder::ucSize(char const * pchStart, char const * pchEnd) const
{
   return pEcm->unicodeLength(
      (unsigned char const *)pchStart, pchEnd-pchStart
   );
}
int Encoder::ucSize(char const * pszSource) const
{
   return pEcm->unicodeLength(
      (unsigned char const *)pszSource, strlen(pszSource)
   );
}

/*---------------------------------------------------------------------ungetc-+
| Description:                                                                |
|   This is a hack which is required because of the poor design               |
|   of sputbackc().  sputbackc(c) requires that the character 'c'             |
|   which is putback is the same than the one that was there.                 |
|   However, some implementations (Visual) try to really write this           |
|   character back on to the stream, and, if the stream is read-only,         |
|   it fails!  GNU has sungetc(), which is better.  The true way to           |
|   do it would be to do a seekoff(0, ios::beg) but... i'm not sure           |
|   if Visual would not do a true IO (while I want this Sniffer to            |
|   be real quick.)                                                           |
+----------------------------------------------------------------------------*/
class Hacked_Streambuf : public streambuf {
public:
   void ungetc() {
      if (eback() < gptr()) { gbump(-1); } else { sputbackc(sgetc()); }
   }
};

/*-------------------------------------------------Encoder::interpretByteMark-+
| Effects:                                                                    |
|    Recognize the encoding of a stream, from:                                |
|     - a byte order mask, if such one exists                                 |
|     - optionaly, 1 or 2 characters the stream is supposed to begin          |
|       with.  These characters are not systematically checked: they          |
|       just act as an aid to help determining the encoding.                  |
|                                                                             |
| Args:                                                                       |
|   pSb:      pointer to the stream buffer to read from                       |
|   pszStamp: pointer to an ASCIIZ string of the characters                   |
|             the stream is expected to begin with.                           |
|             No more than 2 characters will be used.                         |
|             Can be NULL if no such recogniztion need to be performed.       |
| Returns:                                                                    |
|   The OS encoding of the stream, or 0 if it was impossible to determine.    |
|   Note that ISO8879 really means UTF-8, ASCII, ISO 646, or one of the       |
|   variable-width sets like Shift-JIS, EUC, etc.                             |
|   The stream buffer is updated so the next character available is           |
|   after any Byte Order Mask, since it is not really part of the             |
|   stream data.                                                              |
|                                                                             |
| Example:                                                                    |
|   streambuf * theStream;                                                    |
|   OsEncoding whatEncoding = Encoder::interpretByteMark(theStream, "<?XML"); |
|   if (whatEncoding) {                                                       |
|      [try parsing <?XML ...]                                                |
|   }                                                                         |
|                                                                             |
| Implementation Notes:                                                       |
|   No more than 2 characters are allowed:                                    |
|     sputback() doesn't allow it (unbuffered streams)                        |
|   By nature, this code is a wee bit tedious.                                |
|   It should be burried inside an Encoder constructor.                       |
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::Id Encoder::interpretByteMark(
   streambuf * pSb, char const * pszStamp
) {
   union {
      unsigned short i;
      char c[2];
   }bom;                       // Byte Order Mark

   int i = pSb->sbumpc();      // read 1st byte
   if (i == -1) {  // EOF
      return EncodingModule::ISO_8859; // empty file: who cares of the encoding?
   }
   bom.c[0] = i;
   i = pSb->sgetc();           // peek 2nd byte
   if (i == -1) {  // EOF
      ((Hacked_Streambuf *)pSb)->ungetc();  // unread 1st byte
      return EncodingModule::ISO_8859;
   }
   bom.c[1] = i;
   switch (bom.i) {
   case 0xFEFF:                // zero width no-break space
      pSb->sbumpc();           // eat 2nd byte
      return EncodingModule::UCS_2S;
   case 0xFFFE:                // reverted endians
      pSb->sbumpc();           // eat 2nd byte
      return EncodingModule::UCS_2R;
   default:
      ((Hacked_Streambuf *)pSb)->ungetc();  // unread 1st byte
      if (!pszStamp || !pszStamp[0]) {
         return EncodingModule::ISO_8859;
      }else if (bom.i == pszStamp[0]) {
         return EncodingModule::UCS_2S;
      }else if (bom.i == (pszStamp[0] << 8)) {
         return EncodingModule::UCS_2R;
 //|  }else if (conversion of pszStamp[0] matches c[0] and c[1]) {
 //|     return theEncoding_that_matches;
      }else if (!pszStamp[1]) {
         return EncodingModule::ISO_8859;
      }else if ((bom.c[0] == pszStamp[0]) && (bom.c[1] == pszStamp[1])) {
         return EncodingModule::ISO_8859;
 //|  }else if (conversion of pszStamp matches c[0] and c[1]) {
 //|     return theEncoding_that_matches;
      }
      break;
   }
   return EncodingModule::ISO_8859;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
