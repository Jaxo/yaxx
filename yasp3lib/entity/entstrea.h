/*
* $Id: entstrea.h,v 1.5 2002-04-10 05:25:12 pgr Exp $
*
* Stream classes for internal SGML entities:
*
* Regular internal entities            EntStreamMem
* Bracketed internal entities          EntStreamMemBracketed
*/

#if !defined ENTSTREA_HEADER && defined __cplusplus
#define ENTSTREA_HEADER

/*--------------+
| Include Files |
+--------------*/
#include <string.h>
#include "../parser/yspenum.h"
#include "../../toolslib/MemStream.h"

class Entity;
class UnicodeString;

/*---------------------------------------------------------BracketedStreamBuf-+
| This class shouldn't be used directly                                       |
+----------------------------------------------------------------------------*/
class BracketedStreamBuf : public MemStreamBuf {
public:
   BracketedStreamBuf(
      UCS_2 const * pUcPrefixArg,
      UCS_2 const * pUcBodyArg,
      UCS_2 const * pUcSuffixArg
   );
   int underflow();
   streampos seekoff(streamoff so, ios::seekdir dir, ios__openmode om);
   streampos seekpos(streampos sp, ios__openmode om);
   int in_avail();                                      // non virtual!
   int pcount();

private:
   UCS_2 const * const pUcPrefix;
   int const iLenPrefix;
   UCS_2 const * const pUcBody;
   int const iLenBody;
   UCS_2 const * const pUcSuffix;
   int iLenSuffix;

   BracketedStreamBuf& operator=(BracketedStreamBuf const& source); // no!
   BracketedStreamBuf(BracketedStreamBuf const& source);            // no!
};

/* -- INLINES -- */
inline int BracketedStreamBuf::pcount() {
   return iLenPrefix + iLenBody + iLenSuffix;
}

/*---------------------------------------------------------------EntStreamMem-+
| Internal Entity                                                             |
+----------------------------------------------------------------------------*/
class EntStreamMem : public istream {
public:
   EntStreamMem(UnicodeString const & ucsContent);
private:
   MemStreamBuf buf;

   EntStreamMem& operator=(               // no!
      EntStreamMem const& source
   );
   EntStreamMem(                          // no!
      EntStreamMem const& source
   );
};

/*------------------------------------------------------EntStreamMemBracketed-+
| Bracketed Internal Entity [10.5.4]                                          |
+----------------------------------------------------------------------------*/
class EntStreamMemBracketed : public istream {
public:
   EntStreamMemBracketed(
      UnicodeString const & pUcsPrefix,
      UnicodeString const & pUcsBody,
      UnicodeString const & pUcsSuffix
   );
   int pcount() { return buf.pcount(); }
private:
   BracketedStreamBuf buf;

   EntStreamMemBracketed& operator=(              // no!
      EntStreamMemBracketed const& source
   );
   EntStreamMemBracketed(
      EntStreamMemBracketed const& source         // no!
   );
};

#endif /* ENTSTREA_HEADER ===================================================*/
