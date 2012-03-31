/*
* $Id: mdltree.cpp,v 1.3 2011-07-29 10:26:39 pgr Exp $
*
* Binary tree of element's content model
*
* These routines construct the ModelTree structure, which is a binary
* left threaded tree representing the content model of each element.
*/

/*--------------+
| Include Files |
+--------------*/
#include <iostream>
#include <string.h>
#include <new>
#include "../../toolslib/tplistix.h"
#include "mdltree.h"

ModelTree const ModelTree::Nil;
ModelTree const ModelTree::Undefined(0);  // element not explicitely defined

/*-------------------------------------------------------------------ModelTree-+
| Null Constructor - waiting for proper insertions.                            |
+-----------------------------------------------------------------------------*/
ModelTree::ModelTree()
{
   iNbrOfTokens = 0;
   pTknBase = 0;
   kind = ExpectingChild;
   stStatus = GRPO_EXPECTED;
}

/*-------------------------------------------------------------------ModelTree-+
| Constructor for a default - this kind accepts everything and is undeclared   |
| Note: since it is undeclared, we do mot accept mixed content.                |
+-----------------------------------------------------------------------------*/
ModelTree::ModelTree(int)
{
   iNbrOfTokens = 0;
   pTknBase = 0;
   kind = Any;       // but NO mixed content
   stStatus = OK;
}

/*-------------------------------------------------------------------ModelTree-+
| Copy Constructor (shallow copy)                                              |
| Will *not* work if the model is not OK                                       |
+-----------------------------------------------------------------------------*/
ModelTree::ModelTree(ModelTree const& mdlSource)
{
   if (mdlSource.stStatus == OK) {
      iNbrOfTokens = mdlSource.iNbrOfTokens;
      pTknBase = mdlSource.pTknBase;
      kind = mdlSource.kind;
      stStatus = OK;
      if (pTknBase) {               // update reference count
         pTknBase->sibling = (ModelToken *)(1 + (long)pTknBase->sibling);
      }
   }else {
      iNbrOfTokens = 0;
      pTknBase = 0;
      kind = (Any | MixedContent);
      stStatus = ERROR_NO_MEMORY;   // could use a more precize error
   }
}
/*-------------------------------------------------------------ModelTree::copy-+
| Deep copy - reset indices                                                    |
+-----------------------------------------------------------------------------*/
ModelTree ModelTree::copy(TpNameIxXtable const & ixtbl) const
{
   ModelTree target;
   target.iNbrOfTokens = iNbrOfTokens;
   target.kind = kind;
   target.stStatus = stStatus;
   bool isOk = true;
   if (pTknBase) {
      target.pTknBase = new ModelToken(isOk, *pTknBase, ixtbl);
      if (!target.pTknBase) {
         target.stStatus = ERROR_NO_MEMORY;
      }else if (!isOk) {
         target.cleanup();
         target.stStatus = ERROR_NO_MEMORY;
      }else {
         if (target.stStatus == OK) {
            target.pTknBase->sibling = (ModelToken *)1;  // set reference count;
         }
      }
   }
   return target;
}

/*--------------------------------------------------------ModelTree::operator=-+
| Assignement                                                                  |
+-----------------------------------------------------------------------------*/
ModelTree& ModelTree::operator=(ModelTree const& mdlSource)
{
   if (pTknBase != mdlSource.pTknBase) {
      this->~ModelTree();
      new(this) ModelTree(mdlSource);
   }
   return *this;
}

/*------------------------------------------------------------------~ModelTree-+
| Destructor                                                                   |
+-----------------------------------------------------------------------------*/
ModelTree::~ModelTree() {
   if (pTknBase && (stStatus == OK)) {   // update reference count
      pTknBase->sibling = (ModelToken *)((long)pTknBase->sibling - 1);
      if ((long)pTknBase->sibling > 0) return;
   }
   cleanup();
}
/*---------------------------------------------------------------------cleanup-+
| Clean up the tree (called by destructor)                                     |
+-----------------------------------------------------------------------------*/
void ModelTree::cleanup()
{
   if (pTknBase) {
      if (stStatus != OK) {           // if pTknBase is still not the base
         while (pTknBase->sibling) {  // climb to the base
            pTknBase = pTknBase->sibling;
         }
      }
      pTknBase->cleanup();
      delete pTknBase;
      pTknBase = 0;
   }
   kind = 0;
   iNbrOfTokens = 0;
}

/*------------------------------------------------------------------operator==-+
|                                                                              |
+-----------------------------------------------------------------------------*/
bool ModelTree::operator==(ModelTree const& mdlSource) const
{
   if ((stStatus == OK) && (mdlSource.stStatus == OK)) {
      if ((pTknBase) && (mdlSource.pTknBase)) {
         if (*pTknBase == *mdlSource.pTknBase) {
            return true;
         }
      }else {                   // one (at least) is NULL
         if ((pTknBase == mdlSource.pTknBase) && (kind == mdlSource.kind)) {
            return true;
         }
      }
   }
   return false;
}

