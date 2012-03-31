/*
* $Id: quantity.h,v 1.5 2002-04-14 23:26:04 jlatone Exp $
*
* SGML Quantities
*/

#if !defined QUANTITY_HEADER && defined __cplusplus
#define QUANTITY_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"


/*------------------------------+
| Indeces to the Quantity Table |
+------------------------------*/
/*
| Values from the Reference Concrete Syntax
*/
enum { ATTCNT_RCS =     40 };
enum { ATTSPLEN_RCS =  960 };
enum { BSEQLEN_RCS =   960 };
enum { DTAGLEN_RCS =    16 };
enum { DTEMPLEN_RCS =   16 };
enum { ENTLVL_RCS =     16 };
enum { GRPCNT_RCS =     32 };
enum { GRPGTCNT_RCS =   96 };
enum { GRPLVL_RCS =     16 };
enum { LITLEN_RCS =    240 };
enum { NAMELEN_RCS =     8 };
enum { NORMSEP_RCS =     2 };
enum { PILEN_RCS =     240 };
enum { TAGLEN_RCS =    960 };
enum { TAGLVL_RCS =     24 };

/*
| Maximum values allowed by the Parser
*/
enum { ATTCNT_MAX =                         180 };
enum { ATTSPLEN_MAX =                     16000 };
enum { BSEQLEN_MAX =                BSEQLEN_RCS };
enum { DTAGLEN_MAX =                DTAGLEN_RCS };
enum { DTEMPLEN_MAX =              DTEMPLEN_RCS };
enum { ENTLVL_MAX =                          40 };
#if (INT_MAX == SHRT_MAX)
enum { GRPCNT_MAX =                         100 };
enum { GRPGTCNT_MAX =                       300 };
#else
enum { GRPCNT_MAX =                         500 };
enum { GRPGTCNT_MAX =                      1000 };
#endif
enum { GRPLVL_MAX =                  GRPLVL_RCS };
enum { LITLEN_MAX =                        3000 };
enum { NAMELEN_MAX =                        128 };
enum { NORMSEP_MAX =                      16000 };
enum { PILEN_MAX =                         3000 };
enum { TAGLEN_MAX =                       16000 };
enum { TAGLVL_MAX =                          40 };

class YASP3_API Quantity  { // qnty
public:
   /*-------------------+
   | Public Definitions |
   +-------------------*/
   enum e_Ix {
      IX_ATTCNT,
      IX_ATTSPLEN,
      IX_BSEQLEN,
      IX_DTAGLEN,
      IX_DTEMPLEN,
      IX_ENTLVL,
      IX_GRPCNT,
      IX_GRPGTCNT,
      IX_GRPLVL,
      IX_LITLEN,
      IX_NAMELEN,
      IX_NORMSEP,
      IX_PILEN,
      IX_TAGLEN,
      IX_TAGLVL,
      IX_NOT_FOUND, MAX_SIZE = IX_NOT_FOUND  // must be last entries
   };

   /*-----------------+
   | Public Functions |
   +-----------------*/
   Quantity();

   int operator[](e_Ix ix) const;
   bool set(e_Ix ix, int value);

private:
   int qntytab[MAX_SIZE+1];
};

/*--- INLINES ---*/
inline int Quantity::operator[](e_Ix ix) const { return qntytab[ix]; }

#endif /* QUANTITY_HEADER ====================================================*/
