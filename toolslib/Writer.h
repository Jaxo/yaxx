/*
* $Id: Writer.h,v 1.9 2011-07-29 10:26:37 pgr Exp $
*
* Define following classes:
*
* UnicodeWriter         (base class)
* UnicodeMemWriter      write a unicode memory-file
* UnicodeFileWriter     write a unicode i/o file
* MultibyteWriter       (base class)
* MultibyteMemWriter    write a multibyte memory-file from Unicode
* MultibyteFileWriter   write a multibyte i/o file from Unicode
*
* DAG:                       ios
*                             |
*                          Writer
*                         /      \
*                        /        \
*                       /          \
*                      /            \
*           UnicodeWriter          MultibyteWriter
*            /        \             /           \
*           /          \           /             \
*  UnicodeMemWriter     \  MultibyteMemWriter     \
*                        \                         \
*                 UnicodeFileWriter        MultibyteFileWriter
*
*/

#ifndef COM_JAXO_TOOLS_WRITER_H_INCLUDED
#define COM_JAXO_TOOLS_WRITER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <iostream>

#ifdef __MWERKS__
#include "PalmFileStream.h"
#define XxxFileStreamBuf PalmFileStreamBuf
#else
#include "StdFileStream.h"
#define XxxFileStreamBuf StdFileStreamBuf
#endif

#include "toolsgendef.h"
#include "migstream.h"
#include "MemStream.h"
#include "Encoder.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------------------------Writer-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API Writer :
   protected TpManipInt::Set, public ios {
   friend class UnicodeWriter;
   friend class MultibyteWriter;
public:
   Writer(streambuf * bpArg) : ios(bpArg) {
      width(-1);
   }
   virtual Writer & operator<<(UCS_2 uc) = 0;
   virtual Writer & operator<<(char c) = 0;
   virtual Writer & operator<<(char const * psz) = 0;
   virtual Writer & operator<<(UnicodeString const & src) = 0;
   virtual Writer & operator<<(UnicodeSubstring const & src) = 0;
   void setWidth(int w);
   Writer & operator<<(int i) {
      return operator<<(UnicodeString(i));
   }
   Writer & operator<<(unsigned int i) {
      return operator<<(UnicodeString(i));
   }
   Writer & operator<<(UCS_2 const *pUc) {
      return operator<<(UcStringFlash(pUc));
   }
   Writer & operator<<(void (*f)(Writer &)) {
      f(*this);
      return *this;
   }
   Writer & operator<<(TpManipInt const & m) {
      (this->*m.fct)(m.val);
      return *this;
   }
   Writer & seekp(streampos);
   Writer & seekp(streamoff, ios::seekdir);
   streampos tellp();

private:
   Writer & operator=(Writer const &); // no!
   Writer(Writer const &);             // no!
};

inline Writer & Writer::seekp(streampos sp) {
   seekp(sp, ios::beg);
   return *this;
}

inline streampos Writer::tellp() {
   return rdbuf()->pubseekoff(0, ios::cur, ios::out);
}
inline void endl(Writer & outArg) {
   outArg << '\n';
}
inline void ends(Writer & outArg) {
   outArg << (char)0;
}
inline void flush(Writer & outArg) {
   outArg.rdbuf()->pubsync();
}

