/*
* $Id: mdltree.h,v 1.2 2011-07-29 10:26:40 pgr Exp $
*
* Build the tree representing a content model
*/

#if !defined MDLTREE_HEADER && defined __cplusplus
#define MDLTREE_HEADER

/*--------------+
| Include Files |
+--------------*/
#include "../syntax/delim.h"
#include "mdltkn.h"
#include <iostream>

// class ostream;
class TpNameIxXtable;

/*-------------------------------------------------------------------ModelTree-+
|                                                                              |
| When:                                 status is:              pTknBase is:   |
| the element was not declared,         GRPO_EXPECTED           NULL           |
| the element has a declared content,   OK                      NULL           |
| the model content is OK               OK                      non null       |
| the model content is not yet ended    OK < and < ERROR_ALL    non null       |
| the model content is incorrect        >= ERROR_ALL            NULL           |
+-----------------------------------------------------------------------------*/
class ModelToken;                // (internal)

class ModelTree {
   friend class ModelLocator;
   friend std::ostream& operator<<(std::ostream& out, ModelTree const& tree);
public:
   /*-------------------+
   | Public Definitions |
   +-------------------*/
   enum e_Status {       // Current status of the ModelTree
      // Ended OK: the ModelTree has been sucessfully built
      OK,

      // Not yet ended: GRPO delimiter now expected
      GRPO_EXPECTED,

      // Not yet ended: Generic Identifier or GRPO now expected
      GI_OR_GRPO_EXPECTED,

      // Not yet ended: GRPC, or occurence indicator, or connector now expected
      GRPC_OR_OCC_OR_CONN_EXPECTED,

      // Not yet ended: GRPC or connector now expected
      GRPC_OR_CONN_EXPECTED,

      // Not yet ended: Occurence indicator or END now expected
      OCC_OR_END_EXPECTED,

      // Not yet ended: endOfModel now expected
      END_EXPECTED,

      // Errors can be tested by >= ERROR_ALL
      ERROR_ALL,

      // Ended in error: GRPO was expected
      ERROR_GRPO_EXPECTED = ERROR_ALL,

      // Ended in error: Generic Identifier or GRPO was expected
      ERROR_GI_OR_GRPO_EXPECTED,

      // Ended in error: GRPC, or occurence indicator, or connector was expected
      ERROR_GRPC_OR_OCC_OR_CONN_EXPECTED,

      // Ended in error: GRPC or connector was expected
      ERROR_GRPC_OR_CONN_EXPECTED,

      // Ended in error: Occurence indicator or END was expected
      ERROR_OCC_OR_END_EXPECTED,

      // Ended in error: endOfModel was expected
      ERROR_END_EXPECTED,

      // Ended in error: attempt to enter a lextoken after successful END
      ERROR_ENDED,

      // Ended in error: inconsistent connector -- as in (A | B | C & D)
      ERROR_INCONSISTENT_CONN,

      // Ended in error: memory shortage - tree cleaned up as much as possible
      ERROR_NO_MEMORY
   };
   enum e_DeclaredClass {
      INVALID,
      ANY,
      EMPTY,
      CDATA,
      RCDATA
   };
   /*---------------+
   | Public Methods |
   +---------------*/
   ModelTree();                                          // Null constructor
   ModelTree(ModelTree const& mdlSource);                // shallow copy
   ~ModelTree();                                         // destructor

   ModelTree& operator=(ModelTree const& source);        // shallow assign
   bool operator==(ModelTree const& mdlSource) const;
   bool operator!=(ModelTree const& mdlSource) const;

   ModelTree copy(TpNameIxXtable const & ixtbl) const;   // deep copy

   // Insert a parsed model using classical functions...
   bool insert(Delimiter::e_Ix ixdlm);
   bool insert(e_DeclaredClass dcls);
   bool insert(int ixElm);
   bool endOfModel();

   // or use insertion operators (more readable, but must check isOk()
   // use "<< endm;" to end the model
   ModelTree& operator<<(            // insertion manipulator
      ModelTree& (*f)(ModelTree&)
   );
   ModelTree& operator<<(Delimiter::e_Ix ixdlm);
   ModelTree& operator<<(e_DeclaredClass dcls);
   ModelTree& operator<<(int ixElm);

   // Harmless try to insert a new component (delimiter, handle, or end)
   // If it fails -- return != OK --, keep the model as it was
   e_Status tryInsert(Delimiter::e_Ix ixdlm);
   e_Status tryInsert(e_DeclaredClass dcls);
   e_Status tryInsert(int ixElm);
   e_Status tryEnd();

   bool isOk() const;             // still valid while building it
   operator void *() const;          // isOk?
   bool operator!() const;        // is not ok?
   e_Status status() const;          // get current status

