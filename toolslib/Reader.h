/*
* $Id: Reader.h,v 1.6 2011-07-29 10:26:36 pgr Exp $
*
* The MultibyteReader class defines streams for Unicode consumers of
* ultimate producers with any encoding recognized by the PLS package.
*
* It has been designed to:
*  - provide all functions common to istream's
*  - ensure the correctness of the translation for wierd cases: unreads,
*    mixed encoding, etc...
*  - enable "peeks" and "skips" at some (customizable) offset.
*    This for more efficient parsing.
*
* Ultimate producers are istreams dynamically attached at any time
* to the MultibyteReader object.  Their encoding is passed as argument
* to the attach() method.  If the encoding of a producer changes, it just
* needs to be attached again while specifying the new encoding.
*
*
* MORE DETAILS....
*
* MultibyteReader's are like regular istream's, allowing to read a
* multibyte encoded input stream as if the input was UCS-2 encoded.
*
* The input stream, named "producer", is defined as a regular C++ stream
* buffer (streambuf), attached to the MultibyteReader object either
* at construction time or dynamically.
*
* The encoding of the producer is defined by an Encoder object, which is
* a C++ wrapper over the Encoding Conversion Module.  An OsEncoding can be
* used instead of a reference to an Encoder ncoding since an Encoder can be
* constructed from an OsEncoding.
*
* Warning:
*  ! Methods that re-position the producer stream: seekg(), tellg(), sync()
*  ! detach(), attach() may not work:
*  ! - for "dos" file not opened in "binary" mode
*  ! - for state dependent encodings
*  !
*  ! For example, detach() does a sync() on the actual producer.
*  ! So is attach(), that detaches the actual producer before to attach
*  ! the new one.   The position and state of the detached producer might
*  ! be wrong if it does not comply with the conditions stated above.
*  ! The safe solution is to construct a new MultibyteReader per producer.
*  ! Since this restriction only applies to the "detached" producer, it
*  ! is perfectly safe to use the NULL constructor and to later attach the
*  ! producer, or to attach new producers, but not resume any detached ones.
*
*
* Constructors:
*
*     - The NULL constructor makes a MultibyteReader with an empty producer
*       and the locale encoding.
*     - The 2nd constructor takes the address of the stream buffer of the
*       the producer.
*     - The 3rd constructor takes the address of the stream buffer of the
*       the producer, and a reference to an Encoder defining what encoding
*       the producer is in.
*
*     Both 2nd and 3rd constructors have optional arguments:
*
*     - CatchSoftEof fCatch:  when set to "doCatchSoftEof", the DOS EOF
*       character (26, 0x1A, 032, "SUB", ^Z, ...) is recognized  as a
*       signal of the physical end of the producer stream, if and only
*       if nothing follows it.
*
*     - ostream * clone: to duplicate the input stream to an ouput stream
*       in synchronism with the input stream reads.  Its use is specific
*       (parser) and this parameter should be null in most applications.
*
*
* General Methods:
*     Mostly all methods of istream's.
*
* Specific Methods:
*
* "peek(int offset)" performs a trial read from the current read location
*         to a number of Unicode characters.  It doesn't change the location.
*
*         With the restrictions indicated in Note 1, peek(i, uc) will always
*         be successful when "i" is in the range:
*         [RWBuffer::MINPEEK ... RWBuffer::MAXPEEK]
*
*         "peek" returns EOF if it was unsuccessful.
*
* "skip(int toSkip)" repositions the current location -- generally, this is
*         done after enough peek()'s determined a correct assumption in the
*         parse flow.
*
*         With the restrictions indicated in Note 1, skip(i) will always be
*         successful for any "i" greater or equal to the (negative) value
*         MINPEEK, (and less than the current position to the EOF.)
*
*         "skip" returns the number of Unicode characters skipped.
*         Less values indicate the EOF reached (the ios::eofbit is set) or
*         a multibyte code that wasn't properly translated using the current
*         encoding.
*
* "str(iLen, iOffset)" returns an array made of iLen const UCS_2, read
*         from the current location + iOffset.  If less than iLen UCS_2
*         can be provided, 0 is returned.  The number of characters
*         available forward is furnished by rdbuf()->in_avail().
*         When looking backward for previous chars, use rdbuf()->in_back().
*         Note! str() doesn't do any skip: the stream position doesn't change
*         -- this explains that rdbuf()->in_avail() returns the maximum length
*         available after str() returned 0.
*
* "attach" attaches an istream as the ultimate producer.  The same producer
*         can be attached many times, and any number of producers can be
*         attached in any order.  New producers are always attached at the
*         current read position of the previous producer.  So if the latter
*         is attached again, it will restart from its current position: this
*         eases implementation of stack of producers.
*
*         The 2nd attach method takes an Encoder argument to declare what
*         encoding the producer uses.  An OsEncoding can be used instead
*         since an Encoder can be created from an OsEncoding.
*
*         When the enum "CatchSoftEof" argument is set to "doCatchSoftEof",
*         the DOS EOF character (26, 0x1A, 032, "SUB", ^Z, ...) is
*         recognized as a signal of the physical end of the producer stream,
*         if and only if nothing follows it.
*
* "detach" removes the link with the producer, and returns the producer's
*         istream (pointer).  Before to perform any operations on the
*         attached istream, it must be first detach()'ed.
*         Hence, once an istream has been attached, no one but the
*         MultibyteReader owner must access it.   This rule is inforced
*         in safe implementations by making the detach() method the only way
*         to get back the istream pointer.
*
* "seekg()" and "tellg()" methods for positioning are provided.
*         tellg() indicates the position within the currently attached
*         producer.  The same producer must be re-attached first before
*         to seekg to the position tellg() returned.  Due to the complexity
*         of the Multibyte encoding, only the "seekg" method using a
*         "streampos" returned by "tellg" is guaranteed to work.
*
*         ---------------
* Note 1: the producer size and current location effectively reached might
*         restrict these limits: one cannot peek at -5 when at the beginning
*         of the file, or skip at MINPEEK and peek at -1.
*
* Note 2: some iostream implementations do not properly set the ios::failbit
*         when a seekg failed.
*/

