/*
* $Id: RWBuffer.cpp,v 1.8 2011-07-29 10:26:36 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <assert.h>
#include <new>
#include "RWBuffer.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

#define EOFSIZE 1      // size of the SOFTEOF, to make code more readable
#define RESTRICTED_EBACK


/*============================= UnicodeRWBuffer =============================*/


/*---------------------------------------------------UnicodeRWBuffer::x_reset-+
| Reset all x_pointers                                                        |
+----------------------------------------------------------------------------*/
inline void UnicodeRWBuffer::x_reset()
{
   // x_pbase = x_pptr =  (PUT: Not Yet Implemented)
   // x_epptr = x_ebuf;   (PUT: Not Yet Implemented)
   x_eback = x_gptr = x_egptr = x_base;
}

/*-------------------------------------------UnicodeRWBuffer::UnicodeRWBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeRWBuffer::UnicodeRWBuffer(streambuf * pSbProdArg)
{
   pSbProd = pSbProdArg;
   bCatchSoftEof = 0;
   bSoftEofFound = 0;
   bCloneEnabled = 0;
   x_pbase = x_pptr = x_epptr = 0;  //  (PUT: Not Yet Implemented)
   x_base = (UCS_2 *)new char[UCBUFSIZE*sizeof(UCS_2)];
   if (x_base) {
      x_ebuf = x_base + UCBUFSIZE;
      if (pSbProd) bEof = 0; else bEof = 1;
      bOk = 1;
   }else {
      x_ebuf = 0;
      bEof = 1;
      bOk = 0;
   }
   x_reset();
}

/*------------------------------------------UnicodeRWBuffer::~UnicodeRWBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeRWBuffer::~UnicodeRWBuffer()
{
   delete [] (char *)x_base;
   x_base = x_ebuf = 0;
   x_reset();
   bEof = 1;
   bOk = 0;
}

/*------------------------------------------------------UnicodeRWBuffer::peek-+
|                                                                             |
+----------------------------------------------------------------------------*/
int UnicodeRWBuffer::peek(int iOffset)
{
   if (iOffset >= 0) {
      while (iOffset >= (x_egptr - x_gptr)) {
         if (underflow() == EOF_SIGNAL) {
            return EOF_SIGNAL;
         }
      }
   }else {
      if (iOffset < (x_eback - x_gptr)) {
         return EOF_SIGNAL;
      }
   }
   return UCZAPEOF(x_gptr[iOffset]);
}

/*------------------------------------------------------UnicodeRWBuffer::skip-+
| Skip at most next/previous `iToSkip' UCS_2.                                 |
| Returns number of UCS_2 actually skipped                                    |
+----------------------------------------------------------------------------*/
int UnicodeRWBuffer::skip(int iToSkip)
{
   if (iToSkip >= 0) {
      int iRemainToSkip = iToSkip;
      int iSkipping;
      do {
         iSkipping = x_egptr - x_gptr;         // iSkipping = min(
         if (iSkipping > iRemainToSkip) {      //    in_avail,
            x_gptr += iRemainToSkip;           //    iRemainToSkip
            iRemainToSkip = 0;                 // )
            break;
         }
         x_gptr += iSkipping;
      }while (
         (iRemainToSkip -= iSkipping) && (underflow() != EOF_SIGNAL)
      );
      return (iToSkip - iRemainToSkip);
   }else {
      if (iToSkip < (x_eback - x_gptr)) {
         iToSkip = (x_eback - x_gptr);
      }
      x_gptr += iToSkip;
   }
   return iToSkip;
}