/*--------------------------------------------------------------UnicodeWriter-+
| The UnicodeWriter class IS_LIKE an ostream, but is specifically             |
| dedicated to produce Unicode.                                               |
|                                                                             |
| For conveniency, 2 classes have been derived:                               |
|     UnicodeMemWriter,  that creates a memory stream                         |
|     UnicodeFileWriter, that creates a file stream                           |
|                                                                             |
| Typical use:                                                                |
|                                                                             |
| UnicodeWriter uost(cout); // output is Unicode                              |
|                                                                             |
| uost                                                                        |
|   << unicodeString                   // native UnicodeString                |
|   << aUCSChar16_string_zero_ended    // array of UCS_2 *                    |
|   << setw(10) << aUCSChar16_string   // not zero ended - also no conversion |
|   << 123                             // UnicodeString(123)                  |
|   << "Hello, world!"                 // latin char string                   |
|   << 'a'                             // latin char                          |
|   << aMultibyteStringInShiftJis      // UnicodeString(MultibyteString &)    |
|   << endl;                           // a unicode '\n'                      |
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeWriter : public Writer {
public:
   UnicodeWriter(streambuf * bpArg) : Writer(bpArg) {}

   // non virtuals ("hide", rather than "overload")
   Writer & operator<<(int i) {
      return operator<<(UnicodeString(i));
   }
   Writer & operator<<(unsigned int i) {
      return operator<<(UnicodeString(i));
   }
   Writer & operator<<(UCS_2 const *pUc) {
      return operator<<(UcStringFlash(pUc));
   }
   Writer & operator<<(void (*f)(Writer &)) {
      f(*this);
      return *this;
   }
   Writer & operator<<(TpManipInt const & m) {
      (this->*m.fct)(m.val);
      return *this;
   }

   // resolution of pure virtuals
   Writer & operator<<(UCS_2 uc) {
      if (!rdstate() && (sizeof uc != rdbuf()->sputn((char *)&uc, sizeof uc))) {
         clear(rdstate() | ios::eofbit | ios::failbit);
      }
      return *this;
   }
   Writer & operator<<(char c) {
      return operator<<((UCS_2)c);
   }
   Writer & operator<<(char const * psz) {
      return operator<<(UnicodeString(psz));
   }
   Writer & operator<<(UnicodeString const & src) {
      if (!rdstate() && !src.write(rdbuf(), width(-1))) {
         clear(rdstate() | ios::eofbit | ios::failbit);
      }
      return *this;
   }
   Writer & operator<<(UnicodeSubstring const & src) {
      if (!rdstate() && !src.write(rdbuf(), width(-1))) {
         clear(rdstate() | ios::eofbit | ios::failbit);
      }
      return *this;
   }
private:
   UnicodeWriter & operator=(UnicodeWriter const &); // no!
   UnicodeWriter(UnicodeWriter const &);             // no!

};

/*-----------------------------------------------------------UnicodeMemWriter-+
| This class has bridges with UnicodeString.                                  |
| Example:                                                                    |
|                                                                             |
|    UnicodeString catenate(                                                  |
|       UnicodeString const & string1,                                        |
|       char c,                                                               |
|       UnicodeString const & string2                                         |
|    ) {                                                                      |
|       MultibyteWriter umost;                                                |
|       return umost << ucstring1 << 'a' << ucstring2;                        |
|    }                                                                        |
|                                                                             |
| This is what should be used when many catenations are desired within a      |
| UnicodeString.                                                              |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeMemWriter : public UnicodeWriter {
public:
   UnicodeMemWriter();
   UnicodeMemWriter(UCS_2 * buf, int iLen);

   UCS_2 * str()          { return (UCS_2 *)mstbuf.str(); }
   void reset()           { mstbuf.reset(); }
   int inAvail()          { return mstbuf.in_avail() / sizeof(UCS_2); }
   int pcount()           { return mstbuf.pcount() / sizeof(UCS_2); }
   bool isFixed()const    { return mstbuf.isFixed(); }
private:
   UnicodeMemWriter & operator=(UnicodeMemWriter const &); // no!
   UnicodeMemWriter(UnicodeMemWriter const &);             // no!

   MemStreamBuf mstbuf;
};

/*----------------------------------------------------------UnicodeFileWriter-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeFileWriter : public UnicodeWriter {
public:
   UnicodeFileWriter(
      char const * name,
      ios__openmode om = (ios__openmode)(ios::out | ios::binary)
   );
   ~UnicodeFileWriter();

   // ISO_14882 got rid of file descriptors!
   /*
   | UnicodeFileWriter(int fd);
   | UnicodeFileWriter(int fd, char* p, int l);
   */

private:
   UnicodeFileWriter & operator=(UnicodeFileWriter const &); // no!
   UnicodeFileWriter(UnicodeFileWriter const &);             // no!

   XxxFileStreamBuf fstbuf;
};