#ifndef COM_JAXO_TOOLS_READER_H_INCLUDED
#define COM_JAXO_TOOLS_READER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <iostream>
#include <fstream>

#include "toolsgendef.h"
#include "migstream.h"
#include "RWBuffer.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------------MultibyteReader-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API MultibyteReader : public UnicodeExtractor {
public:
   enum CatchSoftEof { noCatchSoftEof=0, doCatchSoftEof=1 };

   MultibyteReader();
   MultibyteReader(
      streambuf * pSbProducer,
      CatchSoftEof fCatch=doCatchSoftEof,
      ostream * pOstClone = 0
   );
   MultibyteReader(
      streambuf * pSbProducer,
      Encoder const & encoder,
      CatchSoftEof fCatch=doCatchSoftEof,
      ostream * pOstClone = 0
   );
   MultibyteRWBuffer * rdbuf()  { return &buf; }  // overload ios
   bool attach(
      streambuf * pSbProducer,
      CatchSoftEof fCatch=doCatchSoftEof,
      ostream * pOstClone = 0
   );
   bool attach(
      streambuf * pSbProducer,
      Encoder const & encoder,
      CatchSoftEof fCatch=doCatchSoftEof,
      ostream * pOstClone = 0
   );
   streambuf * detach();
   bool resetEncoding(Encoder const & encoder);
   int peek(int iOffset);
   int skip(int iToSkip);
   MultibyteReader & get(UCS_2 & uc);
   MultibyteReader & putback(UCS_2 uc);
   UCS_2 const * str(int iLen, int iOffset=0);
   MultibyteReader & seekg(streampos);
   MultibyteReader & seekg(streamoff, ios::seekdir);
   streampos tellg();

   void enableClone(bool isToEnable);
   bool isCloned() const;

private:
   MultibyteRWBuffer buf;

   MultibyteReader & operator=(MultibyteReader const &); // no!
   MultibyteReader(MultibyteReader const &);             // no!
};

/* -- INLINES -- */
inline streambuf * MultibyteReader::detach() {
   return buf.detach();
}
inline bool MultibyteReader::resetEncoding(Encoder const & encoder) {
   return buf.resetEncoding(encoder);
}
inline int MultibyteReader::peek(int iOffset) {
   return buf.peek(iOffset);
}
inline int MultibyteReader::skip(int iToSkip) {
    if (0 == (state & ios::failbit)) {
      int const iSkipped = buf.skip(iToSkip);
      if ((iSkipped < iToSkip) && (buf.isEofReached())) {
         state |= ios::eofbit;
      }else {
         state &= ~ios::eofbit;
      }
      return iSkipped;
   }else {
      return 0;
   }
}
inline MultibyteReader & MultibyteReader::get(UCS_2 & uc) {
   int i = buf.sgetUc();
   if (i == EOF) state |= ios::eofbit;
   uc = (UCS_2)i;
   return *this;
}
inline MultibyteReader & MultibyteReader::seekg(streampos sp) {
   return seekg(sp, ios::beg);
}
inline streampos MultibyteReader::tellg() {
   return buf.seekoff(0, ios::cur, ios::in);
}
inline MultibyteReader & MultibyteReader::putback(UCS_2 uc) {
   if (!buf.sputbackUc(uc)) state |= ios::failbit; else state &= ~ios::eofbit;
   return *this;
}
inline UCS_2 const * MultibyteReader::str(int iLen, int iOffset) {
   return buf.str(iLen, iOffset);
}
inline void MultibyteReader::enableClone(bool isToEnable) {
   buf.enableClone(isToEnable);
}
inline bool MultibyteReader::isCloned() const {
   return buf.isCloned();
}


#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/

