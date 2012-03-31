/*
* $Id: mdltkn.h,v 1.2 2011-07-29 10:26:39 pgr Exp $
*
*  Nodes of the Model Tree (internal class)
*/

#if !defined MDLTKN_HEADER && defined __cplusplus
#define MDLTKN_HEADER

/*--------------+
| Include files |
+--------------*/
#include <assert.h>
#include "../syntax/delim.h"
#include <iostream>

// class ostream;
class TpNameIxXtable;

/*------------------------------------------------------------------ModelToken-+
|                                                                              |
+-----------------------------------------------------------------------------*/
class ModelToken
{
   friend std::ostream& operator<<(std::ostream& out, ModelToken const& tkn);
public:
   /*------------+
   | Public Data |
   +------------*/
   ModelToken * sibling;   // sibling if Sibbled true.  Else thread.
   ModelToken * child;     // isGroup? (ModelToken *)
   int ixElm;              // !isGroup? Element index

   /*---------------+
   | Public Methods |
   +---------------*/
   ModelToken(ModelToken *pTknRelative, int isRelativeParent);
   ModelToken(             // deep copy
      bool &isOk,
      ModelToken const& tknSource,
      TpNameIxXtable const & ixtbl
   );
   bool operator==(ModelToken const& tknSource) const;
   bool operator!=(ModelToken const& tknSource) const;
   void setElement(int ixElmArg);
   void setBase();
   void setOccurence(Delimiter::e_Ix const ixdlm);
   bool setConnector(Delimiter::e_Ix const ixdlm);
   void inheritOmissibility();
   void finishToken();
   void merge(ModelToken *pTknFrom);
   void cleanup();
   static void sequenceNodes(ModelToken *pTkn, unsigned int &iSeq);

   ModelToken const * parent() const;
   unsigned int seqNo() const;
   bool isElement() const;
   bool isGroup() const;
   bool isOrGroup() const;
   bool isAndGroup() const;
   bool isSeqGroup() const;
   bool isUnconnected_group() const;
   bool isOmissible() const;
   bool isRepeatable() const;
   bool isYounger() const;

private:
   /*---------------------+
   | Private Declarations |
   +---------------------*/
   enum e_Connector {
      NO_CONN = 0,
      AND_CONN,
      OR_CONN,
      SEQ_CONN
   };

   /*-------------+
   | Private Data |
   +-------------*/
   unsigned short iSeq;            // sequence number
   e_Connector conn;               // connector

   unsigned int bOmissible  :1;    // REP or OPT
   unsigned int bRepeatable :1;    // PLUS or REP
   unsigned int bSibbled    :1;    // if false sibling is a thread
   unsigned int bGroup      :1;    // if false child is an ixElm
   unsigned int bTykReqd    :1;    // if one child is required
   unsigned int bTykOmit    :1;    // if one child is omittable
   unsigned int bBase       :1;    // this is the base

   /*----------------+
   | Private Methods |
   +----------------*/
   char name_connector() const;
   char name_occurence() const;
};

/*--- INLINES ---*/
inline bool ModelToken::operator!=(ModelToken const& source) const {
   return operator==(source)? false : true;
}
inline unsigned int ModelToken::seqNo() const {
   return iSeq;
}
inline bool ModelToken::isElement() const {
   return (bGroup)? false : true;
}
inline bool ModelToken::isGroup() const {
   return (bGroup)? true : false;
}
inline bool ModelToken::isOrGroup() const {
   return (conn == OR_CONN)? true : false;
}
inline bool ModelToken::isAndGroup() const {
   return (conn == AND_CONN)? true : false;
}
inline bool ModelToken::isSeqGroup() const {
   return (conn == SEQ_CONN)? true : false;
}
inline bool ModelToken::isUnconnected_group() const {
   return (conn == NO_CONN)? true : false;
}
inline bool ModelToken::isOmissible() const {
   return bOmissible ? true : false;
}
inline bool ModelToken::isRepeatable() const {
   return bRepeatable ? true : false;
}
inline bool ModelToken::isYounger() const {
   return bSibbled? false : true;
}
inline void ModelToken::setElement(int ixElmArg) {
   ixElm = ixElmArg;
}
inline void ModelToken::setBase() {
   bBase = 1;
}
#endif /* MDLTKN_HEADER ======================================================*/
