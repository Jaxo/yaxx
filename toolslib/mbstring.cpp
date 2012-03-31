/*
* $Id: mbstring.cpp,v 1.3 2011-07-29 10:26:37 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include <string.h>
#include "mbstring.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

MultibyteString const MultibyteString::Nil;
CompositeString const CompositeString::Nil;

/*STATIC------------------------------------------MultibyteString::Rep::alloc-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline MultibyteString::Rep * MultibyteString::Rep::alloc(
   int chSize, Encoder const & ecm
) {
   Rep * pRep = (Rep *)new char[sizeof(Rep) + chSize + 1];
   if (pRep) {
      pRep->iReferenceCount = 1;
      new(&pRep->ecm) Encoder(ecm);
      pRep->iLength = chSize;
   }
   return pRep;
}

/*-------------------------------------------MultibyteString::MultibyteString-+
| Constructor from an array of Unicode, lengthed                              |
+----------------------------------------------------------------------------*/
MultibyteString::MultibyteString(
   UCS_2 const * pUcSource,
   Encoder const & ecm,
   int iLenSource
) {
   if (iLenSource) {
      int const iLenTarget = ecm.mbSize(pUcSource, pUcSource + iLenSource);
      pRep = Rep::alloc(iLenTarget, ecm);
      if (pRep) {
         char * pchTarget = pRep->data();
         ecm.unicode2target(pUcSource, iLenSource, pchTarget, iLenTarget);
         pRep->iLength = pchTarget - pRep->data();  // set exact size
         *pchTarget = '\0';
      }
   }else {
      pRep = 0;
   }
}

/*-------------------------------------------MultibyteString::MultibyteString-+
| Constructor from a unicode string                                           |
+----------------------------------------------------------------------------*/
MultibyteString::MultibyteString(
   UnicodeString const & ucsSource,
   Encoder const & ecm
) {
   new(this) MultibyteString(ucsSource(ecm));
}

/*-------------------------------------------MultibyteString::MultibyteString-+
| Constructor from a multibyte C string (temporary?)                          |
+----------------------------------------------------------------------------*/
MultibyteString::MultibyteString(char const * pszSource, Encoder const & ecm)
{
   if (pszSource) {
      pRep = Rep::alloc(strlen(pszSource), ecm);
      if (pRep) {
         memcpy(pRep->data(), pszSource, pRep->iLength + 1);
      }
   }else {
      pRep = 0;
   }
}

/*-------------------------------------------MultibyteString::MultibyteString-+
| Copy constructor (shallow)                                                  |
+----------------------------------------------------------------------------*/
MultibyteString::MultibyteString(MultibyteString const & mbsSource)
{
   pRep = mbsSource.pRep;
   if (pRep) ++pRep->iReferenceCount;
}

/*------------------------------------------MultibyteString::~MultibyteString-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
MultibyteString::~MultibyteString()
{
   if ((pRep) && (!--pRep->iReferenceCount)) {
      delete [] (char *) pRep;
   }
   pRep = 0;
}

/*-------------------------------------------------MultibyteString::operator=-+
| Assignment                                                                  |
+----------------------------------------------------------------------------*/
MultibyteString& MultibyteString::operator=(MultibyteString const & mbsSource)
{
   if (pRep != mbsSource.pRep) {
      this->~MultibyteString();
      new(this) MultibyteString(mbsSource);
   }
   return *this;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
