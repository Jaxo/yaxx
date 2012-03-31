/* $Id: CharConverter.h,v 1.13 2002-08-25 11:46:22 pgr Exp $ */

#ifndef COM_JAXO_YAXX_CHARCONVERTER_H_INCLUDED
#define COM_JAXO_YAXX_CHARCONVERTER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class RexxString;

/*------------------------------------------------------ class CharConverter -+
|                                                                             |
+----------------------------------------------------------------------------*/
class CharConverter {
public:
   virtual ~CharConverter();
   virtual void operator()(char * pTgt, char const * pSrc, int len) const;
   // so to do: CharConverter::Upper(uppered, source, len);
   static CharConverter const Upper;
};

class DynamicCharConverter : public CharConverter {
public:
   DynamicCharConverter(char pad, RexxString const & tableo);
   DynamicCharConverter(RexxString const & tablei, char pad);
   DynamicCharConverter(RexxString const & tablei, char pad, RexxString const & tableo);
private:
   void operator()(char * pTgt, char const * pSrc, int len) const;
   char m_table[257];
};

/* -- INLINES -- */
inline CharConverter::~CharConverter() {}


#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
