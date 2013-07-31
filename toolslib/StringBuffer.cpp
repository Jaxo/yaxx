/* $Id: StringBuffer.cpp,v 1.13 2011-07-29 10:26:37 pgr Exp $ */

#include <string.h>
#include <assert.h>
#include <new>
#include <limits.h>
#include "StringBuffer.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------------StringBuffer::init-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::init(int capacity) {
   assert (capacity >= 0);
   m_value = new char[capacity];
   m_shared = false;
   m_capacity = capacity;
   m_count = 0;
}

/*-------------------------------------------------StringBuffer::StringBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
StringBuffer::StringBuffer(char const * str, int len) {
   init(len + 16);
   append(str, len);
}

/*------------------------------------------------StringBuffer::~StringBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
StringBuffer::~StringBuffer() {
   delete [] m_value;
}

/*-------------------------------------------------------StringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::append(streambuf & sb)
{
   int const INCRMT = 200;
   do {
      ensureCapacity(m_count + INCRMT);
      m_count += sb.sgetn(m_value+m_count, INCRMT);
   }while (sb.sgetc() != EOF);
}

/*-------------------------------------------------------StringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::append(streambuf & sb, int ncount)
{
   assert(ncount >= 0);
   ncount += m_count;
   ensureCapacity(ncount);
   do {
      m_count += sb.sgetn(m_value+m_count, ncount-m_count);
   }while ((m_count < ncount) && (sb.sgetc() != EOF));
}

/*-------------------------------------------------------StringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::append(streambuf & sb, char delim)
{
   int c;
   while (EOF != (c=sb.sbumpc())) {
      if (c == delim) {
         sb.sputbackc(c);
         break;
      }
      append(c);
   }
}

/*-------------------------------------------------------StringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::append(streambuf & sb, int ncount, char delim)
{
   int c;
   while (ncount-- && (EOF != (c=sb.sbumpc()))) {
      if (c == delim) {
         sb.sputbackc(c);
         break;
      }
      append(c);
   }
}

/*-----------------------------------------------StringBuffer::ensureCapacity-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::ensureCapacity(int minimumCapacity) {
   if (minimumCapacity > m_capacity) {
      expandCapacity(minimumCapacity);
   }
}

/*----------------------------------------------------StringBuffer::setLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::setLength(int newLength)
{
   assert (newLength >= 0);
   if (newLength > m_capacity) {
      expandCapacity(newLength);
   }
   if (newLength > m_count) {
      if (m_shared) copy();
      while (m_count < newLength) {
         m_value[m_count++] = '\0';
      }
   }else {
      m_count = newLength;
      if (m_shared) {
         if (newLength > 0) {
            copy();
         }else {
            // If newLength is zero, assume the StringBuffer is being
            // stripped for reuse; Make new buffer of default size
            m_capacity = 16;
            delete [] m_value;
            m_value = new char[16];
            m_shared = false;
         }
      }
   }
}

/*-------------------------------------------------------StringBuffer::charAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
char StringBuffer::charAt(int index) const {
   assert ((index >= 0) && (index < m_count));
   return m_value[index];
}

/*----------------------------------------------------StringBuffer::setCharAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::setCharAt(int index, char ch) {
   assert ((index >= 0) && (index < m_count));
   if (m_shared) copy();
   m_value[index] = ch;
}

/*--------------------------------------------------StringBuffer::stripSpaces-+
| Remove leading and trailing spaces.                                         |
+----------------------------------------------------------------------------*/
void StringBuffer::stripSpaces()
{
   char * c_p = m_value;
   while (m_count && (*c_p <= ' ')) { ++c_p, --m_count; }
   memmove(m_value, c_p, m_count);
   c_p = m_value + m_count;
   while (m_count && (*--c_p <= ' ')) { --m_count; }
}

/*-------------------------------------------------------StringBuffer::remove-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::remove(int start, int end) {
   assert (start >= 0);
   if (end > m_count) {
      end = m_count;
   }
   if (end > start) {
      if (m_shared) copy();
      memmove(m_value+start, m_value+end, m_count-end);
      m_count -= (end - start);
   }else {
      assert (start == end);
   }
}

/*-------------------------------------------------StringBuffer::deleteCharAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::deleteCharAt(int index) {
   assert ((index >= 0) && (index < m_count));
   if (m_shared) copy();
   memcpy(m_value+index, m_value+index+1, m_count-index-1);
   --m_count;
}

/*-------------------------------------------------------StringBuffer::insert-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::insert(int offset, char const * str, int start, int len)
{
   assert ((offset >= 0) && (offset < m_count));
   assert ((start >= 0) && (start + len >= 0));
   assert (len >= 0);
   if (len) {
      int newCount = m_count + len;
      if (newCount > m_capacity) {
         expandCapacity(newCount);
      }else if (m_shared) {
         copy();
      }
      memcpy(m_value+offset+len, m_value+offset, m_count-offset);
      memcpy(m_value+offset, str+start, len);
      m_count = newCount;
   }
}

/*-------------------------------------------------------StringBuffer::insert-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::insert(int offset, char c) {
   int newcount = m_count + 1;
   if (newcount > m_capacity) {
      expandCapacity(newcount);
   }else if (m_shared) {
      copy();
   }
   memcpy(m_value+offset+1, m_value+offset, m_count-offset);
   m_value[offset] = c;
   m_count = newcount;
}

/*---------------------------------------------------------StringBuffer::copy-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::copy() {
   char * newValue = new char[m_capacity];
   memcpy(newValue, m_value, m_count);
   delete [] m_value;
   m_value = newValue;
   m_shared = false;
}

/*-----------------------------------------------StringBuffer::expandCapacity-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StringBuffer::expandCapacity(int minimumCapacity) {
   m_capacity = (m_capacity + 1) * 2;
   if (m_capacity < 0) {
      m_capacity = INT_MAX;
   }else if (m_capacity < minimumCapacity) {
      m_capacity = minimumCapacity;
   }
   char * newValue = new char[m_capacity];
   memcpy(newValue, m_value, m_count);
   delete [] m_value;
   m_value = newValue;
   m_shared = false;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
