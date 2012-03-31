/*
* $Id: ecm.cpp,v 1.4 2002-08-28 02:07:40 jlatone Exp $
*/

/*---------+
| Includes |
+---------*/
#include "ecm.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

static UnicodeString const ucsC_0("ISO8859_1");
static UnicodeString const ucsC_1("ASCII");
static UnicodeString const ucsC_2("Cp1252");
static UnicodeString const ucsUTF8_0("UTF8");
static UnicodeString const ucsUCS2_0("UnicodeLittle");
static UnicodeString const ucsUCS2R_0("UnicodeBig");
static UnicodeString const ucsCP850_0("Cp850");
static UnicodeString const ucsROM_8_0("Roman8");
static UnicodeString const ucsSJIS_0("JIS0201");
static UnicodeString const ucsSJIS_1("JISAutoDetect");
static UnicodeString const ucsEUC_0("EUC_JP");
static UnicodeString const ucsEUC_1("EUC_KR");

/*-----------------------------------------------------------Encoder::Encoder-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule * EncodingModule::makeEncoding(EncodingModule::Id encoding)
{
   switch (encoding) {
   case ISO_8859:
      return new EncodingModule_C;
#if !defined MWERKS_NEEDS_ENC_FIX      
   case UTF_8:
      return new EncodingModule_UTF8;
   case UCS_2S:
      return new EncodingModule_UCS2;
   case UCS_2R:
      return new EncodingModule_UCS2R;
   case CP_850:
      return new EncodingModule_CP850;
   case ROMAN_8:
      return new EncodingModule_ROM8;
   case SJIS:
      return new EncodingModule_SJIS;
   case EUC:
      return new EncodingModule_EUC;
#endif      
   default:
      return 0;
   }
}

/*------------------------------------------------EncodingModule::getEncoding-+
|                                                                             |
+----------------------------------------------------------------------------*/
EncodingModule::Id EncodingModule::getEncoding(
   UnicodeString const & ucsEncoding
) {
   if (ucsEncoding == ucsC_0)          return ISO_8859;
   else if (ucsEncoding == ucsUTF8_0)  return UTF_8;
   else if (ucsEncoding == ucsUCS2_0)  return UCS_2S;
   else if (ucsEncoding == ucsUCS2R_0) return UCS_2R;
   else if (ucsEncoding == ucsCP850_0) return CP_850;
   else if (ucsEncoding == ucsROM_8_0) return ROMAN_8;
   else if (ucsEncoding == ucsSJIS_0)  return SJIS;
   else if (ucsEncoding == ucsEUC_0)   return EUC;
   else if (ucsEncoding == ucsC_1)     return ISO_8859;
   else if (ucsEncoding == ucsSJIS_1)  return SJIS;
   else if (ucsEncoding == ucsEUC_1)   return EUC;
   else if (ucsEncoding == ucsC_2)     return ISO_8859;
   else                                return UTF_8;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