/*----------------------------------------------UnicodeRWBuffer::skip_n_count-+
| Skip at most next/previous `iToSkip' UCS_2.                                 |
| Update count of `ucCounted' characters found while skipping, and the count  |
| since the last `ucCounted' was found.                                       |
| Returns number of UCS_2 actually skipped                                    |
| Typical use: updating a line/column position                                |
|                                                                             |
| Warning: when iToSkip is negative (skip backward), iSinceLast may be wrong. |
|          (i.e.: the column number is undefined when reaching previous line.)|
+----------------------------------------------------------------------------*/
int UnicodeRWBuffer::skip_n_count(
   int iToSkip, UCS_2 ucCounted,
   int & iCount, int & iSinceLast   // updated
) {
   if (iToSkip >= 0) {
      int iRemainToSkip = iToSkip;
      do {
         int iSkipping = x_egptr - x_gptr;     // iSkipping = min(
         if (iSkipping > iRemainToSkip) {      //    in_avail,
            iSkipping = iRemainToSkip;         //    iRemainToSkip
            iRemainToSkip = 0;                 // )
         }else {
            iRemainToSkip -= iSkipping;
         }
         while (iSkipping--) {
            if (*x_gptr++ == ucCounted) {
               iSinceLast = 0;
               ++iCount;
            }else {
               ++iSinceLast;
            }
         }
      }while ((iRemainToSkip) && (underflow() != EOF_SIGNAL));
      return (iToSkip - iRemainToSkip);
   }
   if (iToSkip < (x_eback - x_gptr)) {
      iToSkip = (x_eback - x_gptr);
   }
   int iSkipping = iToSkip;
   while (iSkipping++) {
       if (*--x_gptr == ucCounted) {
          iSinceLast = -1;
          --iCount;
       }else {
          --iSinceLast;
       }
   };
   return iToSkip;
}

/*------------------------------------------------------UnicodeRWBuffer::tidy-+
| Rearrange buffer to make more room.                                         |
+----------------------------------------------------------------------------*/
inline int UnicodeRWBuffer::tidy()
{
   UCS_2 * ebackNew = x_gptr - MINUNGET;
   if (ebackNew < x_eback) {
      ebackNew = x_eback;
   }
   int const iLenDiscarded = ebackNew - x_base;
   if (iLenDiscarded > 0) {
      memmove(x_base, ebackNew, ((char *)x_egptr - (char *)ebackNew));
      x_egptr -= iLenDiscarded;
      x_gptr -= iLenDiscarded,
      x_eback = x_base;
      // x_pptr = x_egptr;  (PUT: Not Yet Implemented)
      return iLenDiscarded;
   }else {
      return 0;                // too much peeks!
   }
}

/*--------------------------------------------------UnicodeRWBuffer::overflow-+
| When pptr >= epptr...                                                       |
+----------------------------------------------------------------------------*/
// (PUT: Not Yet Implemented)
//int UnicodeRWBuffer::overflow(int uc)
//{
//   if (!tidy() || (uc == EOF)) {
//      return EOF_SIGNAL;
//   }else {
//      *x_pptr++ = (UCS_2)uc;
//      x_egptr = x_pptr;
//      return myZapeof(uc);
//   }
//}

/*VIRTUAL------------------------------------------UnicodeRWBuffer::underflow-+
| When gptr >= egptr...                                                       |
| Note: 0 is returned when OK (vs: the character)                             |
+----------------------------------------------------------------------------*/
int UnicodeRWBuffer::underflow()
{
   if (bEof) return EOF_SIGNAL;

   int iUcSizeTarget = x_ebuf - x_egptr;
   if (!iUcSizeTarget) {
      iUcSizeTarget = tidy();                              // try to make room
      if (!iUcSizeTarget) {
         return EOF_SIGNAL;
      }
   }
   int iUcCount = pSbProd->sgetn(
      (char *)x_egptr, iUcSizeTarget * sizeof(UCS_2)
   ) / sizeof(UCS_2);
   if (!iUcCount) {
      bEof = 1;
      return EOF_SIGNAL;
   }
   x_egptr += iUcCount;
// if (iUcCount < iUcSizeTarget) {
//    bEof = 1;
//    if (!iUcCount) {
//       return EOF_SIGNAL;
//    }
// }
   return 0;
}

/*VIRTUAL-----------------------------------------------UnicodeRWBuffer::sync-+
| Clear all bytes in the get area, and send'em back to the ultimate           |
| producer, so in_avail is now zero                                           |
+----------------------------------------------------------------------------*/
int UnicodeRWBuffer::sync()
{
   if (!pSbProd) return EOF_SIGNAL;
   if (x_base) {              // if no base, then sync is OK
      int iBytesCount = (x_egptr -x_gptr) * sizeof(UCS_2);
      if (iBytesCount) {
         if (
            (
               #if (defined(__IBMCPP__) || defined (__IBMC__)) // CSET/2 BUG
                  EOF == pSbProd->pubseekoff(
                     pSbProd->pubseekoff(
                       0,
                       ios::cur,
                       ios::in
                     ) - iBytesCount,
                     ios::beg,
                     ios::in
                  )
               #else
                  -1 == pSbProd->pubseekoff( // EOF
                     -iBytesCount,
                     ios::cur,
                     ios::in
                  )
               #endif
            )
         ) {
           return EOF_SIGNAL;
         }
         // x_pbase = x_pptr = ??? (PUT: Not Yet Implemented)
         x_egptr = x_gptr;
         bEof = 0;
      }
   }
   return 0;
}

