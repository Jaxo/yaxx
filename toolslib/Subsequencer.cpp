/*
* $Id: Subsequencer.cpp,v 1.2 2003-01-26 10:59:51 pgr Exp $
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

/*--------------+
| Include Files |
+--------------*/
#include "Subsequencer.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------Subsequencer::getLongest-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Subsequencer::getLongest()
{
   int i1;
   int i2;
   int size = (m_len1+1) * (m_len2+1);
   short * array = new short[size];
   short * pVal = array + size - 1;

   for (i2 = m_len2; i2 >= 0; --i2, --pVal) *pVal = 0;
   for (i1 = m_len1-1; i1 >= 0; --i1) {
      *pVal-- = 0;
      for (i2 = m_len2-1; i2 >= 0; --i2, --pVal) {
         if (isEqual(i1, i2)) {
            *pVal = 1 + *(pVal+m_len2+2);
         }else {
            if (*(pVal+m_len2+1) > *(pVal+1)) {
               *pVal = *(pVal+m_len2+1);
            }else {
               *pVal = *(pVal+1);
            }
         }
      }
   }
   i1 = 0;
   i2 = 0;
   pVal = array;
   b2ndPass = true;
   while ((i1 < m_len1) && (i2 < m_len2)) {
      if (isEqual(i1, i2)) {
         tellEqual(i1, i2);
         pVal += m_len2+2, ++i1, ++i2;
      }else {
         if (*(pVal+m_len2+1) >= *(pVal+1))  {
            pVal += m_len2+1, ++i1;
         }else {
            ++pVal, ++i2;
         }
      }
   }
   delete [] array;
}


#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
