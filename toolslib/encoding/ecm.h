/*
* $Id: ecm.h,v 1.9 2002-04-14 23:26:04 jlatone Exp $
*/

#ifndef COM_JAXO_TOOLS_ECM_H_INCLUDED
#define COM_JAXO_TOOLS_ECM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "../toolsgendef.h"
#include "../ucstring.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

class Encoder;

/*----------------------------------------------------- class EncodingModule -+
|
|  General Contract for Encoding Modules
|  =====================================
|
|  ----------------- inqEncoding
|  Prototype:
|     virtual Id inqEncoding()
|
|  Effects:
|     Identifies uniquely the type of conversion that this encoder
|     performs.
|
|  Returns:
|     one of the values of the EncodingModule::Id enumeration below.
|
|  ----------------- inqUnicodeLength
|  Prototype:
|     virtual int unicodeLength(
|        unsigned char const * pchSource,
|        int iSizeSource
|     );
|
|  Effects:
|     Computes the maximum number of Unicode characters that would result
|     when converting the given string in 'pchSource'.
|     Any invalid multi-byte character, or partial multi-byte characters
|     at the end of the string will be counted for 1 Unicode character.
|
|  Parameters:
|     pchSource   The Multibyte string to convert.
|     iSizeSource Its length
|
|  Returns:
|     Number of Unicode characters as explain above.
|
|
|  ----------------- maxMbSize:
|  Prototype:
|     virtual int maxMbSize() const;
|
|  Effects:
|      Obtain the maximum number of bytes taken up by a
|      multi-byte character in this encoding.
|
|  Returns:
|      Maximum number of bytes as explained above.
|
|  ----------------- target2unicode:
|  Prototype:
|     virtual RC target2unicode(
|        unsigned char const ** ppchSource, unsigned int iSizeSource,
|        UCS_2 ** ppUcTarget, unsigned int iSizeTarget
|     );
|
|
|  Effects:
|     Converts string in the target encoding to Unicode.
|
|     On completion, the variable pointed to by ppchSource is updated
|     to point to the first byte past the last converted character,
|     and the variable pointed to by ppUcTarget is updated to point
|     to the first character past the last character converted.
|
|     If a sequence of input bytes does not form a valid character
|     in the specified encoding, or if the input buffer ends with
|     an incomplete sequence of bytes, conversion stops as if
|     the first byte in the sequence marked the end of the buffer.
|
|     If the output buffer cannot hold the entire converted input,
|     conversion stops as if the first byte in the sequence of input
|     bytes that would cause the output buffer to overflow marked the
|     end of the input buffer.
|
|  Parameters:
|     ppchSource      pointer to pointer to source bytes string to convert
|     iSizeSource     max number of bytes to convert in source string
|     ppUcTarget      pointer to pointer to target UCS-2 string
|     iSizeTarget     max number of bytes to convert in target UCS-2 string
|
|  Returns:
|     One of the value of the EncodingModule::RC enumeration
|
|  ----------------- unicode2target:
|  Prototype:
|     virtual RC unicode2target(
|        UCS_2 const ** ppUcSource, unsigned int iSizeSource,
|        unsigned char ** ppchTarget, unsigned int iSizeTarget
|     );
|
|  Effects:
|     Converts string in Unicode to the target encoding.
|
|     On completion, the variable pointed to by ppUcSource is updated
|     to point to the first character past the last converted character,
|     and the variable pointed to by ppchTarget is updated to point
|     to the first byte past the last character converted.
|
|     If a Unicode character has no corresponding value in the target
|     encoding, conversion stops as if this Unicode character marked the
|     end of the input buffer.
|
|     If the output buffer cannot hold the entire converted input,
|     conversion stops as if the Unicode input character that would
|     cause the output buffer to overflow marked the end of the input
|     buffer.
|
|  Parameters:
|     ppUcSource      pointer to pointer to source UCS-2 string to convert
|     iSizeSource     max number of UCS-2 to convert in source string
|     ppchTarget      pointer to pointer to target bytes string
|     iSizeTarget     max number of bytes to convert in target bytes string
|
|  Returns:
|     One of the value of the EncodingModule::RC enumeration
+----------------------------------------------------------------------------*/
class TOOLS_API EncodingModule {
   friend class Encoder;
public:
   enum Id {
      ISO_8859,
      UTF_8,
      UCS_2S,
      UCS_2R,
      CP_850,
      ROMAN_8,
      SJIS,
      EUC
   };
   enum RC {
      OK,             // Conversion successful.
      TOOSMALL,       // Output buffer too small.
      NOTWHOLE,       // Partial multi-byte characters on input
      INVALID         // Invalid character in input.
   };
   static EncodingModule * makeEncoding(EncodingModule::Id encoding);
   static Id getEncoding(UnicodeString const & ucsEncoding);

protected:
   EncodingModule();
   virtual Id inqEncoding() const = 0;
   virtual int unicodeLength(
      unsigned char const * pchSource, int iSizeSource
   ) = 0;
   virtual int maxMbSize() const = 0;
   virtual RC target2unicode(
      unsigned char const ** pchSource, unsigned int iSizeSource,
      UCS_2 ** pUcTarget, unsigned int iSizeTarget
   ) = 0;
   virtual RC unicode2target(
      UCS_2 const ** pUcSource, unsigned int iSizeSource,
      unsigned char ** pchTarget, unsigned int iSizeTarget
   ) = 0;
private:
   int iReferenceCount;
};

class EncodingModule_C : public EncodingModule {
   Id inqEncoding() const { return ISO_8859; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};


inline EncodingModule::EncodingModule() {
   iReferenceCount = 1;
}

class EncodingModule_UCS2 : public EncodingModule {
   Id inqEncoding() const { return UCS_2S; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};

class EncodingModule_UCS2R : public EncodingModule {
   Id inqEncoding() const { return UCS_2R; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};

class EncodingModule_UTF8 : public EncodingModule {
   Id inqEncoding() const { return UTF_8; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};

class EncodingModule_CP850 : public EncodingModule {
   Id inqEncoding() const { return CP_850; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};

class EncodingModule_ROM8 : public EncodingModule {
   Id inqEncoding() const { return ROMAN_8; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};

class EncodingModule_SJIS : public EncodingModule {
   Id inqEncoding() const { return SJIS; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};

class EncodingModule_EUC : public EncodingModule {
   Id inqEncoding() const { return EUC; }
   int unicodeLength(unsigned char const *, int);
   int maxMbSize() const;
   RC target2unicode(unsigned char const **, unsigned int, UCS_2 **, unsigned int);
   RC unicode2target(UCS_2 const **, unsigned int, unsigned char **, unsigned int);
};

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
