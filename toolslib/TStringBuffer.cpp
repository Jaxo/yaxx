/*
* $Id: TStringBuffer.cpp,v 1.2 2002-12-19 06:46:43 pgr Exp $
*
* (C) Copyright Jaxo Systems, Inc. 2002
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 12/15/2002
*
*/

#ifdef JAXO_INSTANCIATE_TEMPLATE   // otherwise, this file is not compiled

/*--------------+
| Include Files |
+--------------*/
#include <string.h>
#include <assert.h>
#include <new.h>
#include <limits.h>

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------------TStringBuffer::init-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::init(int capacity) {
   assert (capacity >= 0);
   m_value = new T[capacity];
   m_shared = false;
   m_capacity = capacity;
   m_count = 0;
}

/*-----------------------------------------------TStringBuffer::TStringBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TStringBuffer<T>::TStringBuffer(TString<T> const & str) {
   int len = str.length();
   init(len + 16);
   append(str, len);
}

/*-----------------------------------------------TStringBuffer::TStringBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TStringBuffer<T>::TStringBuffer(T const * str, int len) {
   init(len + 16);
   append(str, len);
}

/*----------------------------------------------TStringBuffer::~TStringBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TStringBuffer<T>::~TStringBuffer() {
   delete [] m_value;
}

/*------------------------------------------TStringBuffer::operator T const *-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TStringBuffer<T>::operator T const *() const {
   /* >>>JAL:
   | Assure an ending zero sentinel.
   | I think the operator is dangerous without doing this, e.g.,
   |  foo = new TStringBuffer("", 0);
   |  T const *bar = *foo;
   | You better know the capacity() was 0,
   | otherwise you're in trouble.  I know this is debatable,
   | but I spent a couple hours on this.
   */
   // Following 2 lines violates constness.
   ((TStringBuffer *)this)->append((T)0);  // add an extra zero
   --((TStringBuffer *)this)->m_count;     // don't say it's part of the buffer
   return m_value;
}

/*------------------------------------------------------TStringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T> inline void TStringBuffer<T>::append(T const * str, int len) {
   int newcount = m_count + len;
   if (newcount > m_capacity) {
      expandCapacity(newcount);
   }
   memcpy(m_value + m_count, str, len * sizeof(T));
   m_count = newcount;
}

/*------------------------------------------------------TStringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::append(streambuf & sb)
{
   int const INCRMT = 200;
   do {
      ensureCapacity(m_count + INCRMT);
      m_count += sb.sgetn((char *)(m_value+m_count), INCRMT);
   }while (sb.sgetc() != EOF);
}

/*------------------------------------------------------TStringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::append(streambuf & sb, int ncount)
{
   assert(ncount >= 0);
   ncount += m_count;
   ensureCapacity(ncount);
   do {
      m_count += sb.sgetn((char *)(m_value+m_count), ncount-m_count);
   }while ((m_count < ncount) && (sb.sgetc() != EOF));
}

/*------------------------------------------------------TStringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::append(streambuf & sb, T delim)
{
   int c;
   while (EOF != (c=sb.sbumpc())) {
      if (c == (int)delim) {
         sb.sputbackc(c);
         break;
      }
      append(c);
   }
}

/*------------------------------------------------------TStringBuffer::append-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::append(streambuf & sb, int ncount, T delim)
{
   int c;
   while (ncount-- && (EOF != (c=sb.sbumpc()))) {
      if (c == (int)delim) {
         sb.sputbackc(c);
         break;
      }
      append(c);
   }
}

/*----------------------------------------------TStringBuffer::ensureCapacity-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::ensureCapacity(int minimumCapacity) {
   if (minimumCapacity > m_capacity) {
      expandCapacity(minimumCapacity);
   }
}

/*---------------------------------------------------TStringBuffer::setLength-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::setLength(int newLength)
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
            // If newLength is zero, assume the TStringBuffer is being
            // stripped for reuse; Make new buffer of default size
            m_capacity = 16;
            delete [] m_value;
            m_value = new T[16];
            m_shared = false;
         }
      }
   }
}

/*------------------------------------------------------TStringBuffer::charAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
T TStringBuffer<T>::charAt(int index) const {
   assert ((index >= 0) && (index < m_count));
   return m_value[index];
}

/*---------------------------------------------------TStringBuffer::setCharAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::setCharAt(int index, T ch) {
   assert ((index >= 0) && (index < m_count));
   if (m_shared) copy();
   m_value[index] = ch;
}

/*-------------------------------------------------TStringBuffer::stripSpaces-+
| Remove leading and trailing spaces.                                         |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::stripSpaces()
{
   T * c_p = m_value;
   while (m_count && (*c_p <= ' ')) { ++c_p, --m_count; }
   memmove(m_value, c_p, m_count * sizeof (T));
   c_p = m_value + m_count;
   while (m_count && (*--c_p <= ' ')) { --m_count; }
}

/*------------------------------------------------------TStringBuffer::remove-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::remove(int start, int end) {
   assert (start >= 0);
   if (end > m_count) {
      end = m_count;
   }
   if (end > start) {
      if (m_shared) copy();
      memmove(m_value+start, m_value+end, (m_count-end) * sizeof (T));
      m_count -= (end - start);
   }else {
      assert (start == end);
   }
}

/*------------------------------------------------TStringBuffer::deleteCharAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::deleteCharAt(int index) {
   assert ((index >= 0) && (index < m_count));
   if (m_shared) copy();
   memcpy(m_value+index, m_value+index+1, (m_count-index-1) * sizeof (T));
   --m_count;
}

/*------------------------------------------------------TStringBuffer::insert-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::insert(int offset, T const * str, int start, int len)
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
      memcpy(m_value+offset+len, m_value+offset, (m_count-offset) * sizeof (T));
      memcpy(m_value+offset, str+start, len * sizeof (T));
      m_count = newCount;
   }
}

/*------------------------------------------------------TStringBuffer::insert-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::insert(int offset, T c) {
   int newcount = m_count + 1;
   if (newcount > m_capacity) {
      expandCapacity(newcount);
   }else if (m_shared) {
      copy();
   }
   memcpy(m_value+offset+1, m_value+offset, (m_count-offset) * sizeof (T));
   m_value[offset] = c;
   m_count = newcount;
}

/*--------------------------------------------------------TStringBuffer::copy-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::copy() {
   T * newValue = new T[m_capacity];
   memcpy(newValue, m_value, m_count * sizeof (T));
   delete [] m_value;
   m_value = newValue;
   m_shared = false;
}

/*----------------------------------------------TStringBuffer::expandCapacity-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringBuffer<T>::expandCapacity(int minimumCapacity) {
   m_capacity = (m_capacity + 1) * 2;
   if (m_capacity < 0) {
      m_capacity = INT_MAX;
   }else if (m_capacity < minimumCapacity) {
      m_capacity = minimumCapacity;
   }
   T * newValue = new T[m_capacity];
   memcpy(newValue, m_value, m_count * sizeof (T));
   delete [] m_value;
   m_value = newValue;
   m_shared = false;
}

#ifdef TOOLS_NAMESPACE
}
#endif

#endif // JAXO_INSTANCIATE_TEMPLATE
/*===========================================================================*/
