/* $Id: StringBuffer.h,v 1.15 2011-07-29 10:26:37 pgr Exp $ */

#ifndef COM_JAXO_TOOLS_STRINGBUFFER_H_INCLUDED
#define COM_JAXO_TOOLS_STRINGBUFFER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <string.h>

#if defined __MWERKS__
#include "ios.h"
#include "iostream.h"  //JAL Have to use "" instead of <> for MWERKS
#else
#include <iostream>
#endif

#include "toolsgendef.h"

#include "migstream.h"


#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------------------StringBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API StringBuffer {
public:
   StringBuffer(int capacity = 16);
   StringBuffer(char const * str, int len);
   ~StringBuffer();

   int length() const;
   int capacity() const;
   operator char const *() const;
   void ensureCapacity(int minimumCapacity);
   void setLength(int newLength);

   char charAt(int index) const;
   void setCharAt(int index, char ch);

   void append(char const * str, int len);
   void append(char c);
   void append(streambuf & is);
   void append(streambuf & is, int ncount);
   void append(streambuf & is, char delim);
   void append(streambuf & is, int ncount, char delim);
   void remove(int start, int end);
   void deleteCharAt(int index);
   void insert(int offset, char const * str, int start, int len);
   void insert(int offset, char c);
   void stripSpaces();
   void empty();

private:
   char * m_value;
   int m_count;
   int m_capacity;
   bool m_shared;

   void init(int capacity);
   void copy();
   void expandCapacity(int minimumCapacity);

   StringBuffer & operator=(StringBuffer const & source); // no!
   StringBuffer(StringBuffer const & source);             // no!
};

/* --- INLINES --- */
inline StringBuffer::StringBuffer(int capacity) { init(capacity); }

inline int StringBuffer::length() const {
   return m_count;
}

inline int StringBuffer::capacity() const {
   return m_capacity;
}

inline StringBuffer::operator char const *() const {
   /* >>>JAL:
   | Assure an ending zero sentinel.
   | I think the operator is dangerous without doing this, e.g.,
   |  foo = new StringBuffer("", 0);
   |  char const *bar = *foo;
   | You better know the capacity() was 0,
   | otherwise you're in trouble.  I know this is debatable,
   | but I spent a couple hours on this.
   */
   // Following 2 lines violates constness.
   ((StringBuffer *)this)->append('\0');     // add an extra zero
   --((StringBuffer *)this)->m_count;        // don't say it's part of the buffer
   return m_value;
}

inline void StringBuffer::empty() {  // doesn't free the buffer! (use setLength())
  m_count = 0;
}

inline void StringBuffer::append(char const * str, int len) {
   int newcount = m_count + len;
   if (newcount > m_capacity) {
      expandCapacity(newcount);
   }
   memcpy(m_value + m_count, str, len);
   m_count = newcount;
}


inline void StringBuffer::append(char c) {
   if (m_count >= m_capacity) {
      expandCapacity(m_count+1);
   }
   m_value[m_count++] = c;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
