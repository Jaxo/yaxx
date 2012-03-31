/*
* $Id: ByteString.h,v 1.6 2002-11-20 05:53:03 jlatone Exp $
*/

#ifndef COM_JAXO_TOOLS_BYTESTRING_H_INCLUDED
#define COM_JAXO_TOOLS_BYTESTRING_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"
typedef char Byte;

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------------- class ByteString -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API ByteString
{
public:
   class TOOLS_API Rep {
   /*
   | This class has been designed for performance.
   | C++ does not provide any light way to both reuse a base
   | code for Strings and allow extensions thru derivations.
   */

   public:
      struct NewParam {
         NewParam(int bufSize, int fixedSize) :  // regular
            m_fixedSize(fixedSize),
            m_bufSize(bufSize)
         {}
         NewParam(void * staticMem) :            // static (for Nil);
            m_fixedSize(0),
            m_staticMem(staticMem)
         {}
         size_t m_fixedSize;
         union {
            int m_bufSize;
            void * m_staticMem;
         };
      };

      void * operator new(size_t, NewParam *);

      Rep();
      Rep(NewParam & p);
      virtual ~Rep() {}

      /*
      | Basic string methods
      */
      int skipSpaces(int i) const;
      int skipWord(int i) const;
      int wordIndex(int wordNo) const;
      int wordPos(Rep const * phrase, int wordNo) const;
      int wordsCount() const;
      int pos(Rep const * needle, int pos) const;
      int lastpos(Rep const * needle, int pos) const;
      int compare(Rep const * against, char pad) const;

      unsigned short iReferenceCount;
      /*
      | following const makes that m_pch will never be zeroed by mistake
      | and ensures that a Nil is recognized by m_pch = 0;
      */
      Byte * const m_pch;      // Data buffer
      int m_len;               // Actual length of string

   private:
      Rep & operator=(Rep const& repSource);        // no!
      Rep(Rep const & repSource);                   // no!
   };

   static ByteString const NilByte;

   ByteString();
   ~ByteString();
   ByteString(ByteString const & source);
   ByteString & operator=(ByteString const & source);
   bool exists() const;
   operator Byte const *() const;
   unsigned int hash() const;

protected:
   Rep * m_pRep;

private:
   ByteString(void ***);       // "Nil" constructor
};


/* --- INLINES --- */
inline bool isSpace(char c) {
   return ((c <=  ' ') && ((c==' ')||(c == '\n')||(c=='\t')||(c=='\r')));
}

inline bool ByteString::exists() const {
   return (m_pRep->m_pch != 0);
}

inline ByteString & ByteString::operator=(ByteString const & source) {
   if (m_pRep != source.m_pRep) {
      this->~ByteString();
      new(this) ByteString(source);
   }
   return *this;
}

inline ByteString::operator Byte const *() const {
   return m_pRep->m_pch;
}

inline ByteString::ByteString(ByteString const & source) {
   m_pRep = source.m_pRep;
   if (m_pRep->m_pch) {
      ++m_pRep->iReferenceCount;
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
