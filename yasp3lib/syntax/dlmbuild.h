/*
* $Id: dlmbuild.h,v 1.4 2011-07-29 10:26:40 pgr Exp $
*
* This component is for the private use only of the
* DelimList and DelimFinder classes.
* A DelimiterBuilder is a transaction to construct a DelimiterFinder
* from a DelimList.
*/

#if !defined DLMBUILD_HEADER && defined __cplusplus
#define DLMBUILD_HEADER

#include "../../toolslib/ucstring.h"
#include "../../toolslib/sort.h"
#include "delim.h"

// class ostream;
class DelimList;
class Features;
class Charset;
struct VectorItem;
class UnicodeComposer;

/*-------------------------------------------------------------------DelimNode-+
| DelimNode class: Nodes of the delimiter tree                                 |
+-----------------------------------------------------------------------------*/
class DelimNode {                     // Threaded tree, as usual...
   friend ostream& operator<<(ostream& out, DelimNode const& tr);
public:
   DelimNode *child;
   DelimNode *sibling;
   int len;                           // Prim: pcl length, secd: cont length
   union {                            // Secd:
      unsigned int iLenBseq;          //    if Bseq: minimum length
      UCS_2 ucPrp;                //    else Preponderant Character
   };
   UCS_2 const * pUcContString;   // Secd: continuation string

   Delimiter::e_Ix ixdl;
   bool isSibbled;                 // if false sibling is a thread
   bool isParent;                  // if false child is an hElm
   bool isBseq;                    // This prim node has a Bseq secd node
   /**/                               // or this is a Bseq secd node

   DelimNode();                       // NULL constructor for a Head
   DelimNode(DelimNode *pTrRelative, bool isChild);
   static void clean(DelimNode * pTr);
};

/*-----------------------------------------------------------------DlmDscTable-+
| DlmDscTable objects are used to retrieve the delimiter values                |
| of DelimList objects, filtered by the features and recognition modes.        |
| Row is a cursor class indexing each delimiter.                               |
+-----------------------------------------------------------------------------*/
class DlmDscTable {
public:
   class Item {
   public:
      Item();
      Item(
         UnicodeString const & ucsDelimArg,
         Delimiter::e_Ix ixdlArg,
         unsigned int iModeArg
      );
      int compare(Item const& itemAgainst);
      void exchange(Item & itemAgainst);

      UnicodeString const ucsDelim;
      Delimiter::e_Ix const ixdl;
      unsigned int const iMode;
   private:
      Item& operator=(Item const& source);       // no!
      Item(Item const& source);                  // no!
   };
   class Row {                                   // Row class is a cursor
   public:
      Row(                                       // regular constructor
         DlmDscTable const& table, e_RecognitionMode rmode
      );
      Row(Row const& from);                      // Copy constructor
      UnicodeString const & value() const;       // return a ucsDelim
      Delimiter::e_Ix code() const;              // get delim code
      Row& operator=(Row const& from);           // Assignment operator
      void hiBound();                            // set the highest boundary
      bool operator<(Row const& from) const;  // Less than operator
      bool operator<=(Row const& from) const; // Less or equal operator
      bool operator==(Row const& from) const; // Equality operator
      bool operator>=(Row const& from) const; // Greater or equal operator
      bool operator>(Row const& from) const;  // Greater than operator
      Row& operator++();                         // prefix increment
      Row& operator++(int);                      // postfix increment
   private:
      Item const* aDlmdsc;
      int const iCountDlmDsc;
      int ix;
      unsigned int bmode;

      bool isRecognized();
   };

   DlmDscTable(
      DelimList const & dlmlst,
      Features const & ft,
      bool & isOk
   );
   ~DlmDscTable();

private:
   friend class DlmDscTable::Row;
   Item *aDlmdsc;
   int iCountDlmDsc;

   void sort();                                  // sort function
};

// MAKE_SortFunction is not nestable for SGI5
MAKE_SortFunction(DlmDscTable__SortMe, DlmDscTable::Item);

