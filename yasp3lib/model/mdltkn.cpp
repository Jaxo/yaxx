/*
* $Id: mdltkn.cpp,v 1.8 2011-07-29 10:26:39 pgr Exp $
*
* Defines the nodes (tokens) of the ModelTree
*
* The nodes of the ModelTree are internally used by the ModelTree object.
* No need to make this public.
*/

/*--------------+
| Include files |
+--------------*/
#include <iostream>
#include "../../toolslib/tplistix.h"
#include "mdltkn.h"

/*------------------------------------------------------------------ModelToken-+
| Constructor                                                                  |
+-----------------------------------------------------------------------------*/
ModelToken::ModelToken(ModelToken *pTknRelative, int isRelativeParent)
{
   conn = NO_CONN;
   bGroup = 0;
   bSibbled = 0;
   bTykReqd = 0;
   bTykOmit = 0;
   bOmissible = 0;
   bRepeatable = 0;
   bBase = 0;
   if (isRelativeParent) {
      sibling = pTknRelative;           // parent thread
      if (pTknRelative) {
         pTknRelative->child = this;    // from parent link
         pTknRelative->bGroup = 1;      // qualify parent
      }
   }else {
      sibling = pTknRelative->sibling;  // parent thread
      pTknRelative->sibling = this;     // from sibling link
      pTknRelative->bSibbled = 1;       // qualify sibling
   }
}

/*------------------------------------------------------------------ModelToken-+
| Copy constructor (deep copy)                                                 |
+-----------------------------------------------------------------------------*/
ModelToken::ModelToken(
   bool & isOk,
   ModelToken const & tknSource,
   TpNameIxXtable const & ixtbl
)
{
   iSeq = tknSource.iSeq;
   conn = tknSource.conn;
   bOmissible = tknSource.bOmissible;
   bRepeatable = tknSource.bRepeatable;
   bSibbled = tknSource.bSibbled;
   bGroup = tknSource.bGroup;
   bTykReqd = tknSource.bTykReqd;
   bTykOmit = tknSource.bTykOmit;
   bBase = tknSource.bBase;
   if (bGroup) {
      child = new ModelToken(isOk, *tknSource.child, ixtbl);
      if ((!child) || (!isOk)) {
         if (!child) bGroup = 0; // avoid problems with ModelToken::cleanup
         isOk = false;
         return;
      }
      ModelToken const* pTknSource = tknSource.child;
      ModelToken * pTkn;
      for (pTkn=child; pTkn->bSibbled; pTkn=pTkn->sibling) {
         pTknSource = pTknSource->sibling;
         pTkn->sibling = new ModelToken(isOk, *pTknSource, ixtbl);
         if ((!pTkn->sibling) || (!isOk)) {
            isOk = false;
            return;
         }
      }
      pTkn->sibling = this;                              // thread
   }else {
      ixElm = ixtbl[tknSource.ixElm];
   }
}

/*------------------------------------------------------------------operator==-+
| true if is (deeply) equal                                                    |
+-----------------------------------------------------------------------------*/
bool ModelToken::operator==(ModelToken const& tknSource) const
{
   if (
      (iSeq == tknSource.iSeq) &&
      (conn == tknSource.conn) &&
      (bOmissible == tknSource.bOmissible) &&
      (bRepeatable == tknSource.bRepeatable) &&
      (bSibbled == tknSource.bSibbled) &&
      (bGroup == tknSource.bGroup) &&
      (bTykReqd == tknSource.bTykReqd) &&
      (bTykOmit == tknSource.bTykOmit)
   ) {
     if (bGroup) {
        ModelToken const* pTknSource = tknSource.child;
        ModelToken const* pTkn = child;
        while (*pTkn == *pTknSource) {
           if (!pTkn->bSibbled) { // so is pTknSource, or == would be false
              return true;
           }
           pTkn = pTkn->sibling;
           pTknSource = pTknSource->sibling;
        }
     }else {
        if (ixElm == tknSource.ixElm) {
           return true;
        }
     }
   }
   return false;
}

/*----------------------------------------------------------------setOccurence-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void ModelToken::setOccurence(Delimiter::e_Ix const ixdlm)
{
   switch (ixdlm) {
   case Delimiter::IX_OPT:
      bOmissible = 1;
      break;
   case Delimiter::IX_PLUS:
      bRepeatable = 1;
      break;
   default: /* case Delimiter::IX_REP */
      bOmissible = 1;
      bRepeatable = 1;
      break;
   }
}

/*----------------------------------------------------------------setConnector-+
|                                                                              |
+-----------------------------------------------------------------------------*/
bool ModelToken::setConnector(Delimiter::e_Ix const ixdlm)
{
   e_Connector connFound;
   switch (ixdlm) {
   case Delimiter::IX_SEQ:
      connFound = SEQ_CONN;
      break;
   case Delimiter::IX_OR:
      connFound = OR_CONN;
      break;
   default: /* case Delimiter::IX_AND: */
      connFound = AND_CONN;
      break;
   }
   if (sibling->conn == NO_CONN) {       // if dad is unconnected
      sibling->conn = connFound;         // me (first child) sets connector
   }else {
      if (sibling->conn != connFound) {  // if inconsistent
         return false;
      }
   }
   return true;
}

