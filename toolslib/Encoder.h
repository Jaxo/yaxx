/*
* $Id: Encoder.h,v 1.5 2002-06-18 10:36:31 pgr Exp $
*
* Encoder from/to Unicode to/from Multibyte
*/

#ifndef COM_JAXO_TOOLS_ENCODER_H_INCLUDED
#define COM_JAXO_TOOLS_ENCODER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include "toolsgendef.h"
#include "encoding/ecm.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

class UnicodeString;
typedef char const * Encoding;

/*--------------------------------------------------------------------Encoder-+
| Encoder is just a C++ wrapper over ECM's                                    |
+----------------------------------------------------------------------------*/
class TOOLS_API Encoder {
public:
   Encoder(EncodingModule::Id encoding = EncodingModule::ISO_8859);
   Encoder(Encoder const & source);  // copy
   ~Encoder();
   Encoder & operator=(Encoder const & source);

   static EncodingModule::Id getEncoding(UnicodeString const & ucsEncoding);
   bool setEncoding(Encoder const & encoder);
   EncodingModule::Id inqEncoding() const;

   int maxMbSize() const;

   int mbSize(UCS_2 const * pUcStart, UCS_2 const * pUcEnd) const;
   int mbSize(UCS_2 const * pUcStart) const;

   int ucSize(char const * pszStart, char const * pszEnd) const;
   int ucSize(char const * pszSource) const;

   bool sputnMb(streambuf * bp, UCS_2 const * pUc, int iLen) const;

   void target2unicode(
      char const *& pszSource, int iSizeSource,
      UCS_2 *& pUcTarget, int iSizeTarget
   )const;
   void unicode2target(
      UCS_2 const *& pUcSource, int iSizeSource,
      char *& pszTarget, int iSizeTarget
   )const;


   static EncodingModule::Id interpretByteMark(
      streambuf * pSb, char const * pszStamp
   );

   bool operator==(Encoder const & source) const;  // equality
   bool operator!=(Encoder const & source) const;  // inequality

   bool isValid() const;

   // these calls are now obsolete (based on zero sentinels)
   int native2unicode(char const *pszSource, UCS_2 * pUcTarget) const;
   int native2unicode(
       char const *pszSource, UCS_2 * pUcTarget, int iSize
   ) const;
   int unicode2native(UCS_2 const * pUcSource, char * pchTarget) const;

private:
   EncodingModule * pEcm;

public:
};

/*-- INLINES --*/
inline Encoder::Encoder(EncodingModule::Id encoding) {
   pEcm = EncodingModule::makeEncoding(encoding);
}

inline Encoder::~Encoder() {
   if (--pEcm->iReferenceCount == 0) delete pEcm;
}

inline EncodingModule::Id Encoder::getEncoding(UnicodeString const & ucsEncoding) {
   return EncodingModule::getEncoding(ucsEncoding);
}

inline bool Encoder::isValid() const {
   return (pEcm != 0);
}

inline bool Encoder::setEncoding(Encoder const & encoder) {
   *this = encoder;
   return true;
}

inline EncodingModule::Id Encoder::inqEncoding() const {
   return pEcm->inqEncoding();
}

inline int Encoder::maxMbSize() const {
   return pEcm->maxMbSize();
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
