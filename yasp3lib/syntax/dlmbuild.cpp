/*
* $Id: dlmbuild.cpp,v 1.4 2011-07-29 10:26:40 pgr Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include <iostream>
#include "charset.h"
#include "dlmlist.h"
#include "dlmfind.h"
#include "dlmbuild.h"
#include "../../reslib/resources.h"
#include "../../toolslib/uccompos.h"

/*--------------------------------------------------------DelimNode::DelimNode-+
| DelimNode NULL constructor for a Head                                        |
+-----------------------------------------------------------------------------*/
inline DelimNode::DelimNode()
{
   isParent = false;
   isSibbled = true;
   isBseq = false;
   child = sibling = this;
}

/*--------------------------------------------------------DelimNode::DelimNode-+
| DelimNode constructor for a child, or for a sibling                          |
+-----------------------------------------------------------------------------*/
inline DelimNode::DelimNode(DelimNode *pTrRelative, bool isChild)
{
   isSibbled = isParent = false;      // just threads for the moment
   isBseq = false;                    // default
   if (isChild) {
      sibling = pTrRelative;          // set father thread
      child = pTrRelative->child;     // inherits brother/uncle thread
      pTrRelative->isParent = true;   // Brand new dad
      pTrRelative->child = this;      // link father to his child
   }else {
      child = pTrRelative;            // set elder thread
      sibling = pTrRelative->sibling; // inherits father thread
      pTrRelative->isSibbled = true;  // Brand new elder
      pTrRelative->sibling = this;    // link elder to his brother
   }
}

/*STATIC------------------------------------------------------DelimNode::clean-+
| Clean recursively DelimNodes  (recursive routine)                            |
+-----------------------------------------------------------------------------*/
void DelimNode::clean(DelimNode * pTr)
{
   do {
      DelimNode * pTrCurrent = pTr;
      if (pTrCurrent->isParent) {
         clean(pTrCurrent->child);
      }
      if (pTrCurrent->isSibbled) {
         pTr = pTrCurrent->sibling;
      }else {
         pTr = 0;
      }
      delete pTrCurrent;
   }while (pTr);
}


/*----------------------------------------------operator<<(ostream, DelimNode)-+
| Display a node of Delim (debug only)                                         |
+-----------------------------------------------------------------------------*/
#if defined DEBUG_DELIM || defined __DEBUG_ALLOC__
ostream& operator<<(ostream& out, DelimNode const& tr)
{
   int const iRecursion = out.width();
   DelimNode const* pTr = &tr;

   out.width(iRecursion);
   out << "" << pTr->len << " secd node(s)";
   if (pTr->isBseq) out << " followed by a BSEQ";
   out << '/' << (int)pTr->ixdl << endl;
   while (pTr->isSibbled) {
      pTr = pTr->sibling;
      out.width(iRecursion);
      if (pTr->isBseq) {
         out << "" << "(BSEQ " << pTr->iLenBseq << ')';
      }else {
         out << "" << (char)pTr->ucPrp;
      }
      for (int i=0; i < pTr->len; ++i) out << (char)pTr->pUcContString[i];
      out << '/' << (int)pTr->ixdl << endl;
      if (pTr->isParent) {
         out.width(iRecursion + 3);
         out << *pTr->child;
      }
   }
   return out;
}
#endif /* DEBUG_DELIM || __DEBUG_ALLOC__ */

/*--------------------------------------------------DelimBuilder::DelimBuilder-+
|                                                                              |
+-----------------------------------------------------------------------------*/
DelimBuilder::DelimBuilder(
   DelimList const & dlmlst,
   Features const & ft,
   UnicodeComposer & erhArg,
   bool & isOk
) :
   erh(erhArg),
   chs(dlmlst.charset()),
   table(dlmlst, ft, isOk)
{}