/*VIRTUAL--------------------------------------------UnicodeRWBuffer::seekoff-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos UnicodeRWBuffer::seekoff(
   streamoff so,
   ios::seekdir dir,
   ios__openmode om
) {
   if (om & ios::in) {
      if (sync() != EOF_SIGNAL) {
         if (so || (dir != ios::cur)) x_reset();
         return pSbProd->pubseekoff(so, dir, om);
      }
   }
   return EOF_SIGNAL;
}

/*---------------------------------------------------UnicodeRWBuffer::seekpos-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos UnicodeRWBuffer::seekpos(
   streampos sp,
   ios__openmode om
) {
   return seekoff((streamoff)sp, ios::beg, om);
}

/*VIRTUAL--------------------------------------UnicodeRWBuffer::resetEncoding-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool UnicodeRWBuffer::resetEncoding(Encoder const &)
{
   return false;
}

/*VIRTUAL-----------------------------------------UnicodeRWBuffer::inqEncoder-+
|                                                                             |
+----------------------------------------------------------------------------*/
Encoder const * UnicodeRWBuffer::inqEncoder() const
{
   return 0;
}

/*VIRTUAL----------------------------------------UnicodeRWBuffer::enableClone-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool UnicodeRWBuffer::enableClone(bool)
{
   return false;
}


/*============================ MultibyteRWBuffer ============================*/


/*--------------------------------------------MultibyteRWBuffer::setMinRemain-+
| Each time an underflow occurs, there must be enough remaining bytes         |
| in the multiByte buffer so that, when these bytes are converted,            |
| they produce at least UCPERREAD UCS_2.                                      |
| This value: iRemainLenMin is computed as being:  UCPERREAD * maxMbSize()    |
+----------------------------------------------------------------------------*/
inline void MultibyteRWBuffer::setMinRemain()
{
   iRemainLenMin = UCPERREAD * maxMbSize();
   if (iRemainLenMin < (EOFSIZE+1)) iRemainLenMin = 1+EOFSIZE;
   assert  (MBBUFLEN >= iRemainLenMin);
}

/*----------------------------------------------------MultibyteRWBuffer::init-+
|                                                                             |
+----------------------------------------------------------------------------*/
void MultibyteRWBuffer::init()
{
   x_pbase = x_pptr = x_epptr = 0;  //  (PUT: Not Yet Implemented)
   // the extra 2 bytes at the end are for historical reasons... (now useless)
   x_base = (UCS_2 *)new char[(UCBUFSIZE*sizeof(UCS_2))+MBBUFLEN+2];
   if (x_base) {
      x_ebuf = x_base + UCBUFSIZE;
      bOk = 1;
      if (isValid()) {              // check the encoder
         setMinRemain();
         bOk = 1;
      }else {
         bOk = 0;                   // bad encoder
      }
   }else {
      bOk = 0;
      x_base = 0;
      x_ebuf = 0;
   }
   x_reset();   // x_eback = x_gptr = x_egptr = x_base;
   pchMbytes = (char *)x_ebuf;
   pSbProd = 0;
   pOstClone = 0;
   bCatchSoftEof = 0;
   bCloneEnabled = 0;
   bEof = 1;
   iRemainLen = 0;
}


/*---------------------------------------------MultibyteRWBuffer::setProducer-+
|                                                                             |
+----------------------------------------------------------------------------*/
void MultibyteRWBuffer::setProducer(
   streambuf * pSbProdArg,
   bool isCatchSoftEofArg,
   ostream * pOstCloneArg
) {
   pSbProd = pSbProdArg;
   pOstClone = pOstCloneArg;
   bCatchSoftEof = isCatchSoftEofArg;
   bSoftEofFound = 0;
   if (pOstClone) bCloneEnabled = 1; else bCloneEnabled = 0;
   bEof = 0;
}

