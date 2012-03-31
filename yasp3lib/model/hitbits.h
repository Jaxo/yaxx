/*
* $Id: hitbits.h,v 1.1 2002-01-17 06:57:44 pgr Exp $
*
* Array of Bits
*/

#if !defined HITBITS_HEADER && defined __cplusplus
#define HITBITS_HEADER

/*--------------+
| Include Files |
+--------------*/
#include <string.h>
#include <assert.h>

class HitBits {
public:
   HitBits(int iBitsMax);                   // Constructor

   operator void *() const;                 // isOk?
   bool operator!() const;                  // is not ok?

   bool is_on(int iSeq) const;              // check if this bit is on
   void    set_on(int iSeq);                // set this bit on
   void    set_off(int iSeq);               // set this bit off

private:
   enum { iBitsMax = 512 };
   unsigned char auch[(iBitsMax+7)/8];      // enough for 512 hits...
};

/*-- INLINES --*/
inline HitBits::HitBits(
   int iBitsMaxArg
) {
   assert  (iBitsMaxArg < iBitsMax);
   memset(auch, '\0', sizeof auch);
}
inline bool HitBits::operator!() const {
   return false;
}
inline HitBits::operator void *() const {
   return (void *)this;
}
inline bool HitBits::is_on(int iSeq) const {
   return (auch[iSeq>>3]&(1<<(iSeq & 0x07)))? true : false;
}
inline void HitBits::set_on(int iSeq) { auch[iSeq>>3] |= 1<<(iSeq & 0x07); }
inline void HitBits::set_off(int iSeq) { auch[iSeq>>3] &= ~(1<<(iSeq & 0x07)); }

#endif /* HITBITS_HEADER =====================================================*/