/*---------------------------------------------------------DelimBuilder::build-+
|                                                                              |
+-----------------------------------------------------------------------------*/
VectorItem const * DelimBuilder::build(e_RecognitionMode rmode)
{
   DlmDscTable::Row rowMin(table, rmode);
   DlmDscTable::Row rowHigh(rowMin);  ++rowHigh;
   DlmDscTable::Row rowMax(rowMin);   rowMax.hiBound();
   DelimNode trHead;
   iCountVi = 0;
   iBufferLength = 1;               // at least one for ending 0

   //>>> NOT IMPLEMENTED
   /*
   | Warning: if SHORTREF SGMLREF was used,
   | then TAB *must* have been defined as a function character
   | It's now time to catch this error.
   */
   if (!process_sublist(0, rowMin, rowHigh, rowMax, &trHead)) {
      if (trHead.isParent) {
         DelimNode::clean(trHead.child);
      }
      return &VectorItem::Nil;
   }
   #if defined DEBUG_DELIM
      cerr
         << "For recognition mode " << (int)rmode << ", "
         << iCountVi << " nodes were built" << endl
         << *trHead.child
         << "------------------------------------------------"
         << endl;
   #endif /* DEBUG_DELIM */

   pUcBuffer = new UCS_2[iBufferLength];
   if (!pUcBuffer) {
      return &VectorItem::Nil;
   }
   pUcBuffer[0] = 0;
   pUcBufferRoom = pUcBuffer + 1;
   VectorItem * pViEntry = pViCurr = new VectorItem[iCountVi];
   if (!pViEntry) {
      delete [] pUcBuffer;
      return &VectorItem::Nil;
   }
   pViPrevBseq = 0;
   prim_node_visit(trHead.child);
   return pViEntry;
}

