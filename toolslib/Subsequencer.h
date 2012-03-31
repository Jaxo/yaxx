/*
* $Id: Subsequencer.h,v 1.2 2003-01-26 10:59:51 pgr Exp $
*
* (C) Copyright Jaxo Systems, Inc. 2002
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 12/16/2002
*
*/

#ifndef COM_JAXO_TOOLS_SEQUENCER_H_INCLUDED
#define COM_JAXO_TOOLS_SEQUENCER_H_INCLUDED
#ifdef __cplusplus

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------- class Subsequencer -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Subsequencer {
public:
   Subsequencer(int len1, int len2);
   void getLongest();
   int longestMaxLength() const;
protected:
   virtual bool isEqual(int i1, int i2) = 0;
   virtual void tellEqual(int i1, int i2) = 0;
   bool is2ndPass() const;
private:
   int const m_len1;
   int const m_len2;
   bool b2ndPass;
};

/* -- INLINES -- */
inline Subsequencer::Subsequencer(int len1, int len2) :
   m_len1(len1), m_len2(len2), b2ndPass(false) {
   /* empty body */
}
inline int Subsequencer::longestMaxLength() const {
   return (m_len1 > m_len2)? m_len1 : m_len2;
}
inline bool Subsequencer::is2ndPass() const {
   return b2ndPass;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif /* __cplusplus */
#endif // COM_JAXO_TOOLS_SEQUENCER_H_INCLUDED

/*===========================================================================*/
