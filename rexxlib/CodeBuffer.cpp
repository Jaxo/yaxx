/* $Id: CodeBuffer.cpp,v 1.25 2002-11-13 21:06:24 jlatone Exp $ */

#include <string.h>
#include "CodeBuffer.h"
#include "../toolslib/MemStream.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

CodePosition CodeBuffer::Start(0);
CodePosition CodeBuffer::Unknown(0xFFFF);

/*-----------------------------------------------------CodeBuffer::CodeBuffer-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
CodeBuffer::CodeBuffer(streambuf * sb) :
   m_sb(*((sb == 0)? new MemStreamBuf() : sb)),
   m_posWrite(0)
{
   m_isSbOwned = (sb == 0);
   assert (sizeof (unsigned char) == 1);
   assert (sizeof (unsigned short) == 2);
   assert (sizeof (void *) == 4);
}

/*----------------------------------------------------CodeBuffer::~CodeBuffer-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
CodeBuffer::~CodeBuffer() {
   if (m_isSbOwned) delete &m_sb;
}

/*------------------------------------------------------CodeBuffer::writeByte-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline void CodeBuffer::writeByte(unsigned char c) {
   m_sb.sputc(c);
}

/*------------------------------------------------------CodeBuffer::writeWord-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline void CodeBuffer::writeWord(unsigned int i) {
   unsigned short s = i;
   m_sb.sputc(*(unsigned char *)&s);
   m_sb.sputc(*((unsigned char *)&s+1));

}

/*---------------------------------------------------CodeBuffer::writePointer-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline void CodeBuffer::writePointer(void const * p) {
   m_sb.sputn((char *)&p, sizeof p);
}

/*-----------------------------------------------------CodeBuffer::addPointer-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addPointer(void const * p) {
   m_posWrite += sizeof p;
   writePointer(p);
}

/*----------------------------------------------------------CodeBuffer::addOp-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOp(OpCode op) {
   m_posWrite += 1;
   writeByte((unsigned char)op);
}

/*------------------------------------------------------CodeBuffer::addOpByte-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpByte(OpCode op, unsigned char arg) {
   m_posWrite += 1+1;
   writeByte((unsigned char)op);
   writeByte((unsigned char)arg);
}

/*---------------------------------------------------CodeBuffer::addOpBytePos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpBytePos(OpCode op, unsigned char arg, CodePosition pos) {
   m_posWrite += 1+1+sizeof pos;
   writeByte((unsigned char)op);
   writeByte(arg);
   writeWord(pos);
}

/*-------------------------------------------------------CodeBuffer::addOpPos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpPos(OpCode op, CodePosition pos) {
   m_posWrite += 1+sizeof pos;
   writeByte((unsigned char)op);
   writeWord(pos);
}

/*------------------------------------------------------CodeBuffer::addOpWord-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpWord(OpCode op, unsigned int arg) {
   m_posWrite += 1+sizeof (unsigned short);
   writeByte((unsigned char)op);
   writeWord(arg);
}

/*-------------------------------------------------------CodeBuffer::addOpPtr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpPtr(OpCode op, void const * p) {
   m_posWrite += 1+sizeof p;
   writeByte((unsigned char)op);
   writePointer(p);
}

/*----------------------------------------------------CodeBuffer::addOpSymbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpSymbol(
   OpCode op,
   Symbol const * p
) {
   m_posWrite += 1 + sizeof p;
   writeByte((unsigned char)op);
   writePointer(p);
}

/*----------------------------------------------------CodeBuffer::addOpString-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpString(
   OpCode op,
   RexxString const & str
) {
   m_posWrite += 1 + sizeof (void *);
   writeByte((unsigned char)op);
   writePointer((void *)&str);
}

/*----------------------------------------------------CodeBuffer::addOpSymbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpByteSymbol(
   OpCode op,
   unsigned char arg,
   Symbol const * p
) {
   m_posWrite += 1 + 1 + sizeof p;
   writeByte((unsigned char)op);
   writeByte(arg);
   writePointer(p);
}

/*---------------------------------------------CodeBuffer::addOpByteSymbolPos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpByteSymbolPos(
   OpCode op,
   unsigned char arg,
   Symbol const * p,
   CodePosition pos
) {
   m_posWrite += 1 + 1 + sizeof p + sizeof pos;
   writeByte((unsigned char)op);
   writeByte(arg);
   writePointer(p);
   writeWord(pos);
}

/*-----------------------------------------CodeBuffer::addOpByteSymbolPosByte-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpByteSymbolPosByte(
   OpCode op,
   unsigned char arg1,
   Symbol const * p,
   CodePosition pos,
   unsigned char arg2
) {
   m_posWrite += 1 + 1 + sizeof p + sizeof pos + 1;
   writeByte((unsigned char)op);
   writeByte(arg1);
   writePointer(p);
   writeWord(pos);
   writeByte(arg2);
}

/*------------------------------------------------CodeBuffer::addOpCallParams-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::addOpCallParams(OpCode op, CallParams & parms)
{
   m_posWrite += (1 + sizeof parms);
   writeByte((unsigned char)op);
   m_sb.sputn((char *)&parms, sizeof parms);
}

/*-------------------------------------------------CodeBuffer::readCallParams-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::readCallParams(CallParams & parms) {
   m_sb.sgetn((char *)&parms, sizeof parms);
}

/*-----------------------------------------------------CodeBuffer::addOpNoPos-+
|                                                                             |
+----------------------------------------------------------------------------*/
CodePosition CodeBuffer::addOpNoPos(OpCode op) {
   CodePosition here(m_posWrite);
   addOpWord(op, 0);
   return here;
}

/*----------------------------------------------------CodeBuffer::addOpNoByte-+
|                                                                             |
+----------------------------------------------------------------------------*/
CodePosition CodeBuffer::addOpNoByte(OpCode op) {
   CodePosition here(m_posWrite);
   addOpByte(op, 0);
   return here;
}

/*-----------------------------------------------------CodeBuffer::resolvePos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::resolvePos(CodePosition pos) {
   unsigned short s = m_posWrite;
   m_sb.pubseekpos(pos+1, ios::out);
   writeWord(s);
   m_sb.pubseekpos(m_posWrite, ios::out);
}

/*----------------------------------------------------CodeBuffer::resolveByte-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::resolveByte(CodePosition pos, unsigned char toSet) {
   unsigned short s = m_posWrite;
   m_sb.pubseekpos(pos+1, ios::out);
   writeByte(toSet);
   m_sb.pubseekpos(m_posWrite, ios::out);
}

/*-----------------------------------------------------CodeBuffer::insertByte-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::insertByte(CodePosition pos, unsigned char b)
{
   int size = m_posWrite - pos;
   char * temp = new char[size];
   m_sb.pubseekpos(pos, ios::in);
   m_sb.sgetn(temp, size);
   m_sb.pubseekpos(0, ios::in);
   m_sb.pubseekpos(pos, ios::out);
   m_sb.sputc(b);
   m_sb.sputn(temp, size);
   delete [] temp;
   m_posWrite += 1;
}

/*-------------------------------------------------------CodeBuffer::truncate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void CodeBuffer::truncate(CodePosition pos) {
   m_sb.pubsync();
   m_sb.pubseekpos(pos, ios::out);
   m_posWrite = pos;
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
