/*
* $Id: Reader.cpp,v 1.7 2011-07-29 10:26:36 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <assert.h>
#include <new>
#include "Reader.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif


/*------------------------------------------------------------MultibyteReader-+
| Null constructor                                                            |
+----------------------------------------------------------------------------*/
MultibyteReader::MultibyteReader()
{
   if (!buf) state |= ios::failbit;
   state |= ios::eofbit;
}

/*------------------------------------------------------------MultibyteReader-+
| Regular constructor                                                         |
+----------------------------------------------------------------------------*/
MultibyteReader::MultibyteReader(
   streambuf * pSbProducer,
   CatchSoftEof fCatch,
   ostream * pOstClone
) :
   buf(pSbProducer, (fCatch==doCatchSoftEof)? true : false, pOstClone)
{
   if (!buf) state |= ios::failbit;
}

/*------------------------------------------------------------MultibyteReader-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
MultibyteReader::MultibyteReader(
   streambuf * pSbProducer,
   Encoder const & encoder,
   CatchSoftEof fCatch,
   ostream * pOstClone
) :
   buf(pSbProducer, encoder, (fCatch==doCatchSoftEof)? true:false, pOstClone)
{
   if (!buf) state |= ios::failbit;
}

/*-----------------------------------------------------MultibyteReader::seekg-+
|                                                                             |
+----------------------------------------------------------------------------*/
MultibyteReader & MultibyteReader::seekg(streamoff so, ios::seekdir dir)
{
   if (EOF == buf.seekoff(so, ios::beg, ios::in)) {
      state |= ios::failbit;
   }
   return *this;
}

/*----------------------------------------------------MultibyteReader::attach-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool MultibyteReader::attach(
   streambuf * pSbProducer,
   CatchSoftEof fCatch,
   ostream * pOstClone
) {
   if (buf.attach(
         pSbProducer, (fCatch==doCatchSoftEof)? true : false, pOstClone
      )
   ) {
      state &= ~ios::eofbit;
      return true;
   }
   return false;
}

/*----------------------------------------------------MultibyteReader::attach-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool MultibyteReader::attach(
   streambuf * pSbProducer,
   Encoder const & encoder,
   CatchSoftEof fCatch,
   ostream * pOstClone
) {
   if (buf.attach(
         pSbProducer, encoder, (fCatch==doCatchSoftEof)? true:false, pOstClone
      )
   ) {
      state &= ~ios::eofbit;
      return true;
   }
   return false;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