/*-------------------------------------------------------------------tryInsert-+
| Insert a declared content                                                    |
+-----------------------------------------------------------------------------*/
ModelTree::e_Status ModelTree::tryInsert(e_DeclaredClass dcl)
{
   if (pTknBase) {
      return tryInsert(Delimiter::IX_NOT_FOUND); // to get the right status
   }
   switch (dcl) {
   case EMPTY:
      kind |= Empty;
      break;
   case CDATA:
      kind |= Cdata;
      break;
   case RCDATA:
      kind |= Rcdata;
      break;
   default: // case ANY:
      kind |= (Any | MixedContent);
      break;
   }
   stStatus = END_EXPECTED;
   return OK;
}


/*-------------------------------------------------------------------tryInsert-+
| Insert a delimiter found in the content model                                |
+-----------------------------------------------------------------------------*/
ModelTree::e_Status ModelTree::tryInsert(Delimiter::e_Ix ixdlm)
{
   e_Status stStatusRet = ERROR_NO_MEMORY;   // avoid uninitialized values!

   switch (stStatus) {
   case GRPO_EXPECTED:
      switch (ixdlm) {
      case Delimiter::IX_GRPO:
         stStatusRet = setOpenGroup();
         if (stStatusRet == OK) {
            stStatus = GI_OR_GRPO_EXPECTED;
         }
         break;
      default:
         stStatusRet = ERROR_GRPO_EXPECTED;
         break;
      }
      break;

   case GI_OR_GRPO_EXPECTED:
      switch (ixdlm) {
      case Delimiter::IX_GRPO:
         stStatusRet = setOpenGroup();
         if (stStatusRet == OK) {
            stStatus = GI_OR_GRPO_EXPECTED;
         }
         break;
      default:
         stStatusRet = ERROR_GI_OR_GRPO_EXPECTED;
         break;
      }
      break;

   case GRPC_OR_OCC_OR_CONN_EXPECTED:
      switch (ixdlm) {
      case Delimiter::IX_GRPC:
         setCloseGroup();
         if (pTknBase->sibling) {  // not at the base?
            stStatus = GRPC_OR_OCC_OR_CONN_EXPECTED;
         }else {
            stStatus = OCC_OR_END_EXPECTED;
         }
         stStatusRet = OK;
         break;
      case Delimiter::IX_SEQ:
      case Delimiter::IX_OR:
      case Delimiter::IX_AND:
         stStatusRet = setConnector(ixdlm);
         if (stStatusRet == OK) {
            stStatus = GI_OR_GRPO_EXPECTED;
         }
         break;
      case Delimiter::IX_OPT:
      case Delimiter::IX_PLUS:
      case Delimiter::IX_REP:
         pTknBase->setOccurence(ixdlm);
         stStatus = GRPC_OR_CONN_EXPECTED;
         stStatusRet = OK;
         break;
      default:
         stStatusRet = ERROR_GRPC_OR_OCC_OR_CONN_EXPECTED;
         break;
      }
      break;

   case GRPC_OR_CONN_EXPECTED:
      switch (ixdlm) {
      case Delimiter::IX_GRPC:
         setCloseGroup();
         if (pTknBase->sibling) {  // not at the base?
            stStatus = GRPC_OR_OCC_OR_CONN_EXPECTED;
         }else {
            stStatus = OCC_OR_END_EXPECTED;
         }
         stStatusRet = OK;
         break;
      case Delimiter::IX_SEQ:
      case Delimiter::IX_OR:
      case Delimiter::IX_AND:
         stStatusRet = setConnector(ixdlm);
         if (stStatusRet == OK) {
            stStatus = GI_OR_GRPO_EXPECTED;
         }
         break;
      default:
         stStatusRet = ERROR_GRPC_OR_CONN_EXPECTED;
         break;
      }
      break;

   case OCC_OR_END_EXPECTED:
      switch (ixdlm) {
      case Delimiter::IX_OPT:
      case Delimiter::IX_PLUS:
      case Delimiter::IX_REP:
         pTknBase->setOccurence(ixdlm);
         stStatus = END_EXPECTED;
         stStatusRet = OK;
         break;
      default:
         stStatusRet = ERROR_OCC_OR_END_EXPECTED;
         break;
      }
      break;

   case END_EXPECTED:
      stStatusRet = ERROR_END_EXPECTED;
      break;

   case OK:
      stStatusRet = ERROR_ENDED;
      break;

   default:
      stStatusRet = stStatus;
      break;
   }
   return stStatusRet;
}