/*---------------------------------------MultibyteRWBuffer::MultibyteRWBuffer-+
| NULL constructor                                                            |
+----------------------------------------------------------------------------*/
MultibyteRWBuffer::MultibyteRWBuffer()
{
   init();
}

/*---------------------------------------MultibyteRWBuffer::MultibyteRWBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
MultibyteRWBuffer::MultibyteRWBuffer(
   streambuf * pSbProdArg,
   bool isCatchSoftEofArg,
   ostream * pOstCloneArg
) {
   init();
   if (bOk && pSbProdArg) {
      setProducer(pSbProdArg, isCatchSoftEofArg, pOstCloneArg);
   }
}

/*---------------------------------------MultibyteRWBuffer::MultibyteRWBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
MultibyteRWBuffer::MultibyteRWBuffer(
   streambuf * pSbProdArg,
   Encoder const & encoder,
   bool isCatchSoftEofArg,
   ostream * pOstCloneArg
) :
   Encoder(encoder)
{
   init();
   if (bOk && pSbProdArg) {
      setProducer(pSbProdArg, isCatchSoftEofArg, pOstCloneArg);
   }
}

/*--------------------------------------------------MultibyteRWBuffer::attach-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool MultibyteRWBuffer::attach(
   streambuf * pSbProdArg,
   bool isCatchSoftEofArg,
   ostream * pOstCloneArg
) {
   if (!bOk || !pSbProdArg) return false;
   sync();      // synchronize the get pointer with the ultimate producer
   setProducer(pSbProdArg, isCatchSoftEofArg, pOstCloneArg);
   #ifdef RESTRICTED_EBACK
      x_eback = x_gptr;
   #endif
   return true;
}

/*--------------------------------------------------MultibyteRWBuffer::attach-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool MultibyteRWBuffer::attach(
   streambuf * pSbProdArg,
   Encoder const & encoder,
   bool isCatchSoftEofArg,
   ostream * pOstCloneArg
) {
   if (!x_base || !encoder.isValid() || !pSbProdArg) return false;
   sync();      // synchronize the get pointer with the ultimate producer
   setProducer(pSbProdArg, isCatchSoftEofArg, pOstCloneArg);
   setEncoding(encoder);
   setMinRemain();
   bOk = 1;
   return true;
}

/*--------------------------------------------------MultibyteRWBuffer::detach-+
|                                                                             |
+----------------------------------------------------------------------------*/
streambuf * MultibyteRWBuffer::detach()
{
   sync();
   streambuf * pSb = pSbProd;
   pSbProd = 0;
   bEof = 1;                  // "no streambuf" is like EOF reached
   return pSb;
}

/*--------------------------------------------------MultibyteRWBuffer::reload-+
| Get more from the producer                                                  |
+----------------------------------------------------------------------------*/
inline bool MultibyteRWBuffer::reload()
{
   if (!bEof) {                         // if not at Eof, ask for more
      memmove(x_ebuf, pchMbytes, iRemainLen); // shift down
      pchMbytes = (char *)x_ebuf;
      int iCount = pSbProd->sgetn(pchMbytes+iRemainLen, MBBUFLEN-iRemainLen);
      if (bCloneEnabled) {
         pOstClone->write(pchMbytes+iRemainLen, iCount);
      }
      iRemainLen += iCount;
      if (iRemainLen < MBBUFLEN) {
         // bEof = 1;   <<<<<<<<<<<
         if (bCatchSoftEof && iRemainLen &&
            (EOFSOFT == pchMbytes[iRemainLen-1])
         ) {
            --iRemainLen;
            bSoftEofFound = 1;
         }
      }
   }
   if (iRemainLen > 0) {
      return true;
   }else {                              // When nothing left
      bEof = 1;                         // <<<<<<<<<<<<
      return false;                     // return
   }
}

