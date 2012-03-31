/* $Id: CodeBuffer.h,v 1.33 2011-07-29 10:26:35 pgr Exp $ */

#ifndef COM_JAXO_YAXX_CODEBUFFER_H_INCLUDED
#define COM_JAXO_YAXX_CODEBUFFER_H_INCLUDED

/*---------+
| Includes |
+---------*/

// class streambuf;

#include "Constants.h"
#include "Arguments.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class Symbol;
class Label;

#if !defined _WIN32 || defined NDEBUG
typedef unsigned short CodePosition;

#else
/*------------------------------------------------------- class CodePosition -+
| ** DEBUG ONLY **   This class should be only defined in DEBUG mode.         |
| It performs a full check that no assumption is made on the kind of integer  |
| a CodePosition carries in other modules than just CodeBuf.                  |
+----------------------------------------------------------------------------*/
class REXX_API CodePosition {
   friend class CodeBuffer;
public:
   CodePosition();
   CodePosition(CodePosition const & src);
   CodePosition & operator=(CodePosition const & src);
   bool operator==(CodePosition const & src) const;
   bool operator!=(CodePosition const & src) const;
   bool operator>=(CodePosition const & src) const;
   bool operator>(CodePosition const & src) const;
   bool operator<(CodePosition const & src) const;
private:
   unsigned short m_val;
   CodePosition::CodePosition(int val);
   operator unsigned short();
   unsigned short CodePosition::operator+=(unsigned short incr);
};

/* -- INLINES -- */
inline CodePosition::CodePosition() {
   m_val = 0xEFBE;
}
inline CodePosition::CodePosition(int val) {
   m_val = (unsigned short)val;
}
inline CodePosition::CodePosition(CodePosition const & src) {
   m_val = src.m_val;
}
inline CodePosition & CodePosition::operator=(CodePosition const & src) {
   m_val = src.m_val; return *this;
}
inline bool CodePosition::operator==(CodePosition const & src) const {
   return m_val == src.m_val;
}
inline bool CodePosition::operator!=(CodePosition const & src) const {
   return m_val != src.m_val;
}
inline bool CodePosition::operator>=(CodePosition const & src) const {
   return m_val >= src.m_val;
}
inline bool CodePosition::operator>(CodePosition const & src) const {
   return m_val > src.m_val;
}
inline bool CodePosition::operator<(CodePosition const & src) const {
   return m_val < src.m_val;
}
inline CodePosition::operator unsigned short() {
   return m_val;
}
inline unsigned short CodePosition::operator+=(unsigned short incr) {
   return m_val += incr;
}
#endif

/*--------------------------------------------------------- class CodeBuffer -+
|                                                                             |
+----------------------------------------------------------------------------*/
class REXX_API CodeBuffer {
public:
   static CodePosition Start;
   static CodePosition Unknown;

   struct CallParams {             // to process calls (ensure correct order)
      Label * label;
      Arguments::PresenceBits presenceBits;
      unsigned short sigl;
      CallType ct;
   };

   CodeBuffer(streambuf * sb = 0);
   ~CodeBuffer();
   CodePosition getEndPos() const;
   CodePosition getRunPos() const;

   void addOp(OpCode toAdd);
   void addOpByte(OpCode op, unsigned char arg);
   void addOpWord(OpCode op, unsigned int arg);
   void addOpPos(OpCode op, CodePosition pos);
   void addOpBytePos(OpCode op, unsigned char arg, CodePosition pos);
   void addOpPtr(OpCode op, void const * p);
   void addOpSymbol(OpCode op, Symbol const * sym);
   void addOpString(OpCode op, RexxString const & str);
   void addOpByteSymbol(OpCode op, unsigned char arg, Symbol const * sym);
   void addOpByteSymbolPos(
      OpCode op,
      unsigned char arg,
      Symbol const * sym,
      CodePosition pos
   );
   void addOpByteSymbolPosByte(
      OpCode op,
      unsigned char arg1,
      Symbol const * sym,
      CodePosition pos,
      unsigned char arg2
   );
   void addOpCallParams(OpCode op, CallParams & parms);
   void addPointer(void const * p);
   CodePosition addOpNoPos(OpCode op);
   CodePosition addOpNoByte(OpCode op);
   void resolvePos(CodePosition pos);
   void resolveByte(CodePosition pos, unsigned char toSet);
   void insertByte(CodePosition pos, unsigned char b);

   OpCode readOpCode();
   void unreadOpCode();
   OpCode peekOpCode() const;
   unsigned char readByte();
   CodePosition readPos();
   unsigned int readWord();
   void * readPointer();
   void readString(RexxString ** pStr);
   void readCallParams(CallParams &);

   void jumpTo(CodePosition pos);
   void truncate(CodePosition pos);

private:
   streambuf & m_sb;
   bool m_isSbOwned;
   CodePosition m_posWrite;

private:
   void writeByte(unsigned char c);
   void writeWord(unsigned int i);
   void writePointer(void const * ptr);
};

/*-- INLINES -- */

inline void CodeBuffer::jumpTo(CodePosition pos) {
   m_sb.pubseekpos(pos, ios::in);
}
inline CodePosition CodeBuffer::getEndPos() const {
   return m_posWrite;
}
inline CodePosition CodeBuffer::getRunPos() const {
   CodePosition pos = (unsigned short)((CodeBuffer * )this)->m_sb.pubseekoff(
      0, ios::cur, ios::in
   );
   return pos;
}
inline OpCode CodeBuffer::readOpCode() {
   return (OpCode)m_sb.sbumpc();
}
inline OpCode CodeBuffer::peekOpCode() const {
   return (OpCode)((CodeBuffer * )this)->m_sb.sgetc();
}
inline void CodeBuffer::unreadOpCode() {
   ((CodeBuffer * )this)->m_sb.pubseekoff(-1, ios::cur, ios::in);
}
inline unsigned char CodeBuffer::readByte() {
   return (unsigned char)((CodeBuffer * )this)->m_sb.sbumpc();
}
inline unsigned int CodeBuffer::readWord() {
   unsigned short s;
   m_sb.sgetn((char *)&s, sizeof s);
   return s;
}
inline CodePosition CodeBuffer::readPos() {
   return (CodePosition)readWord();
}
inline void * CodeBuffer::readPointer() {
   void * p;
   m_sb.sgetn((char *)&p, sizeof p);
   return p;
}
inline void CodeBuffer::readString(RexxString ** ppStr) {
   m_sb.sgetn((char *)ppStr, sizeof (RexxString *));
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