/*-----------------------------------------------DelimBuilder::process_sublist-+
| Process a sublist of delimiters, and build the tree.                         |
|                                                                              |
| On entry: xublist coordinates, parent node                                   |
| When return: the branch at parent node is updated                            |
|                                                                              |
| If iColMin, rowMin, rowHigh, rowHigh are the coordinates of a sublist:       |
|  - for (int row = rowMin; row < rowMax; ++row) {                             |
|        strncmp(list[row], list[rowMin], iColMin-1) is always 0 (equality)    |
|  - for (int row = rowMin; row < rowHigh; ++row) {                            |
|        strncmp(list[row], list[rowMin], iColMin) is always 0 (equality)      |
|                                                                              |
|                                       iColMin                                |
|                                       |                                      |
|                                       V                                      |
|                   rowMin --> abc...tuvw                                      |
|                              abc...tuvw                                      |
|                              abc...tuvw                                      |
|                   rowHigh -> abc...tuvx                                      |
|                              abc...tuvx                                      |
|                              abc...tuvy                                      |
|                   rowMax --> abh.......                                      |
|                                                                              |
+-----------------------------------------------------------------------------*/
bool DelimBuilder::process_sublist(
   unsigned int const iColMin,
   DlmDscTable::Row & rowMin,
   DlmDscTable::Row & rowHigh,
   DlmDscTable::Row const& rowMax,
   DelimNode *pTr
) {
   /*------------------------------------+
   | Create a new child node in the tree |
   +------------------------------------*/
   int iCountPrepChar = 0;
   pTr = new DelimNode(pTr, true);      // primary node
   if (!pTr) return false;
   ++iCountVi;
   pTr->ixdl = Delimiter::IX_NOT_FOUND; // default: no code

   /*---------------------------------------------------+
   | Determines all subclasses of [rowMin..rowMax][col] |
   +---------------------------------------------------*/
   UCS_2 const * pUcDelim = rowMin.value();
   for (;;) {
      if (!pUcDelim[iColMin]) {         // no cont string (first iteration only)
         pTr->ixdl = rowMin.code();
      }else {
         /*---------------------+
         |Get a new brother node|
         +---------------------*/
         pTr = new DelimNode(pTr, false); // get a new brother node
         if (!pTr) return false;
         ++iCountVi;

         unsigned int iCol = iColMin;
         DlmDscTable::Row row(rowMin);
         bool isShortref;
         UCS_2 ucCurr;
         /*----------------------------------------------------------------+
         | Get longest prefix in the [rowMin..rowHigh][iColMin] subclass   |
         |                                                                 |
         | When the loop exits:                                            |
         |    [rowMin..row][iCol] defines the first sub-subclass (if any)  |
         |    [row][iCol] is the first element of the co-subclass (if any) |
         |                                                                 |
         | Force a cleavage (break to avoid continuation string)           |
         | if a B sequence is found: break on the first B.                 |
         +----------------------------------------------------------------*/
         {
            UCS_2 ucToStopAt;

            if (rowMin.code() >= Delimiter::IX_SHORTREF) {
               isShortref = true;
               if (pUcDelim[iColMin] == Charset::ucBSEQ) {
                  ucToStopAt = 0;
                  pTr->isBseq = true;
                  pTr->sibling->child->isBseq = true;  // inform primary node
               }else {
                  ucToStopAt = Charset::ucBSEQ;
               }
            }else {
               isShortref = false;
               ucToStopAt = 0;
            }
            do {
               row = rowMin;
               ucCurr = pUcDelim[++iCol];
               if (ucCurr == 0) {            // delimiter found
                  if (!check_duplicate_delimiter(rowMin, rowHigh, iCol)) {
                     return false;
                  }
                  ++row;
                  break;
               }
               while (
                  (++row < rowHigh) &&
                  (ucCurr == row.value()[iCol]) && (
                     (ucCurr != Charset::ucBSEQ) || (isShortref) ||
                     (row.code() < Delimiter::IX_SHORTREF)
                  )
               )
                  ;
            }while (
               (ucCurr != ucToStopAt) && // cleavage forced by new BSEQ
               (row == rowHigh)          // 1st sub-class found
            );
            /*-----------------------------------------------+
            | Install the prefix found in the secondary node |
            +-----------------------------------------------*/
            if (pTr->isBseq) {
               ++iCountVi;                     // need an extra VectorItem
               unsigned int iColScan = iColMin;
               while (
                  (pUcDelim[++iColScan] == Charset::ucBSEQ) && (iColScan < iCol)
               )
                  ;
               pTr->iLenBseq = iColScan - iColMin;
               pTr->len = iCol - iColScan;
               pTr->pUcContString = pUcDelim + iColScan;
            }else {
               ++iCountPrepChar;
               pTr->ucPrp = pUcDelim[iColMin]; // set preponderant char
               pTr->len = iCol - iColMin - 1;
               pTr->pUcContString = pUcDelim + iColMin + 1;
            }
            if (pTr->len) {
               iBufferLength += pTr->len + 1;
            }
         }
         /*-------------------------------------------------+
         | If a sub-subclass exists, process it recursively |
         | Else: set up the delim code                      |
         +-------------------------------------------------*/
         if ((row < rowHigh) || ((isShortref) && (ucCurr == Charset::ucBSEQ))) {
            pTr->ixdl = Delimiter::IX_NOT_FOUND;
            if (!process_sublist(iCol, rowMin, row, rowHigh, pTr)) {
               return false;
            }
         }else {
            pTr->ixdl = rowMin.code();
         }
      }
      /*---------------------------------------------------------+
      | Check if we can loop for the next co-subclass at iColMin |
      | if so, set the boundaries in [rowMin .. row-hi]          |
      +---------------------------------------------------------*/
      if (rowHigh >= rowMax) break;
      rowMin = rowHigh;
      pUcDelim = rowMin.value();
      {
         UCS_2 ucCurr = pUcDelim[iColMin];
         while ((++rowHigh < rowMax) && (ucCurr == rowHigh.value()[iColMin]))
            ;
      }
   }
   /*-----------------------------------------------------+
   | Set up the Preponderant Characters length in Primary |
   +-----------------------------------------------------*/
   if (iCountPrepChar) {
      iBufferLength += iCountPrepChar + 1;
   }
   pTr->sibling->child->len = iCountPrepChar;
   return true;
}