/*-------------------------------------------------------------------tryInsert-+
| Insert a generic identifier found in the content model                       |
+-----------------------------------------------------------------------------*/
ModelTree::e_Status ModelTree::tryInsert(int ixElm)
{
   if (stStatus != GI_OR_GRPO_EXPECTED) {
      return tryInsert(Delimiter::IX_NOT_FOUND); // to get the right status
   }
   ModelToken * pTkn = new ModelToken(pTknBase, kind & ExpectingChild);
   if (!pTkn) {
      return ERROR_NO_MEMORY;
   }
   kind &= ~ExpectingChild;
   pTkn->setElement(ixElm);
   if (ixElm == -1) {
      pTkn->setOccurence(Delimiter::IX_REP);
      stStatus = GRPC_OR_CONN_EXPECTED;
      kind |= MixedContent;
   }else {
      stStatus = GRPC_OR_OCC_OR_CONN_EXPECTED;
   }
   pTknBase = pTkn;
   return OK;
}

/*----------------------------------------------------------------------tryEnd-+
| End of model found                                                           |
+-----------------------------------------------------------------------------*/
ModelTree::e_Status ModelTree::tryEnd()
{
   e_Status stStatusRet = ERROR_NO_MEMORY;   // avoid uninitialized values!

   switch (stStatus) {
   case END_EXPECTED:
      if (!pTknBase) {      // if declared content
         stStatus = OK;
         stStatusRet = OK;
         kind |= Defined;
         break;
      }
      /* else, fall thru */
   case OCC_OR_END_EXPECTED:
      ModelToken::sequenceNodes(pTknBase, iNbrOfTokens);
      pTknBase->setBase();
      pTknBase->sibling = (ModelToken *)1;   // set reference count
      stStatus = OK;
      stStatusRet = OK;
      kind |= Defined;
      break;
   case GRPO_EXPECTED:
      stStatusRet = ERROR_GRPO_EXPECTED;
      break;
   case GI_OR_GRPO_EXPECTED:
      stStatusRet = ERROR_GI_OR_GRPO_EXPECTED;
      break;
   case GRPC_OR_OCC_OR_CONN_EXPECTED:
      stStatusRet = ERROR_GRPC_OR_OCC_OR_CONN_EXPECTED;
      break;
   case GRPC_OR_CONN_EXPECTED:
      stStatusRet = ERROR_GRPC_OR_CONN_EXPECTED;
      break;
   case OK:
      stStatusRet = ERROR_ENDED;
      break;
   default:
      stStatusRet = stStatus;
      break;
   }
   return stStatusRet;
}

/*----------------------------------------------------------------setOpenGroup-+
|                                                                              |
+-----------------------------------------------------------------------------*/
ModelTree::e_Status ModelTree::setOpenGroup()
{
   ModelToken * pTkn = new ModelToken(pTknBase, kind & ExpectingChild);
   if (!pTkn) {
      return ERROR_NO_MEMORY;
   }
   kind |= ExpectingChild;
   pTknBase = pTkn;
   return OK;
}

/*----------------------------------------------------------------setConnector-+
|                                                                              |
+-----------------------------------------------------------------------------*/
ModelTree::e_Status ModelTree::setConnector(Delimiter::e_Ix const ixdlm)
{
   pTknBase->finishToken();
   if (!pTknBase->setConnector(ixdlm)) {
      return ERROR_INCONSISTENT_CONN;
   }
   return OK;
}

/*---------------------------------------------------------------setCloseGroup-+
| Always OK                                                                    |
+-----------------------------------------------------------------------------*/
void ModelTree::setCloseGroup()
{
   pTknBase->finishToken();
   pTknBase = pTknBase->sibling;            // back to the parent
   pTknBase->inheritOmissibility();
   if (pTknBase->isUnconnected_group()) {   // group with only one child
      ModelToken * pTknChild = pTknBase->child;
      pTknChild->merge(pTknBase);           // pTknBase deleted
      pTknBase = pTknChild;                 // get the child instead
   }
}

/*---------------------------------------------------------ostream& operator<<-+
| Display the ModelTree                                                        |
+-----------------------------------------------------------------------------*/
ostream& operator<<(ostream& out, ModelTree const& tree)
{
   if (tree.pTknBase) {
      out << *tree.pTknBase;
   }else {
      out << "";
      if (tree.kind & ModelTree::Any)    out << "ANY";    else
      if (tree.kind & ModelTree::Cdata)  out << "CDATA";  else
      if (tree.kind & ModelTree::Rcdata) out << "RCDATA"; else
      if (tree.kind & ModelTree::Empty)  out << "EMPTY";  else
      out << "???";
      if (!(tree.kind & ModelTree::Defined)) out << " - not defined";
   }
   return out;
}
/*============================================================================*/