/*-- INLINES --*/
inline void DlmDscTable::sort() {
   DlmDscTable__SortMe(aDlmdsc, iCountDlmDsc);
}
inline DlmDscTable::Item::Item() :
   ucsDelim(), ixdl(Delimiter::IX_NOT_FOUND), iMode(0)
{}
inline DlmDscTable::Item::Item(
   UnicodeString const & ucsDelimArg,
   Delimiter::e_Ix ixdlArg,
   unsigned int iModeArg
) :
   ucsDelim(ucsDelimArg),
   ixdl(ixdlArg),
   iMode(iModeArg)
{
}
inline DlmDscTable::Row::Row(
   DlmDscTable const& table, e_RecognitionMode rmode
) :
   aDlmdsc(table.aDlmdsc), iCountDlmDsc(table.iCountDlmDsc)
{
   bmode = 1 << (unsigned int)rmode;
   ix = 0;
}
inline DlmDscTable::Row::Row(DlmDscTable::Row const& from) :
   aDlmdsc(from.aDlmdsc), iCountDlmDsc(from.iCountDlmDsc)
{
   bmode = from.bmode;
   ix = from.ix;
}
inline UnicodeString const & DlmDscTable::Row::value() const {
   return aDlmdsc[ix].ucsDelim;
}
inline Delimiter::e_Ix DlmDscTable::Row::code() const {
   return aDlmdsc[ix].ixdl;
}
inline DlmDscTable::Row& DlmDscTable::Row::operator=(
   DlmDscTable::Row const& from
) {
   ix = from.ix;
   return *this;
}
inline void DlmDscTable::Row::hiBound() {
   ix = iCountDlmDsc;
}
inline bool DlmDscTable::Row::isRecognized()
{
   if ((ix == 0) || (ix >= iCountDlmDsc) || (bmode & aDlmdsc[ix].iMode)) {
      return true;
   }else {
      return false;
   }
}
inline DlmDscTable::Row& DlmDscTable::Row::operator++() {
   if (ix < iCountDlmDsc) while (++ix, !isRecognized())
      ;
   return *this;
}
inline bool DlmDscTable::Row::operator>(
   DlmDscTable::Row const& from
) const {
   if (ix > from.ix) return true; else return false;
}
inline bool DlmDscTable::Row::operator>=(
   DlmDscTable::Row const& from
) const {
   if (ix >= from.ix) return true; else return false;
}
inline bool DlmDscTable::Row::operator==(
   DlmDscTable::Row const& from
) const {
   if (ix == from.ix) return true; else return false;
}
inline bool DlmDscTable::Row::operator<=(
   DlmDscTable::Row const& from
) const {
   if (ix <= from.ix) return true; else return false;
}
inline bool DlmDscTable::Row::operator<(
   DlmDscTable::Row const& from
) const {
   if (ix < from.ix) return true; else return false;
}

/*-----------------------------------------------------------------------------+
| DelimBuilder class                                                           |
| Session to build the array of DV (and to destroy it)                         |
+-----------------------------------------------------------------------------*/
class DelimBuilder {
public:
   DelimBuilder(
      DelimList const & dlmlst,
      Features const & ft,
      UnicodeComposer & erhArg,
      bool & isOk
   );
   VectorItem const * build(e_RecognitionMode rmode);
private:
   UnicodeComposer & erh;
   Charset const & chs;
   DlmDscTable const table;
   int iCountVi;
   VectorItem * pViCurr;
   VectorItem * pViPrevBseq;

   int iBufferLength;
   UCS_2 * pUcPredCharList;
   UCS_2 * pUcBuffer;
   UCS_2 * pUcBufferRoom;

   bool process_sublist(
      unsigned int const iColMin,
      DlmDscTable::Row & rowMin,
      DlmDscTable::Row & rowHigh,
      DlmDscTable::Row const& rowMax,
      DelimNode *pTr
   );
   bool check_duplicate_delimiter(
      DlmDscTable::Row const& rowMin,
      DlmDscTable::Row const& rowHigh,
      int const iCol
   )const;
   void prim_node_visit(DelimNode *pTr);
   void secd_node_visit(DelimNode *pTr);
   DelimBuilder& operator=(DelimBuilder const& source);          // no!
   DelimBuilder(DelimBuilder const& source);                     // no!
};

#endif /* DLMBUILD_HEADER ====================================================*/