/*VIRTUAL----------------------------------------MultibyteRWBuffer::underflow-+
| When gptr >= egptr...                                                       |
| Note: 0 is returned when OK (vs: the character)                             |
+----------------------------------------------------------------------------*/
int MultibyteRWBuffer::underflow()
{
   if ((iRemainLen < iRemainLenMin) && (!reload())) {
      return EOF_SIGNAL;
   }
   {
      int iUcSizeTarget = x_ebuf - x_egptr;
      if (iUcSizeTarget >= UCPERREAD) {    // can't read no more than UCPERREAD
         iUcSizeTarget = UCPERREAD;
      }else {                              // but if less,
         iUcSizeTarget += tidy();          // try to make room
         if (iUcSizeTarget >= UCPERREAD) {
            iUcSizeTarget = UCPERREAD;
         }
      }
      char const * pch = pchMbytes;
      target2unicode(
         pch,                              // updated on return
         iRemainLen,
         x_egptr,                          // updated on return
         iUcSizeTarget
      );
      if (pch == pchMbytes) {              // no translation made?
         return EOF_SIGNAL;                // just return;
      }
      iRemainLen -= (pch - pchMbytes);
      pchMbytes = (char *)pch;
   }
   assert  (iRemainLen >= 0);   // or we translated past the EOF -- bad, bad
   // x_pptr = x_egptr;  (PUT: Not Yet Implemented)
   return 0;
}

/*VIRTUAL---------------------------------------------MultibyteRWBuffer::sync-+
| Clear all bytes in the get area, and send'em back to the ultimate           |
| producer, so in_avail is now zero                                           |
+----------------------------------------------------------------------------*/
int MultibyteRWBuffer::sync()
{
   if (!pSbProd) return EOF_SIGNAL;
   if (x_base) {              // if no base, then sync is OK
      int iBytesCount = mbSize(x_gptr, x_egptr) + bSoftEofFound + iRemainLen;
      if (iBytesCount) {
         if (
            (
               #if (defined(__IBMCPP__) || defined (__IBMC__)) // CSET/2 BUG
                  EOF == pSbProd->pubseekoff(
                     pSbProd->pubseekoff(
                        0,
                        ios::cur,
                        ios::in
                     ) - iBytesCount,
                     ios::beg,
                     ios::in
                  )
               #else
                  -1 == pSbProd->pubseekoff( // EOF
                     -iBytesCount,
                     ios::cur,
                     ios::in
                  )
               #endif
            ) || (
               pOstClone && bCloneEnabled &&
               pOstClone->seekp(-iBytesCount, ios::cur).fail()
            )
         ) {
           return EOF_SIGNAL;
         }
         // x_pbase = x_pptr = ??? (PUT: Not Yet Implemented)
         x_egptr = x_gptr;
         bEof = 0;
         bSoftEofFound = 0;
         iRemainLen = 0;
      }
      pchMbytes = (char *)x_ebuf;
   }
   return 0;
}

/*VIRTUAL------------------------------------------MultibyteRWBuffer::seekoff-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos MultibyteRWBuffer::seekoff(
   streamoff so,
   ios::seekdir dir,
   ios__openmode om
) {
   streampos sp = UnicodeRWBuffer::seekoff(so, dir, om);
   if (((streamoff)sp != EOF_SIGNAL) && (pOstClone)) {
      pOstClone->rdbuf()->pubseekoff(so, dir, om);
   }
   return sp;
}

/*VIRTUAL------------------------------------MultibyteRWBuffer::resetEncoding-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool MultibyteRWBuffer::resetEncoding(Encoder const & encoder)
{
   if (!x_base || !encoder.isValid()) return false;
   sync();
   setEncoding(encoder);
   setMinRemain();
   bOk = 1;
   return true;
}

/*VIRTUAL---------------------------------------MultibyteRWBuffer::inqEncoder-+
|                                                                             |
+----------------------------------------------------------------------------*/
Encoder const * MultibyteRWBuffer::inqEncoder() const
{
   return this;
}

/*VIRTUAL--------------------------------------MultibyteRWBuffer::enableClone-+
| Enable or disable the clone output file                                     |
+----------------------------------------------------------------------------*/
bool MultibyteRWBuffer::enableClone(bool isToEnable)
{
   if (!pOstClone) {
      if (isToEnable) return false; else return true; // OK to disable oblivion
   }else {
      bCloneEnabled = isToEnable;
   }
   return true;
}


#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
