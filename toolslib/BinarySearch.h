/* $Id: BinarySearch.h,v 1.2 2002-05-14 15:14:40 pgr Exp $ */

#ifndef COM_JAXO_TOOLS_BINARYSEARCH_H_INCLUDED
#define COM_JAXO_TOOLS_BINARYSEARCH_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------- class BinarySearch -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API BinarySearch {
public:
   int search() const;
protected:
   BinarySearch(int size);
   virtual int compare(int ix) const = 0;
private:
   int const m_size;
};

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
