/* $Id: HotSynchId.cpp,v 1.1 2002-08-21 11:09:21 pgr Exp $ */

#include <string.h>
#include "HotSynchId.h"

/*---------------------------------------------------HotSynchId::getIdAndName-+
|                                                                             |
+----------------------------------------------------------------------------*/
void HotSynchId::getIdAndName(char pPalmName[41], UInt16 * pIdPalm)
{
   DmOpenRef dbRef = DmOpenDatabaseByTypeCreator(
      'pref', 'psys', dmModeReadOnly
   );

   if (dbRef && (DmFindResource(dbRef, 'psys', 4, 0) != -1)) {
      MemHandle hRes = DmGetResource('psys', 4);
      char * pRes = (char *)MemHandleLock(hRes);
      strcpy(pPalmName, pRes+42);
      *pIdPalm = (pRes[4] << 8) + pRes[5];
      MemHandleUnlock(hRes);
   }else {
      pPalmName[0] = '\0';
      *pIdPalm = 0;
   }
   DmCloseDatabase(dbRef);
}

/*---------------------------------------------------HotSynchId::setIdAndName-+
|                                                                             |
+----------------------------------------------------------------------------*/
void HotSynchId::setIdAndName(char const * pPalmName, UInt16 const idPalm)
{
   DmOpenRef dbRef = DmOpenDatabaseByTypeCreator(
      'pref', 'psys', dmModeReadWrite
   );
   int lenPalmName = pPalmName? strlen(pPalmName) : 0;

   if (dbRef && lenPalmName && (lenPalmName <= 40)) {
      char * pRes;
      MemHandle hRes;
      int const bufSize = 42+lenPalmName+1+1;
      char * pBuffer = (char *)MemPtrNew(bufSize);
      if (DmFindResource(dbRef, 'psys', 4, 0) != -1) {
         hRes = DmGetResource('psys', 4);
         memcpy(pBuffer, MemHandleLock(hRes), 42);
         MemHandleUnlock(hRes);
         hRes = DmResizeResource(hRes, bufSize);
         pBuffer[bufSize-1] = '\0';
      }else {      // resource not found
         hRes = DmNewResource(dbRef, 'psys', 4, bufSize);
         memset(pBuffer, 0x00, bufSize);
      }
      pRes = (char *)MemHandleLock(hRes);
      pBuffer[4] = idPalm >> 8;
      pBuffer[5] = idPalm;
      pBuffer[38] = (char)(lenPalmName+1);
      strcpy(pBuffer+42, pPalmName);
      DmWrite(pRes, 0, pBuffer, bufSize);
      MemPtrFree(pBuffer);
      MemHandleUnlock(hRes);
      DmReleaseResource(hRes);
   }
   DmCloseDatabase(dbRef);
}

/*===========================================================================*/