/*-------------------------------------DelimBuilder::check_duplicate_delimiter-+
| Check if two delimiter or delimiter-in-context are differents.               |
| NOTE: If it is a delimiter and delimiter_in-context, we could pursue,        |
| since the latter would prevail.                                              |
+-----------------------------------------------------------------------------*/
bool DelimBuilder::check_duplicate_delimiter(
   DlmDscTable::Row const& rowMin,
   DlmDscTable::Row const& rowHigh,
   int const iCol
) const
{
   bool isOk = true;
   Delimiter::e_Ix const ixdl = rowMin.code();
   UCS_2 ucPrevious = 0;
   DlmDscTable::Row row(rowMin);

   while (++row < rowHigh) {
      UCS_2 ucCurr = row.value()[iCol];
      if (!ucCurr) {                   // delimiter is simply duplicated
//       erh << ECE__SEVERE << _SDCL__DUPDLM << rowMin.value() << endm;
         erh << ECE__SEVERE;
         erh << _SDCL__DUPDLM;
         erh << rowMin.value();
         erh << endm;
         isOk = false;
      }else {
         switch (ixdl) {
         case Delimiter::IX_CRO:
            if (!chs.isCref(ucCurr)) continue;
            break;
         case Delimiter::IX_STAGO:
         case Delimiter::IX_ETAGO:
         case Delimiter::IX_ERO:
         case Delimiter::IX_PERO:
         case Delimiter::IX_MDO:
            if (!chs.isNamestart(ucCurr)) continue;
            break;
         default:
            return isOk;               // no need to pursue
         }
         /*
         | (PGR: 03/24/97)
         | XML defines the '&#x' as being a delimiter,
         | conflicting with &# (that can be followed by a name.)
         | Just forgive it.
         */
         if (
            (ixdl == Delimiter::IX_CRO) && (row.code() == Delimiter::IX_HCRO)
         ) {
            continue;
         }
         if (ucPrevious != ucCurr) {   // avoid to tell it more than once
            ucPrevious = ucCurr;
            erh << _SDCL__DUPDIC       // Duplicate delimiter-in-context
               << ECE__ERROR           // Non severe (can recover)
               << rowMin.value()
               << row.value()
               << endm;
         }
      }
   }
   return isOk;
}

/*-----------------------------------------------DelimBuilder::prim_node_visit-+
| Process first node of a left branch of the tree, during postorder traversal. |
| On entry: primary node (first child)                                         |
| When return: the branch starting at this node has been traversed.            |
+-----------------------------------------------------------------------------*/
void DelimBuilder::prim_node_visit(DelimNode *pTr)
{
   pViCurr->f1.h.count = (char)pTr->len;
   pViCurr->f1.h.isBseq = (bool)pTr->isBseq;
   pViCurr->f2.ixdl = pTr->ixdl;
   if (pTr->len) {
      pViCurr->f3.str = pUcPredCharList = pUcBufferRoom;
      pUcBufferRoom += 1 + pTr->len;
      pUcBufferRoom[-1] = 0;
   }else {
      pViCurr->f3.str = pUcBuffer;          // Empty string
   }
   ++pViCurr;
   secd_node_visit(pTr->sibling);
   delete pTr;
}

/*-----------------------------------------------DelimBuilder::secd_node_visit-+
| Node visit, and Postorder traversal of the tree.                             |
| The first left node is visited by prim_node_visit (predominant char list).   |
| When return: the branch starting at this node has been traversed, and freed. |
+-----------------------------------------------------------------------------*/
void DelimBuilder::secd_node_visit(DelimNode *pTr)
{
   VectorItem **ppViNext = &pViCurr->f1.next; // keep a pointer to left pk index

   if (!pTr->isBseq) {
      *pUcPredCharList++ = pTr->ucPrp;     // update predominant char list
   }
   pViCurr->f2.ixdl = pTr->ixdl;           // set its delimiter code
   if (pTr->len) {
      pViCurr->f3.str = pUcBufferRoom;
      int iLenCont = pTr->len;
      UCS_2 const * pUc = pTr->pUcContString;
      while (iLenCont--) *pUcBufferRoom++ = *pUc++;
      *pUcBufferRoom++ = 0;
   }else {
      pViCurr->f3.str = pUcBuffer;         // Empty string
   }
   if (pTr->isBseq) {
      (++pViCurr)->f2.iLenBseq = pTr->iLenBseq;
      pViCurr->f1.previous = pViPrevBseq;
      if (
         (!pTr->len) &&                    // if there is no continuation
         (pTr->isParent) &&                // and the primary child node...
         (pTr->child->isBseq)              // has a brother BSEQ
      ) {
         pViPrevBseq = pViCurr - 1;
         pViCurr->f3.isRecurse = true;     // turn on recurse flag
      }else {
         pViCurr->f3.isRecurse = false;    // turn off recurse flag
         pViPrevBseq = 0;                  // reset pViPrevBseq
      }
   }
   ++pViCurr;
   if (pTr->isSibbled) {
      secd_node_visit(pTr->sibling);
   }
   if (pTr->isParent) {
      *ppViNext = pViCurr;
      prim_node_visit(pTr->child);
   }else {
      *ppViNext = 0;                       // leaf: not forward referenced
   }
   delete pTr;
}
/*============================================================================*/