   bool isMixedContent() const;   // model content has #PCDATA or true ANY
   bool isElementContent() const; // model content w/o #PCDATA or faked ANY
   bool isDefined() const;        // aka seenDcl
   bool isModel() const;
   bool isAny() const;
   bool isDcdataOrEmpty() const;  // RCDATA, CDATA or EMPTY
   bool isDcdata() const;         // RCDATA or CDATA
   bool isCdata() const;
   bool isRcdata() const;
   bool isEmpty() const;
   int inqKind() const;              // faster for multiple inquiries

   enum {
      ExpectingChild = 1,            // used only while building the Tree
      MixedContent   = 1 << 1,
      Defined        = 1 << 2,       // aka: isSeenDecl
      Any            = 1 << 3,
      Empty          = 1 << 4,
      Cdata          = 1 << 5,
      Rcdata         = 1 << 6
   };

private:
   /*-------------+
   | Private Data |
   +-------------*/
   ModelToken * pTknBase;
   unsigned int iNbrOfTokens;
   e_Status stStatus;
   int kind;

   /*----------------+
   | Private Methods |
   +----------------*/
   void cleanup();
   e_Status setOpenGroup();
   e_Status setConnector(Delimiter::e_Ix const ixdlm);
   void setCloseGroup();

public:                  // should be private -- but hp and nec compilers...
   ModelTree(int);       // for Undefined

public:
   static ModelTree const Nil;
   static ModelTree const Undefined;
};


/*-- INLINES --*/
inline bool ModelTree::operator!=(ModelTree const& source) const {
   if (operator==(source)) return false; else return true;
}
inline bool ModelTree::insert(Delimiter::e_Ix ixdlm) {
   e_Status stStatusRet = tryInsert(ixdlm);
   if (stStatusRet != OK) {
      stStatus = stStatusRet;
      cleanup();
      return false;
   }else {
      return true;
   }
}
inline bool ModelTree::insert(ModelTree::e_DeclaredClass dcls) {
   e_Status stStatusRet = tryInsert(dcls);
   if (stStatusRet != OK) {
      stStatus = stStatusRet;
      cleanup();
      return false;
   }else {
      return true;
   }
}
inline bool ModelTree::insert(int ixElm) {
   e_Status stStatusRet = tryInsert(ixElm);
   if (stStatusRet != OK) {
      stStatus = stStatusRet;
      cleanup();
      return false;
   }else {
      return true;
   }
}
inline bool ModelTree::endOfModel() {
   e_Status stStatusRet = tryEnd();
   if (stStatusRet != OK) {
      stStatus = stStatusRet;
      cleanup();
      return false;
   }else {
      return true;
   }
}
inline ModelTree& ModelTree::operator<<(Delimiter::e_Ix ixdlm) {
   insert(ixdlm);
   return *this;
}
inline ModelTree& ModelTree::operator<<(e_DeclaredClass dcls) {
   insert(dcls);
   return *this;
}
inline ModelTree& ModelTree::operator<<(int ixElm) {
   insert(ixElm);
   return *this;
}
inline ModelTree& endm(ModelTree& tree) {
   tree.endOfModel();
   return tree;
}
inline ModelTree& ModelTree::operator<<(ModelTree& (*f)(ModelTree&)) {
   return f(*this);
}
inline bool ModelTree::isOk() const {
   if (stStatus >= ModelTree::ERROR_ALL) return true; else return false;
}
inline ModelTree::operator void *() const {
   if (stStatus == ModelTree::OK) return (void *)this; else return 0;
}
inline bool ModelTree::operator!() const {
   if (stStatus != ModelTree::OK) return true; else return false;
}
inline ModelTree::e_Status ModelTree::status() const {
    return stStatus;
}
inline bool ModelTree::isMixedContent() const {
    if (kind & MixedContent) return true; else return false;
}
inline bool ModelTree::isElementContent() const {
    if (kind & (Cdata | Rcdata | MixedContent)) return false; else return true;
}
inline bool ModelTree::isDefined() const {
    if (kind & Defined) return true; else return false;
}
inline bool ModelTree::isModel() const {
    if (iNbrOfTokens) return true; else return false;
}
inline bool ModelTree::isAny() const {
    if (kind & Any) return true; else return false;
}
inline bool ModelTree::isDcdataOrEmpty() const {
    if (kind & (Cdata | Rcdata | Empty)) return true; else return false;
}
inline bool ModelTree::isDcdata() const {
    if (kind & (Cdata | Rcdata)) return true; else return false;
}
inline bool ModelTree::isCdata() const {
    if (kind & Cdata) return true; else return false;
}
inline bool ModelTree::isRcdata() const {
    if (kind & Rcdata) return true; else return false;
}
inline bool ModelTree::isEmpty() const {
    if (kind & Empty) return true; else return false;
}
inline int ModelTree::inqKind() const {
   return kind;
}

#endif /*MDLTREE_HEADER ======================================================*/
