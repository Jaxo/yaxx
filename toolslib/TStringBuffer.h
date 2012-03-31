/*
* $Id: TStringBuffer.h,v 1.3 2002-12-19 06:46:43 pgr Exp $
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

#ifndef COM_JAXO_TOOLS_TSTRINGBUFFER_H_INCLUDED
#define COM_JAXO_TOOLS_TSTRINGBUFFER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#if defined __MWERKS__
#include "ios.h"
#include "iostream.h"  //JAL Have to use "" instead of <> for MWERKS
#else
#include <iostream.h>
#endif

#include "toolsgendef.h"
#include "migstream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

template <class T> class TOOLS_API TString;

/*--------------------------------------------------------------TStringBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
class TOOLS_API TStringBuffer {
public:
   TStringBuffer(int capacity = 16);
   TStringBuffer(TString<T> const & str);
   TStringBuffer(T const * str, int len);
   ~TStringBuffer();

   int length() const;
   int capacity() const;
   operator TString<T> () const;
   operator T const *() const;
   void ensureCapacity(int minimumCapacity);
   void setLength(int newLength);

   T charAt(int index) const;
   void setCharAt(int index, T ch);

   void append(TString<T> const str);
   void append(T const * str, int len);
   void append(T c);
   void append(streambuf & is);
   void append(streambuf & is, int ncount);
   void append(streambuf & is, T delim);
   void append(streambuf & is, int ncount, T delim);
   void remove(int start, int end);
   void deleteCharAt(int index);
   void insert(int offset, T const * str, int start, int len);
   void insert(int offset, T c);
   void stripSpaces();
   void empty();

private:
   T * m_value;
   int m_count;
   int m_capacity;
   bool m_shared;

   void init(int capacity);
   void copy();
   void expandCapacity(int minimumCapacity);

   TStringBuffer & operator=(TStringBuffer const & src) { return *this; }; // no!
   TStringBuffer(TStringBuffer const & source) {};                         // no!
};

/* --- INLINES --- */
template <class T> inline TStringBuffer<T>::TStringBuffer(int capacity) {
   init(capacity);
}

template <class T> inline int TStringBuffer<T>::length() const {
   return m_count;
}

template <class T> inline int TStringBuffer<T>::capacity() const {
   return m_capacity;
}

template <class T> inline TStringBuffer<T>::operator TString<T> () const {
   return TString<T>(m_value, m_count);
}

template <class T> inline void TStringBuffer<T>::empty() {
   // doesn't free the buffer! (use setLength())
   m_count = 0;
}

template <class T> inline void TStringBuffer<T>::append(TString<T> const str) {
   append(str, str.length());
}

template <class T> inline void TStringBuffer<T>::append(T c) {
   if (m_count >= m_capacity) {
      expandCapacity(m_count+1);
   }
   m_value[m_count++] = c;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif // COM_JAXO_TOOLS_TSTRINGBUFFER_H_INCLUDED


#ifdef JAXO_INSTANCIATE_TEMPLATE
#include "TStringBuffer.cpp"
#endif
/*===========================================================================*/