/*---------------------------------------------------------inheritOmissibility-+
| Inherit omissibility (4.159)                                                 |
+-----------------------------------------------------------------------------*/
void ModelToken::inheritOmissibility()
{
   if (((conn == OR_CONN) && (bTykOmit)) || (!bTykReqd)) {
      bOmissible = 1;
   }
}

/*-----------------------------------------------------------------finishToken-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void ModelToken::finishToken()
{
   if (bOmissible) {                    // occurence inheritancy
      sibling->bTykOmit = 1;
   }else {
      sibling->bTykReqd = 1;
   }
}

/*-----------------------------------------------------------------------merge-+
| Merge two nodes                                                              |
+-----------------------------------------------------------------------------*/
void ModelToken::merge(ModelToken *pTknFrom)
{
   ModelToken *pTknWatch = pTknFrom->sibling;    // Q: Who's watching TknFrom?
   if (pTknWatch) {                              // if NO one, TknFrom is base!
      if (pTknWatch->child == pTknFrom) {        // A: TknFrom's parent?
         pTknWatch->child = this;                //    TknTo is now his child
      }else {                                    // A: TknFrom's elder brother
         pTknWatch = pTknWatch->child;
         while (pTknWatch->sibling != pTknFrom) {
            pTknWatch = pTknWatch->sibling;
         }
         pTknWatch->sibling = this;              //    TknTo is now his brother
      }
   }
   sibling = pTknFrom->sibling;                  // inherits TknFrom's thread
   if (pTknFrom->bOmissible) {                   // If from was omissible
      bOmissible = 1;                            // so is To
   }
   if (pTknFrom->bRepeatable) {                  // If from was repeatable
      bRepeatable = 1;                           // so is To
   }
   delete pTknFrom;                              // destroy TknFrom
}

/*---------------------------------------------------------------------cleanup-+
| Clean recursively a node of ModelTokens  (recursive routine)                 |
+-----------------------------------------------------------------------------*/
void ModelToken::cleanup()
{
   if (bGroup) {
      ModelToken * pTknNext = child;
      do {
         ModelToken * pTkn = pTknNext;
         pTkn->cleanup();
         pTknNext = pTkn->bSibbled? pTkn->sibling : 0;
         delete pTkn;
      }while (pTknNext);
   }
}

/*---------------------------------------------------------------sequenceNodes-+
| Set sequence numbers within the node                                         |
+-----------------------------------------------------------------------------*/
void ModelToken::sequenceNodes(ModelToken *pTkn, unsigned int &iSeq)
{
   do {
      pTkn->iSeq = iSeq++;
      if (pTkn->isGroup()) {
         sequenceNodes(pTkn->child, iSeq);
      }
   }while (pTkn = pTkn->bSibbled? pTkn->sibling : 0, pTkn);
}

/*----------------------------------------------------------------------parent-+
|                                                                              |
+-----------------------------------------------------------------------------*/
ModelToken const* ModelToken::parent() const
{
    ModelToken const* pTkn = this;

    while (pTkn->bSibbled) {            // avoid recursivity for performance
       pTkn = pTkn->sibling;
    }
    if (bBase) return 0; else return pTkn->sibling;
}

/*--------------------------------------------------------------name_connector-+
| For debug purposes                                                           |
+-----------------------------------------------------------------------------*/
inline char ModelToken::name_connector() const
{
   switch(conn) {
   case SEQ_CONN:  return ',';
   case AND_CONN:  return '&';
   case OR_CONN:   return '|';
   default:        return '#';
   }
}

/*--------------------------------------------------------------name_occurence-+
| For debug purposes                                                           |
+-----------------------------------------------------------------------------*/
inline char ModelToken::name_occurence() const
{
   if (bOmissible) {
      if (bRepeatable) {
         return '*';
      }else {
         return '?';
      }
   }else {
      if (bRepeatable) {
         return '+';
      }else {
         return ' ';
      }
   }
}

/*---------------------------------------------------------ostream& operator<<-+
| Display a node of ModelTokens (debug only)                                   |
+-----------------------------------------------------------------------------*/
ostream& operator<<(ostream& out, ModelToken const& tkn)
{
#if !defined MWERKS_NEEDS_Q_FIX    // formatted io
   int const iRecursion = out.width();
   ModelToken const* pTkn = &tkn;
   do {
      out.width(iRecursion);
      if (pTkn->isGroup()) {
         out
            << "" << pTkn->name_connector()
            << '(' << pTkn->name_occurence() << ')'
            << std::endl
         ;
         out.width(iRecursion + 3);
         out << *pTkn->child;
      }else {
         if (pTkn->ixElm == -1) {
            out << "" << "#PCDATA";
         }else {
            out << pTkn->ixElm;
         }
         out << '[' << pTkn->name_occurence() << ']' << std::endl;
      }
   }while (pTkn = pTkn->bSibbled? pTkn->sibling : 0, pTkn);
#endif
   return out;
}

/*===========================================================================*/
