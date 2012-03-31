/*
* $Id: features.h,v 1.5 2002-04-14 23:26:04 jlatone Exp $
*
* Parser features
*/

#if !defined FEATURES_HEADER && defined __cplusplus
#define FEATURES_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "rsvdname.h"

/*-------------------------------------------------------------------Features-+
|                                                                             |
+----------------------------------------------------------------------------*/
class YASP3_API Features {    // ft
public:
   /*-------------------+
   | Public Definitions |
   +-------------------*/
   enum e_ReplaceCode {
      OK,
      SET_TO_MAXIMUM,
      NOT_SUPPORTED,
      NOT_A_FEATURE
   };

   /*-----------------+
   | Public Functions |
   +-----------------*/
   Features();
   e_ReplaceCode set(
      ReservedName::e_Ix ixFeatureName, unsigned int iNumber
   );
   bool isFormal() const;          // formal public identifiers
   bool isConcur() const;          // concurrent document instances
   bool isExplicit() const;        // explicit link process
   bool isImplicit() const;        // implicit link process
   bool isSimple() const;          // simple link process
   bool isSubdoc() const;          // nested subdocuments
   bool isRank() const;            // omitted rank suffix minimization
   bool isDatatag() const;         // datatag minimization
   bool isOmittag() const;         // omitted tag minimization
   bool isShorttag() const;        // short tag minimization
   bool isEmptynrm() const;        // empty element can have end-tags -- Web
   bool isKeeprsre() const;        // turns off ignoring RSs and REs  -- Web
   unsigned int maxSubdoc() const;    // Maximum number of subdocs
   unsigned int maxConcur() const;    // Maximum number of concurrent documents
   unsigned int maxSimple() const;    // Maximum number of simple links
   unsigned int maxExplicit() const;  // Maximum number of explicit links
   unsigned int code() const;         // see ISO8879 appendix G

private:
   enum { MAX_SUBDOC   = 10 };
   enum { MAX_CONCUR   = 10 };
   enum { MAX_SIMPLE   = 10 };
   enum { MAX_EXPLICIT = 10 };
   unsigned int bFormal   :1;     // formal public identifiers
   unsigned int bConcur   :1;     // concurrent document instances
   unsigned int bExplicit :1;     // explicit link process
   unsigned int bImplicit :1;     // implicit link process
   unsigned int bSimple   :1;     // simple link process
   unsigned int bSubdoc   :1;     // nested subdocuments
   unsigned int bRank     :1;     // omitted rank suffix minimization
   unsigned int bDatatag  :1;     // datatag minimization
   unsigned int bOmittag  :1;     // omitted tag minimization
   unsigned int bShorttag :1;     // short tag minimization
   unsigned int bEmptynrm :1;     // empty element can have end-tags
   unsigned int bKeeprsre :1;     // turns off ignoring RSs and REs
   unsigned int iMaxSubdoc;       // Maximum number of subdocs
   unsigned int iMaxConcur;       // Maximum number of concurrent documents
   unsigned int iMaxSimple;       // Maximum number of simple links
   unsigned int iMaxExplicit;     // Maximum number of explicit links
};

/*-- INLINES --*/
inline bool Features::isFormal() const          { return bFormal;      }
inline bool Features::isConcur() const          { return bConcur;      }
inline bool Features::isExplicit() const        { return bExplicit;    }
inline bool Features::isImplicit() const        { return bImplicit;    }
inline bool Features::isSimple() const          { return bSimple;      }
inline bool Features::isSubdoc() const          { return bSubdoc;      }
inline bool Features::isRank() const            { return bRank;        }
inline bool Features::isDatatag() const         { return bDatatag;     }
inline bool Features::isOmittag() const         { return bOmittag;     }
inline bool Features::isShorttag() const        { return bShorttag;    }
inline bool Features::isEmptynrm() const        { return bEmptynrm;    }
inline bool Features::isKeeprsre() const        { return bKeeprsre;    }
inline unsigned int Features::maxSubdoc() const    { return iMaxSubdoc;   }
inline unsigned int Features::maxConcur() const    { return iMaxConcur;   }
inline unsigned int Features::maxSimple() const    { return iMaxSimple;   }
inline unsigned int Features::maxExplicit() const  { return iMaxExplicit; }

#endif /* FEATURES_HEADER ====================================================*/
