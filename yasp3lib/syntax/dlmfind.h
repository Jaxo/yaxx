/*
* $Id: dlmfind.h,v 1.6 2002-04-14 23:26:04 jlatone Exp $
*
* The DelimiterFinder object recognizes SGML delimiters
* using a performant recognition algorithm.
* This code is quite complex: don't fiddle too much with.
*/

#if !defined DLMFIND_HEADER && defined __cplusplus
#define DLMFIND_HEADER

/*---------+
| Includes |
+---------*/
#include "../yasp3gendef.h"
#include "delim.h"
#include "../entity/entmgr.h"

class SgmlDecl;
class DelimList;
class DelimBuilder;
class Charset;
class Quantity;
class Features;
class UnicodeComposer;

/*----------------------------------------------------------------------------+
| VectorItem structure                                                        |
| Created by Delimiter Builder, this structure is then used internally by the |
| Delimiter Finder                                                            |
+----------------------------------------------------------------------------*/
struct VectorItem {              // VI:
   union {                       // for a secondary node, and bseq-node:
      VectorItem *next;          // pointer to next primary node, or
      VectorItem *previous;      // pointer to previous primary node
      struct {                   // for a primary node:
         bool isBseq;         // B sequence indicator
         char count;             // and count of consequent vectors
      }h;
   }f1;
   union {
      unsigned int iLenBseq;     // bseq_node: count of minimum B-seq follows
      Delimiter::e_Ix ixdl;      // delimiter code associated
   }f2;
   union {                       // either
      UCS_2 * str;           // Prep Char List / Continuation String, or
      bool isRecurse;         // bseq_node: fast recognition of a recurrence
   }f3;
   static VectorItem const Nil;  // the bad, but harmless VectorItem
};

/* These specific pointers delimit the start of VectorItem arrays related to
|  each recognition mode
*/
class YASP3_API VectorItemModePtr {
public:
   VectorItemModePtr(DelimBuilder & builder, e_RecognitionMode rmode);
   VectorItemModePtr() : pVi(&VectorItem::Nil) { }
   ~VectorItemModePtr();
   VectorItemModePtr & operator=(VectorItemModePtr const & source);
   operator VectorItem const * () const { return pVi; }  // no check!
private:
   VectorItem const * pVi;
public:
   static VectorItemModePtr const Nil;  // should be here: CSET2 bug (csd 11)
};

class YASP3_API VectorItemArray {
public:
   VectorItemArray(
      DelimList const & dlmlst,
      Features const & ft,
      UnicodeComposer & erh
   );
   void resetDelimiters(
      DelimList const & dlmlst,
      Features const & ft,
      UnicodeComposer & erh
   );
   operator void *() const {
      if (apVi[0] == VectorItemModePtr::Nil) return (void *)this; else return 0;
   }
   bool operator!() const {
      if (apVi[0] == VectorItemModePtr::Nil) return true; else return false;
   }
   VectorItem const * operator[](int i) const { return apVi[i]; }
private:
   VectorItemModePtr apVi[NBR_OF_MODES];
};

/*----------------------------------------------------------------------------+
| DelimFinder class                                                           |
+----------------------------------------------------------------------------*/
class YASP3_API DelimFinder {
public:
   DelimFinder(SgmlDecl const & sdcl, UnicodeComposer & erh);
   void resetDelimiters(SgmlDecl const & sdcl, UnicodeComposer & erh);
   void reset();

   operator void *() const;           // isOk?
   bool operator!() const;         // is not ok?

   e_RecognitionMode mode() const;
   void setMode(e_RecognitionMode rmodeNew);
   bool pushMode(e_RecognitionMode rmodeNew);

   void popMode();
   void setConMode(
      e_RecognitionMode rmodeNew,
      bool isNetToEnable,
      int iDepth,
      UnicodeString const & gi = UnicodeString::Nil
   );
   void pushMsMode(
      e_RecognitionMode rmodeNew, int iDepth = 0
   );
   bool popMsMode();

   bool isDelimStart(UCS_2 uc);
   Delimiter::e_Ix find(OpenEntityCache & oec, UCS_2 uc);
   Delimiter::e_Ix find(OpenEntityCache & oec);
   Delimiter::e_Ix delimFound() const;
   unsigned int delimLength() const;

private:
   enum { MAX_MODE_LVL = 16 };    // modes: maximum level of nesting

   Charset const & chs;
   VectorItemArray const apViEntry;
   unsigned const int iMaxBseqLen;

   UCS_2 * pUcsBase;
   unsigned int iLenDelim;
   Delimiter::e_Ix ixdl;

   e_RecognitionMode rmode;
   int ixMode;
   e_RecognitionMode modeStack[MAX_MODE_LVL];
   int iDepthMsrcd;
   int iDepthDcrcd;
   int iMarkedSectionDepth;

   bool isNetEnabled;
   UnicodeString giData;     // to recognize the ETAGO ending DCDATA

   Delimiter::e_Ix find(OpenEntityCache & oec, UCS_2 const * pUc);
   bool check_contextual_constraint(
      Delimiter::e_Ix ixdl,
      OpenEntityCache & oec,
      int iOffset
   ) const;
   bool giMatch(OpenEntityCache & oec, int iCount) const;
   DelimFinder& operator=(DelimFinder const& source);       // no!
   DelimFinder(DelimFinder const& source);                  // no!
};

/*--- INLINES ---*/
inline bool DelimFinder::isDelimStart(UCS_2 uc) {
   for (UCS_2 const * pUc = pUcsBase; *pUc; ++pUc) {
      if (*pUc == uc) return true;
   }
   return false;
}
inline Delimiter::e_Ix DelimFinder::find(OpenEntityCache & oec, UCS_2 uc) {
   for (UCS_2 const * pUc = pUcsBase; *pUc; ++pUc) {
      if (*pUc == uc) return find(oec, pUc);
   }
   return ixdl = Delimiter::IX_NOT_FOUND;
}
inline Delimiter::e_Ix DelimFinder::find(OpenEntityCache & oec)
{
   UCS_2 const uc = oec.peek(0);
   for (UCS_2 const * pUc = pUcsBase; *pUc; ++pUc) { // search the PCL
      if (*pUc == uc) return find(oec, pUc);
   }
   return ixdl = Delimiter::IX_NOT_FOUND;
}
inline DelimFinder::operator void *() const {
   if (apViEntry) return (void *)this; else return 0;
}
inline bool DelimFinder::operator!() const {
   if (apViEntry) return true; else return false;
}
inline e_RecognitionMode DelimFinder::mode() const {
   return rmode;
}
inline void DelimFinder::setMode(e_RecognitionMode rmodeNew) {
   assert  (rmodeNew > MODE_MS_INCLUDE); // not for CON_xxx, nor MS_RCDATA
   pUcsBase = apViEntry[rmode = rmodeNew]->f3.str;
}
inline bool DelimFinder::pushMode(e_RecognitionMode rmodeNew) {
   assert  (ixMode < MAX_MODE_LVL);
   modeStack[ixMode++] = rmode;
   setMode(rmodeNew);
   return true;
}
inline void DelimFinder::popMode() {
   pUcsBase = apViEntry[rmode = modeStack[--ixMode]]->f3.str;
}
inline Delimiter::e_Ix DelimFinder::delimFound() const {
   return ixdl;
}
inline unsigned int DelimFinder::delimLength() const {
   if (ixdl != Delimiter::IX_NOT_FOUND) {
      return iLenDelim;
   }else {
      return 0;
   }
}
#endif /* DLMFIND_HEADER ====================================================*/