/*------------------------------------------------------------MultibyteWriter-+
| The MultibyteWriter class IS_LIKE an ostream, but is specifically           |
| dedicated to produce Multibyte.                                             |
|                                                                             |
| For conveniency, 2 classes have been derived:                               |
|     MultibyteMemWriter,  that creates a memory stream                       |
|     MultibyteFileWriter, that creates a file i/o stream                     |
|                                                                             |
| Example:                                                                    |
|                                                                             |
| MultibyteWriter mbost1(cout);      // output is Multibyte, using locale     |
| MultibyteWriter mbost2(cerr, "ja_sjis");  // Multibyte, shift-jis           |
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API MultibyteWriter : public Writer {
public:
   // no encoder means LOCALE
   MultibyteWriter(streambuf * bpArg) : Writer(bpArg) {
      if (!encoder.isValid()) {
         clear(rdstate() | ios::failbit);
      }
   }
   MultibyteWriter(streambuf * bpArg, Encoder const & encoderArg) :
      Writer(bpArg), encoder(encoderArg)
   {
      if (!encoder.isValid()) {
         clear(rdstate() | ios::failbit);
      }
   }
   bool resetEncoding(Encoder const & encoderArg) {
      if (!rdstate()) {
         rdbuf()->pubsync();
         encoder = encoderArg;
         if (!encoder.isValid()) {
            clear(rdstate() | ios::failbit);
         }
      }
      if (rdstate()) return false; else return true;
   }

   // non virtuals ("hide", rather than "overload")
   Writer & operator<<(int i) {
      return operator<<(UnicodeString(i));
   }
   Writer & operator<<(unsigned int i) {
      return operator<<(UnicodeString(i));
   }
   Writer & operator<<(UCS_2 const *pUc) {
      return operator<<(UcStringFlash(pUc));
   }
   Writer & operator<<(void (*f)(Writer &)) {
      f(*this);
      return *this;
   }
   Writer & operator<<(TpManipInt const & m) {
      (this->*m.fct)(m.val);
      return *this;
   }

   // resolution of pure virtuals
   Writer & operator<<(UCS_2 uc) {
      return operator<<(UcStringFlash(&uc, 1));
   }
   Writer & operator<<(char c) {
      if (rdbuf()->sputc((unsigned char)c) == -1) { // EOF
         clear(rdstate() | ios::eofbit | ios::failbit);
      }
      return *this;
   }
   Writer & operator<<(char const * psz) {
      if (!rdstate() && psz) {
         int iLen = width(-1);
         if (iLen < 0) iLen = strlen(psz);
         if (rdbuf()->sputn(psz, iLen) != iLen) {
            clear(rdstate() | ios::eofbit | ios::failbit);
         }
      }
      return *this;
   }
   Writer & operator<<(UnicodeString const & src) {
      if (!rdstate() && !src.write(rdbuf(), width(-1), encoder)) {
         clear(rdstate() | ios::eofbit | ios::failbit);
      }
      return *this;
   }
   Writer & operator<<(UnicodeSubstring const & src) {
      if (!rdstate() && !src.write(rdbuf(), width(-1), encoder)) {
         clear(rdstate() | ios::eofbit | ios::failbit);
      }
      return *this;
   }

private:
   MultibyteWriter & operator=(MultibyteWriter const &); // no!
   MultibyteWriter(MultibyteWriter const &);             // no!

   Encoder encoder;
};

/*---------------------------------------------------------MultibyteMemWriter-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API MultibyteMemWriter : public MultibyteWriter {
public:
   MultibyteMemWriter();                            // latin
   MultibyteMemWriter(char * buf, int iLen);
   MultibyteMemWriter(Encoder const & encoderArg);  // encoding 0 --> locale
   MultibyteMemWriter(
      char * buf, int iLen, Encoder const & encoderArg
   );

   char * str()          { return mstbuf.str(); }
   void reset()          { mstbuf.reset(); }
   int inAvail()         { return mstbuf.in_avail(); }
   int pcount()          { return mstbuf.pcount(); }

private:
   MultibyteMemWriter & operator=(MultibyteMemWriter const &); // no!
   MultibyteMemWriter(MultibyteMemWriter const &);             // no!

   MemStreamBuf mstbuf;
};

/*--------------------------------------------------------MultibyteFileWriter-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API MultibyteFileWriter : public MultibyteWriter {
public:
   MultibyteFileWriter(                           // latin
      char const * name,
      ios__openmode = (ios__openmode)(ios::out | ios::binary)
   );
   MultibyteFileWriter(                           // PLS
      char const * name,
      Encoder const & encoderArg,
      ios__openmode = (ios__openmode)(ios::out | ios::binary)
   );
   ~MultibyteFileWriter();

   // ISO_14882 got rid of file descriptors!
   /*
   | MultibyteFileWriter(int fd);
   | MultibyteFileWriter(int fd, char* p, int l);
   |
   | MultibyteFileWriter(
   |    int fd, Encoder const & encoderArg
   | );
   | MultibyteFileWriter(
   |    int fd, Encoder const & encoderArg, char* p, int l
   | );
   */

private:
   MultibyteFileWriter & operator=(MultibyteFileWriter const &); // no!
   MultibyteFileWriter(MultibyteFileWriter const &);             // no!

   XxxFileStreamBuf fstbuf;
};

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
