/* $Id: RefdKey.h,v 1.3 2011-07-29 10:26:36 pgr Exp $ */

#ifndef COM_JAXO_TOOLS_REFDKEY_H_INCLUDED
#define COM_JAXO_TOOLS_REFDKEY_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <new>
#include "toolsgendef.h"
#include "RefdItem.h"
#include "ucstring.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-----------------------------------------------------------------RefdKeyRep-+
| Keyed Items                                                                 |
+----------------------------------------------------------------------------*/
class TOOLS_API RefdKeyRep : public RefdItemRep {    // kirep
   friend class RefdKey;
public:
   RefdKeyRep();                                 // null constructor
   RefdKeyRep(UnicodeString const & ucsKeyArg);  // regular
   UnicodeString const & key() const;
protected:
   virtual ~RefdKeyRep() {}          // virtual destructor

private:
   UnicodeString const ucsKey;        // Pointer to key

   RefdKeyRep& operator=(RefdKeyRep const& repSource); // no!
   RefdKeyRep(RefdKeyRep const& repSource);            // no!
};

/* -- INLINES -- */
inline RefdKeyRep::RefdKeyRep() :
   ucsKey()
{ }
inline RefdKeyRep::RefdKeyRep(UnicodeString const & ucsKeyArg) :
   ucsKey(ucsKeyArg)
{ }
inline UnicodeString const & RefdKeyRep::key() const {
   return ucsKey;
}

/*--------------------------------------------------------------------RefdKey-+
| Referenced Key                                                              |
+----------------------------------------------------------------------------*/
class TOOLS_API RefdKey : public RefdItem {  // ki
public:
   RefdKey() {};                          // Null constructor
   RefdKey(RefdItemRep const * pSource) :      // Regular
      RefdItem(pSource)
   {}
   UnicodeString const & key() const;
   RefdKeyRep * inqKi() const;
public:
   static RefdKey const Nil;
};

/* -- INLINES -- */
inline UnicodeString const & RefdKey::key() const {
   if (pRep) {
      return ((RefdKeyRep const *)pRep)->ucsKey;
   }else {
      return UnicodeString::Nil;
   }
}
inline RefdKeyRep * RefdKey::inqKi() const {
   return (RefdKeyRep *)pRep;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
