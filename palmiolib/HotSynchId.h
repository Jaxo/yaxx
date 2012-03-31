/* $Id: HotSynchId.h,v 1.1 2002-08-21 11:09:21 pgr Exp $ */

/*---------------------------------------------------------------getIdAndName-+
|                                                                             |
+----------------------------------------------------------------------------*/
class HotSynchId {
public:
   static void getIdAndName(char pPalmName[41], UInt16 * pIdPalm);
   static void setIdAndName(char const * pPalmName, UInt16 const idPalm);
};

/*===========================================================================*/
