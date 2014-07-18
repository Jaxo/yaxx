/*********************************************************************
 *
 * Copyright (c) 2000 Matthew Moss. (http://www.splatbang.com/palm)
 * All Rights Reserved.
 *
 * Trap definitions copyright (c) 1994-1999 Palm Computing, Inc. or 
 * its subsidiaries. All rights reserved.
 *
 * DESCRIPTION:  Rez types useful for making Palm OS hacks. Add the
 *               following line to your .r file to use this file:
 *
 *               #include "HackRes.r";
 *
 * REVISION HISTORY:
 *     04/13/01  Added tver definition (Craig Patchett)
 *     03/27/01  Added comments and 3.5/4.0 trap names (Craig Patchett)
 *     09/03/99  Added trap name support (Matthew Moss)
 *     11/30/98  Initial release (Matthew Moss)
 *
 **********************************************************************/
 

#define  HackName    'tAIN'
#define  HackVersion 'tver'
#define  HackTrap    'TRAP'

#define  sysTrapBase  0xA000


/* HackName is used to create the resource for the hack name. Example:
 *
 *   resource 'tAIN' ( 3000 )
 *   {
 *     "My Hack"
 *   };
 */

type HackName 
{
  align word;
  cstring;
};

/* HackVersion is used to create the resource for the hack name. Example:
 *
 *   resource 'tver' ( 1 )
 *   {
 *     "1.0"
 *   };
 */

type HackVersion 
{
  align word;
  cstring;
};

/* HackTrap is used to define the traps you are patching. Example:
 *
 *   resource 'TRAP' ( 1000 ) 
 *   {
 *     sysTrapSysSleep
 *   };
 */

// The trap symbols used below are copied from CoreTraps.h file.
// Regular traps start here and go up by 1's

type HackTrap 
{
  integer sysTrapMemInit = sysTrapBase,
          sysTrapMemInitHeapTable,
          sysTrapMemStoreInit,
          sysTrapMemCardFormat,
          sysTrapMemCardInfo,
          sysTrapMemStoreInfo,
          sysTrapMemStoreSetInfo,
          sysTrapMemNumHeaps,
          sysTrapMemNumRAMHeaps,
          sysTrapMemHeapID,
          sysTrapMemHeapPtr,
          sysTrapMemHeapFreeBytes,
          sysTrapMemHeapSize,
          sysTrapMemHeapFlags,
          sysTrapMemHeapCompact,
          sysTrapMemHeapInit,
          sysTrapMemHeapFreeByOwnerID,
          sysTrapMemChunkNew,
          sysTrapMemChunkFree,
          sysTrapMemPtrNew,
          sysTrapMemPtrRecoverHandle,
          sysTrapMemPtrFlags,
          sysTrapMemPtrSize,
          sysTrapMemPtrOwner,
          sysTrapMemPtrHeapID,
          sysTrapMemPtrCardNo,
          sysTrapMemPtrToLocalID,
          sysTrapMemPtrSetOwner,
          sysTrapMemPtrResize,
          sysTrapMemPtrResetLock,
          sysTrapMemHandleNew,
          sysTrapMemHandleLockCount,
          sysTrapMemHandleToLocalID,
          sysTrapMemHandleLock,
          sysTrapMemHandleUnlock,
          sysTrapMemLocalIDToGlobal,
          sysTrapMemLocalIDKind,
          sysTrapMemLocalIDToPtr,
          sysTrapMemMove,
          sysTrapMemSet,
          sysTrapMemStoreSearch,
          sysTrapMemPtrDataStorage,
          sysTrapMemKernelInit,
          sysTrapMemHandleFree,
          sysTrapMemHandleFlags,
          sysTrapMemHandleSize,
          sysTrapMemHandleOwner,
          sysTrapMemHandleHeapID,
          sysTrapMemHandleDataStorage,
          sysTrapMemHandleCardNo,
          sysTrapMemHandleSetOwner,
          sysTrapMemHandleResize,
          sysTrapMemHandleResetLock,
          sysTrapMemPtrUnlock,
          sysTrapMemLocalIDToLockedPtr,
          sysTrapMemSetDebugMode,
          sysTrapMemHeapScramble,
          sysTrapMemHeapCheck,
          sysTrapMemNumCards,
          sysTrapMemDebugMode,
          sysTrapMemSemaphoreReserve,
          sysTrapMemSemaphoreRelease,
          sysTrapMemHeapDynamic,
          sysTrapMemNVParams,
      
      
          sysTrapDmInit,
          sysTrapDmCreateDatabase,
          sysTrapDmDeleteDatabase,
          sysTrapDmNumDatabases,
          sysTrapDmGetDatabase,
          sysTrapDmFindDatabase,
          sysTrapDmDatabaseInfo,
          sysTrapDmSetDatabaseInfo,
          sysTrapDmDatabaseSize,
          sysTrapDmOpenDatabase,
          sysTrapDmCloseDatabase,
          sysTrapDmNextOpenDatabase,
          sysTrapDmOpenDatabaseInfo,
          sysTrapDmResetRecordStates,
          sysTrapDmGetLastErr,
          sysTrapDmNumRecords,
          sysTrapDmRecordInfo,
          sysTrapDmSetRecordInfo,
          sysTrapDmAttachRecord,
          sysTrapDmDetachRecord,
          sysTrapDmMoveRecord,
          sysTrapDmNewRecord,
          sysTrapDmRemoveRecord,
          sysTrapDmDeleteRecord,
          sysTrapDmArchiveRecord,
          sysTrapDmNewHandle,
          sysTrapDmRemoveSecretRecords,
          sysTrapDmQueryRecord,
          sysTrapDmGetRecord,
          sysTrapDmResizeRecord,
          sysTrapDmReleaseRecord,
          sysTrapDmGetResource,
          sysTrapDmGet1Resource,
          sysTrapDmReleaseResource,
          sysTrapDmResizeResource,
          sysTrapDmNextOpenResDatabase,
          sysTrapDmFindResourceType,
          sysTrapDmFindResource,
          sysTrapDmSearchResource,
          sysTrapDmNumResources,
          sysTrapDmResourceInfo,
          sysTrapDmSetResourceInfo,
          sysTrapDmAttachResource,
          sysTrapDmDetachResource,
          sysTrapDmNewResource,
          sysTrapDmRemoveResource,
          sysTrapDmGetResourceIndex,
          sysTrapDmQuickSort,
          sysTrapDmQueryNextInCategory,
          sysTrapDmNumRecordsInCategory,
          sysTrapDmPositionInCategory,
          sysTrapDmSeekRecordInCategory,
          sysTrapDmMoveCategory,
          sysTrapDmOpenDatabaseByTypeCreator,
          sysTrapDmWrite,
          sysTrapDmStrCopy,
          sysTrapDmGetNextDatabaseByTypeCreator,
          sysTrapDmWriteCheck,
          sysTrapDmMoveOpenDBContext,
          sysTrapDmFindRecordByID,
          sysTrapDmGetAppInfoID,
          sysTrapDmFindSortPositionV10,
          sysTrapDmSet,
          sysTrapDmCreateDatabaseFromImage,
    
    
          sysTrapDbgSrcMessage,
          sysTrapDbgMessage,
          sysTrapDbgGetMessage,
          sysTrapDbgCommSettings,
      
          sysTrapErrDisplayFileLineMsg,
          sysTrapErrSetJump,
          sysTrapErrLongJump,
          sysTrapErrThrow,
          sysTrapErrExceptionList,
      
          sysTrapSysBroadcastActionCode,
          sysTrapSysUnimplemented,
          sysTrapSysColdBoot,
          sysTrapSysReset,
          sysTrapSysDoze,
          sysTrapSysAppLaunch,
          sysTrapSysAppStartup,
          sysTrapSysAppExit,
          sysTrapSysSetA5,
          sysTrapSysSetTrapAddress,
          sysTrapSysGetTrapAddress,
          sysTrapSysTranslateKernelErr,
          sysTrapSysSemaphoreCreate,
          sysTrapSysSemaphoreDelete,
          sysTrapSysSemaphoreWait,
          sysTrapSysSemaphoreSignal,
          sysTrapSysTimerCreate,
          sysTrapSysTimerWrite,
          sysTrapSysTaskCreate,
          sysTrapSysTaskDelete,
          sysTrapSysTaskTrigger,
          sysTrapSysTaskID,
          sysTrapSysTaskUserInfoPtr,
          sysTrapSysTaskDelay,
          sysTrapSysTaskSetTermProc,
          sysTrapSysUILaunch,
          sysTrapSysNewOwnerID,
          sysTrapSysSemaphoreSet,
          sysTrapSysDisableInts,
          sysTrapSysRestoreStatus,
          sysTrapSysUIAppSwitch,
          sysTrapSysCurAppInfoPV20,
          sysTrapSysHandleEvent,
          sysTrapSysInit,
          sysTrapSysQSort,
          sysTrapSysCurAppDatabase,
          sysTrapSysFatalAlert,
          sysTrapSysResSemaphoreCreate,
          sysTrapSysResSemaphoreDelete,
          sysTrapSysResSemaphoreReserve,
          sysTrapSysResSemaphoreRelease,
          sysTrapSysSleep,
          sysTrapSysKeyboardDialogV10,
          sysTrapSysAppLauncherDialog,
          sysTrapSysSetPerformance,
          sysTrapSysBatteryInfoV20,
          sysTrapSysLibInstall,
          sysTrapSysLibRemove,
          sysTrapSysLibTblEntry,
          sysTrapSysLibFind,
          sysTrapSysBatteryDialog,
          sysTrapSysCopyStringResource,
          sysTrapSysKernelInfo,
          sysTrapSysLaunchConsole,
          sysTrapSysTimerDelete,
          sysTrapSysSetAutoOffTime,
          sysTrapSysFormPointerArrayToStrings,
          sysTrapSysRandom,
          sysTrapSysTaskSwitching,
          sysTrapSysTimerRead,
    
    
          sysTrapStrCopy,
          sysTrapStrCat,
          sysTrapStrLen,
          sysTrapStrCompare,
          sysTrapStrIToA,
          sysTrapStrCaselessCompare,
          sysTrapStrIToH,
          sysTrapStrChr,
          sysTrapStrStr,
          sysTrapStrAToI,
          sysTrapStrToLower,
    
          sysTrapSerReceiveISP,
      
          sysTrapSlkOpen,
          sysTrapSlkClose,
          sysTrapSlkOpenSocket,
          sysTrapSlkCloseSocket,
          sysTrapSlkSocketRefNum,
          sysTrapSlkSocketSetTimeout,
          sysTrapSlkFlushSocket,
          sysTrapSlkSetSocketListener,
          sysTrapSlkSendPacket,
          sysTrapSlkReceivePacket,
          sysTrapSlkSysPktDefaultResponse,
          sysTrapSlkProcessRPC,
    
          sysTrapConPutS,
          sysTrapConGetS,
      
          sysTrapFplInit,         // Obsolete, here for compatibilty only!
          sysTrapFplFree,         // Obsolete, here for compatibilty only!
          sysTrapFplFToA,         // Obsolete, here for compatibilty only!
          sysTrapFplAToF,         // Obsolete, here for compatibilty only!
          sysTrapFplBase10Info,   // Obsolete, here for compatibilty only!
          sysTrapFplLongToFloat,  // Obsolete, here for compatibilty only!
          sysTrapFplFloatToLong,  // Obsolete, here for compatibilty only!
          sysTrapFplFloatToULong, // Obsolete, here for compatibilty only!
          sysTrapFplMul,          // Obsolete, here for compatibilty only!
          sysTrapFplAdd,          // Obsolete, here for compatibilty only!
          sysTrapFplSub,          // Obsolete, here for compatibilty only!
          sysTrapFplDiv,          // Obsolete, here for compatibilty only!
  
          sysTrapWinScreenInit,   // was sysTrapScrInit
          sysTrapScrCopyRectangle,
          sysTrapScrDrawChars,
          sysTrapScrLineRoutine,
          sysTrapScrRectangleRoutine,
          sysTrapScrScreenInfo,
          sysTrapScrDrawNotify,
          sysTrapScrSendUpdateArea,
          sysTrapScrCompressScanLine,
          sysTrapScrDeCompressScanLine,
      
          sysTrapTimGetSeconds,
          sysTrapTimSetSeconds,
          sysTrapTimGetTicks,
          sysTrapTimInit,
          sysTrapTimSetAlarm,
          sysTrapTimGetAlarm,
          sysTrapTimHandleInterrupt,
          sysTrapTimSecondsToDateTime,
          sysTrapTimDateTimeToSeconds,
          sysTrapTimAdjust,
          sysTrapTimSleep,
          sysTrapTimWake,
      
          sysTrapCategoryCreateListV10,
          sysTrapCategoryFreeListV10,
          sysTrapCategoryFind,
          sysTrapCategoryGetName,
          sysTrapCategoryEditV10,
          sysTrapCategorySelectV10,
          sysTrapCategoryGetNext,
          sysTrapCategorySetTriggerLabel,
          sysTrapCategoryTruncateName,
      
          sysTrapClipboardAddItem,
          sysTrapClipboardCheckIfItemExist,
          sysTrapClipboardGetItem,
      
          sysTrapCtlDrawControl,
          sysTrapCtlEraseControl,
          sysTrapCtlHideControl,
          sysTrapCtlShowControl,
          sysTrapCtlGetValue,
          sysTrapCtlSetValue,
          sysTrapCtlGetLabel,
          sysTrapCtlSetLabel,
          sysTrapCtlHandleEvent,
          sysTrapCtlHitControl,
          sysTrapCtlSetEnabled,
          sysTrapCtlSetUsable,
          sysTrapCtlEnabled,
    
          sysTrapEvtInitialize,
          sysTrapEvtAddEventToQueue,
          sysTrapEvtCopyEvent,
          sysTrapEvtGetEvent,
          sysTrapEvtGetPen,
          sysTrapEvtSysInit,
          sysTrapEvtGetSysEvent,
          sysTrapEvtProcessSoftKeyStroke,
          sysTrapEvtGetPenBtnList,
          sysTrapEvtSetPenQueuePtr,
          sysTrapEvtPenQueueSize,
          sysTrapEvtFlushPenQueue,
          sysTrapEvtEnqueuePenPoint,
          sysTrapEvtDequeuePenStrokeInfo,
          sysTrapEvtDequeuePenPoint,
          sysTrapEvtFlushNextPenStroke,
          sysTrapEvtSetKeyQueuePtr,
          sysTrapEvtKeyQueueSize,
          sysTrapEvtFlushKeyQueue,
          sysTrapEvtEnqueueKey,
          sysTrapEvtDequeueKeyEvent,
          sysTrapEvtWakeup,
          sysTrapEvtResetAutoOffTimer,
          sysTrapEvtKeyQueueEmpty,
          sysTrapEvtEnableGraffiti,
    
          sysTrapFldCopy,
          sysTrapFldCut,
          sysTrapFldDrawField,
          sysTrapFldEraseField,
          sysTrapFldFreeMemory,
          sysTrapFldGetBounds,
          sysTrapFldGetTextPtr,
          sysTrapFldGetSelection,
          sysTrapFldHandleEvent,
          sysTrapFldPaste,
          sysTrapFldRecalculateField,
          sysTrapFldSetBounds,
          sysTrapFldSetText,
          sysTrapFldGetFont,
          sysTrapFldSetFont,
          sysTrapFldSetSelection,
          sysTrapFldGrabFocus,
          sysTrapFldReleaseFocus,
          sysTrapFldGetInsPtPosition,
          sysTrapFldSetInsPtPosition,
          sysTrapFldSetScrollPosition,
          sysTrapFldGetScrollPosition,
          sysTrapFldGetTextHeight,
          sysTrapFldGetTextAllocatedSize,
          sysTrapFldGetTextLength,
          sysTrapFldScrollField,
          sysTrapFldScrollable,
          sysTrapFldGetVisibleLines,
          sysTrapFldGetAttributes,
          sysTrapFldSetAttributes,
          sysTrapFldSendChangeNotification,
          sysTrapFldCalcFieldHeight,
          sysTrapFldGetTextHandle,
          sysTrapFldCompactText,
          sysTrapFldDirty,
          sysTrapFldWordWrap,
          sysTrapFldSetTextAllocatedSize,
          sysTrapFldSetTextHandle,
          sysTrapFldSetTextPtr,
          sysTrapFldGetMaxChars,
          sysTrapFldSetMaxChars,
          sysTrapFldSetUsable,
          sysTrapFldInsert,
          sysTrapFldDelete,
          sysTrapFldUndo,
          sysTrapFldSetDirty,
          sysTrapFldSendHeightChangeNotification,
          sysTrapFldMakeFullyVisible,
      
          sysTrapFntGetFont,
          sysTrapFntSetFont,
          sysTrapFntGetFontPtr,
          sysTrapFntBaseLine,
          sysTrapFntCharHeight,
          sysTrapFntLineHeight,
          sysTrapFntAverageCharWidth,
          sysTrapFntCharWidth,
          sysTrapFntCharsWidth,
          sysTrapFntDescenderHeight,
          sysTrapFntCharsInWidth,
          sysTrapFntLineWidth,
      
          sysTrapFrmInitForm,
          sysTrapFrmDeleteForm,
          sysTrapFrmDrawForm,
          sysTrapFrmEraseForm,
          sysTrapFrmGetActiveForm,
          sysTrapFrmSetActiveForm,
          sysTrapFrmGetActiveFormID,
          sysTrapFrmGetUserModifiedState,
          sysTrapFrmSetNotUserModified,
          sysTrapFrmGetFocus,
          sysTrapFrmSetFocus,
          sysTrapFrmHandleEvent,
          sysTrapFrmGetFormBounds,
          sysTrapFrmGetWindowHandle,
          sysTrapFrmGetFormId,
          sysTrapFrmGetFormPtr,
          sysTrapFrmGetNumberOfObjects,
          sysTrapFrmGetObjectIndex,
          sysTrapFrmGetObjectId,
          sysTrapFrmGetObjectType,
          sysTrapFrmGetObjectPtr,
          sysTrapFrmHideObject,
          sysTrapFrmShowObject,
          sysTrapFrmGetObjectPosition,
          sysTrapFrmSetObjectPosition,
          sysTrapFrmGetControlValue,
          sysTrapFrmSetControlValue,
          sysTrapFrmGetControlGroupSelection,
          sysTrapFrmSetControlGroupSelection,
          sysTrapFrmCopyLabel,
          sysTrapFrmSetLabel,
          sysTrapFrmGetLabel,
          sysTrapFrmSetCategoryLabel,
          sysTrapFrmGetTitle,
          sysTrapFrmSetTitle,
          sysTrapFrmAlert,
          sysTrapFrmDoDialog,
          sysTrapFrmCustomAlert,
          sysTrapFrmHelp,
          sysTrapFrmUpdateScrollers,
          sysTrapFrmGetFirstForm,
          sysTrapFrmVisible,
          sysTrapFrmGetObjectBounds,
          sysTrapFrmCopyTitle,
          sysTrapFrmGotoForm,
          sysTrapFrmPopupForm,
          sysTrapFrmUpdateForm,
          sysTrapFrmReturnToForm,
          sysTrapFrmSetEventHandler,
          sysTrapFrmDispatchEvent,
          sysTrapFrmCloseAllForms,
          sysTrapFrmSaveAllForms,
          sysTrapFrmGetGadgetData,
          sysTrapFrmSetGadgetData,
          sysTrapFrmSetCategoryTrigger, 
    
          sysTrapUIInitialize,
          sysTrapUIReset,
    
          sysTrapInsPtInitialize,
          sysTrapInsPtSetLocation,
          sysTrapInsPtGetLocation,
          sysTrapInsPtEnable,
          sysTrapInsPtEnabled,
          sysTrapInsPtSetHeight,
          sysTrapInsPtGetHeight,
          sysTrapInsPtCheckBlink,
      
          sysTrapLstSetDrawFunction,
          sysTrapLstDrawList,
          sysTrapLstEraseList,
          sysTrapLstGetSelection,
          sysTrapLstGetSelectionText,
          sysTrapLstHandleEvent,
          sysTrapLstSetHeight,
          sysTrapLstSetSelection,
          sysTrapLstSetListChoices,
          sysTrapLstMakeItemVisible,
          sysTrapLstGetNumberOfItems,
          sysTrapLstPopupList,
          sysTrapLstSetPosition,
      
          sysTrapMenuInit,
          sysTrapMenuDispose,
          sysTrapMenuHandleEvent,
          sysTrapMenuDrawMenu,
          sysTrapMenuEraseStatus,
          sysTrapMenuGetActiveMenu,
          sysTrapMenuSetActiveMenu,
    
          sysTrapRctSetRectangle,
          sysTrapRctCopyRectangle,
          sysTrapRctInsetRectangle,
          sysTrapRctOffsetRectangle,
          sysTrapRctPtInRectangle,
          sysTrapRctGetIntersection,
    
          sysTrapTblDrawTable,
          sysTrapTblEraseTable,
          sysTrapTblHandleEvent,
          sysTrapTblGetItemBounds,
          sysTrapTblSelectItem,
          sysTrapTblGetItemInt,
          sysTrapTblSetItemInt,
          sysTrapTblSetItemStyle,
          sysTrapTblUnhighlightSelection,
          sysTrapTblSetRowUsable,
          sysTrapTblGetNumberOfRows,
          sysTrapTblSetCustomDrawProcedure,
          sysTrapTblSetRowSelectable,
          sysTrapTblRowSelectable,
          sysTrapTblSetLoadDataProcedure,
          sysTrapTblSetSaveDataProcedure,
          sysTrapTblGetBounds,
          sysTrapTblSetRowHeight,
          sysTrapTblGetColumnWidth,
          sysTrapTblGetRowID,
          sysTrapTblSetRowID,
          sysTrapTblMarkRowInvalid,
          sysTrapTblMarkTableInvalid,
          sysTrapTblGetSelection,
          sysTrapTblInsertRow,
          sysTrapTblRemoveRow,
          sysTrapTblRowInvalid,
          sysTrapTblRedrawTable,
          sysTrapTblRowUsable,
          sysTrapTblReleaseFocus,
          sysTrapTblEditing,
          sysTrapTblGetCurrentField,
          sysTrapTblSetColumnUsable,
          sysTrapTblGetRowHeight,
          sysTrapTblSetColumnWidth,
          sysTrapTblGrabFocus,
          sysTrapTblSetItemPtr,
          sysTrapTblFindRowID,
          sysTrapTblGetLastUsableRow,
          sysTrapTblGetColumnSpacing,
          sysTrapTblFindRowData,
          sysTrapTblGetRowData,
          sysTrapTblSetRowData,
          sysTrapTblSetColumnSpacing,
    
          sysTrapWinCreateWindow,
          sysTrapWinCreateOffscreenWindow,
          sysTrapWinDeleteWindow,
          sysTrapWinInitializeWindow,
          sysTrapWinAddWindow,
          sysTrapWinRemoveWindow,
          sysTrapWinSetActiveWindow,
          sysTrapWinSetDrawWindow,
          sysTrapWinGetDrawWindow,
          sysTrapWinGetActiveWindow,
          sysTrapWinGetDisplayWindow,
          sysTrapWinGetFirstWindow,
          sysTrapWinEnableWindow,
          sysTrapWinDisableWindow,
          sysTrapWinGetWindowFrameRect,
          sysTrapWinDrawWindowFrame,
          sysTrapWinEraseWindow,
          sysTrapWinSaveBits,
          sysTrapWinRestoreBits,
          sysTrapWinCopyRectangle,
          sysTrapWinScrollRectangle,
          sysTrapWinGetDisplayExtent,
          sysTrapWinGetWindowExtent,
          sysTrapWinDisplayToWindowPt,
          sysTrapWinWindowToDisplayPt,
          sysTrapWinGetClip,
          sysTrapWinSetClip,
          sysTrapWinResetClip,
          sysTrapWinClipRectangle,
          sysTrapWinDrawLine,
          sysTrapWinDrawGrayLine,
          sysTrapWinEraseLine,
          sysTrapWinInvertLine,
          sysTrapWinFillLine,
          sysTrapWinDrawRectangle,
          sysTrapWinEraseRectangle,
          sysTrapWinInvertRectangle,
          sysTrapWinDrawRectangleFrame,
          sysTrapWinDrawGrayRectangleFrame,
          sysTrapWinEraseRectangleFrame,
          sysTrapWinInvertRectangleFrame,
          sysTrapWinGetFramesRectangle,
          sysTrapWinDrawChars,
          sysTrapWinEraseChars,
          sysTrapWinInvertChars,
          sysTrapWinGetPattern,
          sysTrapWinSetPattern,
          sysTrapWinSetUnderlineMode,
          sysTrapWinDrawBitmap,
          sysTrapWinModal,
          sysTrapWinGetWindowBounds,
          sysTrapWinFillRectangle,
          sysTrapWinDrawInvertedChars,
      
          sysTrapPrefOpenPreferenceDBV10,
          sysTrapPrefGetPreferences,
          sysTrapPrefSetPreferences,
          sysTrapPrefGetAppPreferencesV10,
          sysTrapPrefSetAppPreferencesV10,
    
          sysTrapSndInit,
          sysTrapSndSetDefaultVolume,
          sysTrapSndGetDefaultVolume,
          sysTrapSndDoCmd,
          sysTrapSndPlaySystemSound,
      
          sysTrapAlmInit,
          sysTrapAlmCancelAll,
          sysTrapAlmAlarmCallback,
          sysTrapAlmSetAlarm,
          sysTrapAlmGetAlarm,
          sysTrapAlmDisplayAlarm,
          sysTrapAlmEnableNotification,
      
          sysTrapHwrGetRAMMapping,
          sysTrapHwrMemWritable,
          sysTrapHwrMemReadable,
          sysTrapHwrDoze,
          sysTrapHwrSleep,
          sysTrapHwrWake,
          sysTrapHwrSetSystemClock,
          sysTrapHwrSetCPUDutyCycle,
          sysTrapHwrDisplayInit,            // Before OS 3.5, this trap a.k.a. sysTrapHwrLCDInit
          sysTrapHwrDisplaySleep,           // Before OS 3.5, this trap a.k.a. sysTrapHwrLCDSleep,
          sysTrapHwrTimerInit,
          sysTrapHwrCursorV33,              // This trap obsoleted for OS 3.5 and later
          sysTrapHwrBatteryLevel,
          sysTrapHwrDelay,
          sysTrapHwrEnableDataWrites,
          sysTrapHwrDisableDataWrites,
          sysTrapHwrLCDBaseAddrV33,         // This trap obsoleted for OS 3.5 and later
          sysTrapHwrDisplayDrawBootScreen,  // Before OS 3.5, this trap a.k.a. sysTrapHwrLCDDrawBitmap
          sysTrapHwrTimerSleep,
          sysTrapHwrTimerWake,
          sysTrapHwrDisplayWake,            // Before OS 3.5, this trap a.k.a. sysTrapHwrLCDWake
          sysTrapHwrIRQ1Handler,
          sysTrapHwrIRQ2Handler,
          sysTrapHwrIRQ3Handler,
          sysTrapHwrIRQ4Handler,
          sysTrapHwrIRQ5Handler,
          sysTrapHwrIRQ6Handler,
          sysTrapHwrDockSignals,
          sysTrapHwrPluggedIn,
    
          sysTrapCrc16CalcBlock,
      
          sysTrapSelectDayV10,
          sysTrapSelectTime,
      
          sysTrapDayDrawDaySelector,
          sysTrapDayHandleEvent,
          sysTrapDayDrawDays,
          sysTrapDayOfWeek,
          sysTrapDaysInMonth,
          sysTrapDayOfMonth,
      
          sysTrapDateDaysToDate,
          sysTrapDateToDays,
          sysTrapDateAdjust,
          sysTrapDateSecondsToDate,
          sysTrapDateToAscii,
          sysTrapDateToDOWDMFormat,
          sysTrapTimeToAscii,
        
          sysTrapFind,
          sysTrapFindStrInStr,
          sysTrapFindSaveMatch,
          sysTrapFindGetLineBounds,
          sysTrapFindDrawHeader,
      
          sysTrapPenOpen,
          sysTrapPenClose,
          sysTrapPenGetRawPen,
          sysTrapPenCalibrate,
          sysTrapPenRawToScreen,
          sysTrapPenScreenToRaw,
          sysTrapPenResetCalibration,
          sysTrapPenSleep,
          sysTrapPenWake,
      
          sysTrapResLoadForm,
          sysTrapResLoadMenu,
      
          sysTrapFtrInit,
          sysTrapFtrUnregister,
          sysTrapFtrGet,
          sysTrapFtrSet,
          sysTrapFtrGetByIndex,
      
          sysTrapGrfInit,
          sysTrapGrfFree,
          sysTrapGrfGetState,
          sysTrapGrfSetState,
          sysTrapGrfFlushPoints,
          sysTrapGrfAddPoint,
          sysTrapGrfInitState,
          sysTrapGrfCleanState,
          sysTrapGrfMatch,
          sysTrapGrfGetMacro,
          sysTrapGrfFilterPoints,
          sysTrapGrfGetNumPoints,
          sysTrapGrfGetPoint,
          sysTrapGrfFindBranch,
          sysTrapGrfMatchGlyph,
          sysTrapGrfGetGlyphMapping,
          sysTrapGrfGetMacroName,
          sysTrapGrfDeleteMacro,
          sysTrapGrfAddMacro,
          sysTrapGrfGetAndExpandMacro,
          sysTrapGrfProcessStroke,
          sysTrapGrfFieldChange,
      
          sysTrapGetCharSortValue,
          sysTrapGetCharAttr,
          sysTrapGetCharCaselessValue,
      
          sysTrapPwdExists,
          sysTrapPwdVerify,
          sysTrapPwdSet,
          sysTrapPwdRemove,
      
          sysTrapGsiInitialize,
          sysTrapGsiSetLocation,
          sysTrapGsiEnable,
          sysTrapGsiEnabled,
          sysTrapGsiSetShiftState,
      
          sysTrapKeyInit,
          sysTrapKeyHandleInterrupt,
          sysTrapKeyCurrentState,
          sysTrapKeyResetDoubleTap,
          sysTrapKeyRates,
          sysTrapKeySleep,
          sysTrapKeyWake,
      
          sysTrapDlkControl,        // was sysTrapCmBroadcast
      
          sysTrapDlkStartServer,
          sysTrapDlkGetSyncInfo,
          sysTrapDlkSetLogEntry,
    
          sysTrapIntlDispatch,      // REUSED IN v3.1 (was sysTrapPsrInit in 1.0, removed in 2.0)
          sysTrapSysLibLoad,        // REUSED IN v2.0 (was sysTrapPsrClose)
          sysTrapSndPlaySmf,        // REUSED IN v3.0 (was sysTrapPsrGetCommand in 1.0, removed in 2.0)
          sysTrapSndCreateMidiList, // REUSED IN v3.0 (was sysTrapPsrSendReply in 1.0, removed in 2.0)
      
          sysTrapAbtShowAbout,
      
          sysTrapMdmDial,
          sysTrapMdmHangUp,
      
          sysTrapDmSearchRecord,
    
          sysTrapSysInsertionSort,
          sysTrapDmInsertionSort,
      
          sysTrapLstSetTopItem,
    
      
          // Palm OS 2.X traps      Palm Pilot and 2.0 Upgrade Card
          
          sysTrapSclSetScrollBar,
          sysTrapSclDrawScrollBar,
          sysTrapSclHandleEvent,
      
          sysTrapSysMailboxCreate,
          sysTrapSysMailboxDelete,
          sysTrapSysMailboxFlush,
          sysTrapSysMailboxSend,
          sysTrapSysMailboxWait,
      
          sysTrapSysTaskWait,
          sysTrapSysTaskWake,
          sysTrapSysTaskWaitClr,
          sysTrapSysTaskSuspend,
          sysTrapSysTaskResume,
      
          sysTrapCategoryCreateList,
          sysTrapCategoryFreeList,
          sysTrapCategoryEditV20,
          sysTrapCategorySelect,
      
          sysTrapDmDeleteCategory,
      
          sysTrapSysEvGroupCreate,
          sysTrapSysEvGroupSignal,
          sysTrapSysEvGroupRead,
          sysTrapSysEvGroupWait,
      
          sysTrapEvtEventAvail,
          sysTrapEvtSysEventAvail,
          sysTrapStrNCopy,
      
          sysTrapKeySetMask,
      
          sysTrapSelectDay,
      
          sysTrapPrefGetPreference,
          sysTrapPrefSetPreference,
          sysTrapPrefGetAppPreferences,
          sysTrapPrefSetAppPreferences,
      
          sysTrapFrmPointInTitle,
      
          sysTrapStrNCat,
      
          sysTrapMemCmp,
      
          sysTrapTblSetColumnEditIndicator,
    
          sysTrapFntWordWrap,
      
          sysTrapFldGetScrollValues,
      
          sysTrapSysCreateDataBaseList,
          sysTrapSysCreatePanelList,
      
          sysTrapDlkDispatchRequest,
      
          sysTrapStrPrintF,
          sysTrapStrVPrintF,
      
          sysTrapPrefOpenPreferenceDB,
    
          sysTrapSysGraffitiReferenceDialog,
      
          sysTrapSysKeyboardDialog,
      
          sysTrapFntWordWrapReverseNLines,
          sysTrapFntGetScrollValues,
      
          sysTrapTblSetRowStaticHeight,
          sysTrapTblHasScrollBar,
      
          sysTrapSclGetScrollBar,
      
          sysTrapFldGetNumberOfBlankLines,
      
          sysTrapSysTicksPerSecond,
          sysTrapHwrBacklightV33,     // This trap obsoleted for OS 3.5 and later
          sysTrapDmDatabaseProtect,
      
          sysTrapTblSetBounds,
      
          sysTrapStrNCompare,
          sysTrapStrNCaselessCompare,  
      
          sysTrapPhoneNumberLookup,
      
          sysTrapFrmSetMenu,
      
          sysTrapEncDigestMD5,
      
          sysTrapDmFindSortPosition,
      
          sysTrapSysBinarySearch,
          sysTrapSysErrString,
          sysTrapSysStringByIndex,
      
          sysTrapEvtAddUniqueEventToQueue,
      
          sysTrapStrLocalizeNumber,
          sysTrapStrDelocalizeNumber,
          sysTrapLocGetNumberSeparators,
      
          sysTrapMenuSetActiveMenuRscID,
      
          sysTrapLstScrollList,
      
          sysTrapCategoryInitialize,
      
          sysTrapEncDigestMD4,
          sysTrapEncDES,
      
          sysTrapLstGetVisibleItems,
      
          sysTrapWinSetWindowBounds,
      
          sysTrapCategorySetName,
      
          sysTrapFldSetInsertionPoint,
      
          sysTrapFrmSetObjectBounds,
      
          sysTrapWinSetColors,
      
          sysTrapFlpDispatch,
          sysTrapFlpEmDispatch,
      
      
          // Palm OS 3.0 traps        Palm III and 3.0 Upgrade Card
          
          sysTrapExgInit,
          sysTrapExgConnect,
          sysTrapExgPut,
          sysTrapExgGet,
          sysTrapExgAccept,
          sysTrapExgDisconnect,
          sysTrapExgSend,
          sysTrapExgReceive,
          sysTrapExgRegisterData,
          sysTrapExgNotifyReceive,
          sysTrapExgControl,
      
          sysTrapPrgStartDialogV31,   // Updated in v3.2
          sysTrapPrgStopDialog,
          sysTrapPrgUpdateDialog,
          sysTrapPrgHandleEvent,
      
          sysTrapImcReadFieldNoSemicolon,
          sysTrapImcReadFieldQuotablePrintable,
          sysTrapImcReadPropertyParameter,
          sysTrapImcSkipAllPropertyParameters,
          sysTrapImcReadWhiteSpace,
          sysTrapImcWriteQuotedPrintable,
          sysTrapImcWriteNoSemicolon,
          sysTrapImcStringIsAscii,
      
          sysTrapTblGetItemFont,
          sysTrapTblSetItemFont,
    
          sysTrapFontSelect,
          sysTrapFntDefineFont,
      
          sysTrapCategoryEdit,
      
          sysTrapSysGetOSVersionString,
          sysTrapSysBatteryInfo,
          sysTrapSysUIBusy,
      
          sysTrapWinValidateHandle,
          sysTrapFrmValidatePtr,
          sysTrapCtlValidatePointer,
          sysTrapWinMoveWindowAddr,
          sysTrapFrmAddSpaceForObject,
          sysTrapFrmNewForm,
          sysTrapCtlNewControl,
          sysTrapFldNewField,
          sysTrapLstNewList,
          sysTrapFrmNewLabel,
          sysTrapFrmNewBitmap,
          sysTrapFrmNewGadget,
      
          sysTrapFileOpen,
          sysTrapFileClose,
          sysTrapFileDelete,
          sysTrapFileReadLow,
          sysTrapFileWrite,
          sysTrapFileSeek,
          sysTrapFileTell,
          sysTrapFileTruncate,
          sysTrapFileControl,
      
          sysTrapFrmActiveState,
      
          sysTrapSysGetAppInfo,
          sysTrapSysGetStackInfo,
      
          sysTrapWinScreenMode,       // was sysTrapScrDisplayMode
          sysTrapHwrLCDGetDepthV33,   // This trap obsoleted for OS 3.5 and later
          sysTrapHwrGetROMToken,
      
          sysTrapDbgControl,
      
          sysTrapExgDBRead,
          sysTrapExgDBWrite,
      
          sysTrapHostControl,         // Renamed from sysTrapSysGremlins, functionality generalized
          sysTrapFrmRemoveObject,  
  
          sysTrapSysReserved1,        // "Reserved" trap in Palm OS 3.0 and later trap table
          sysTrapSysReserved2,        // "Reserved" trap in Palm OS 3.0 and later trap table
          sysTrapSysReserved3,        // "Reserved" trap in Palm OS 3.0 and later trap table
  
          sysTrapOEMDispatch,         // OEM trap in Palm OS 3.0 and later trap table (formerly sysTrapSysReserved4)

  
          // Palm OS 3.1 traps        Palm IIIx and Palm V
  
          sysTrapHwrLCDContrastV33,   // This trap obsoleted for OS 3.5 and later
          sysTrapSysLCDContrast,
          sysTrapUIContrastAdjust,    // Renamed from sysTrapContrastAdjust
          sysTrapHwrDockStatus,
  
          sysTrapFntWidthToOffset,
          sysTrapSelectOneTime,
          sysTrapWinDrawChar,
          sysTrapWinDrawTruncChars,

          sysTrapSysNotifyInit,       // Notification Manager traps
          sysTrapSysNotifyRegister,
          sysTrapSysNotifyUnregister,
          sysTrapSysNotifyBroadcast,
          sysTrapSysNotifyBroadcastDeferred,
          sysTrapSysNotifyDatabaseAdded,
          sysTrapSysNotifyDatabaseRemoved,

          sysTrapSysWantEvent,

          sysTrapFtrPtrNew,
          sysTrapFtrPtrFree,
          sysTrapFtrPtrResize,
  
          sysTrapSysReserved5,        // "Reserved" trap in Palm OS 3.1 and later trap table


          // Palm OS 3.2 & 3.3 traps  Palm VII (3.2) and Fall '99 Palm OS Flash Update (3.3)

          sysTrapHwrNVPrefSet,        // mapped to FlashParmsWrite
          sysTrapHwrNVPrefGet,        // mapped to FlashParmsRead
          sysTrapFlashInit,
          sysTrapFlashCompress,
          sysTrapFlashErase,
          sysTrapFlashProgram,
  
          sysTrapAlmTimeChange,
          sysTrapErrAlertCustom,
          sysTrapPrgStartDialog,      // New version of sysTrapPrgStartDialogV31

          sysTrapSerialDispatch,
          sysTrapHwrBattery,
          sysTrapDmGetDatabaseLockState,
  
          sysTrapCncGetProfileList,
          sysTrapCncGetProfileInfo,
          sysTrapCncAddProfile,
          sysTrapCncDeleteProfile,

          sysTrapSndPlaySmfResource,

          sysTrapMemPtrDataStorage,   // Never actually installed until now.
  
          sysTrapClipboardAppendItem,
  
          sysTrapWiCmdV32,            // Code moved to INetLib; trap obsolete
                      
          // TRAPS ABOVE THIS POINT CAN NOT CHANGE BECAUSE THEY HAVE
          // BEEN RELEASED TO CUSTOMERS IN SHIPPING ROMS AND SDKS.
          // (MOVE THIS COMMENT DOWN WHENEVER THE "NEXT" RELEASE OCCURS.)

  

          // WARNING!!  These are new traps for 3.5.  If this file is merged
          // with MAIN sources, new traps that are added for products that precede
          // 3.5 MUST insert their traps BEFORE this section.
  
          // HAL Display-layer new traps
          
          sysTrapHwrDisplayAttributes,
          sysTrapHwrDisplayDoze,
          sysTrapHwrDisplayPalette,

          // Screen driver new traps
          
          sysTrapBltFindIndexes,
          sysTrapBmpGetBits,          // was BltGetBitsAddr
          sysTrapBltCopyRectangle,
          sysTrapBltDrawChars,
          sysTrapBltLineRoutine,
          sysTrapBltRectangleRoutine,
  
          // ScrUtils new traps
          
          sysTrapScrCompress,
          sysTrapScrDecompress,
  
          // System Manager new traps
          
          sysTrapSysLCDBrightness,

          // WindowColor new traps
          
          sysTrapWinPaintChar,
          sysTrapWinPaintChars,
          sysTrapWinPaintBitmap,
          sysTrapWinGetPixel,
          sysTrapWinPaintPixel,
          sysTrapWinDrawPixel,
          sysTrapWinErasePixel,
          sysTrapWinInvertPixel,
          sysTrapWinPaintPixels,
          sysTrapWinPaintLines,
          sysTrapWinPaintLine,
          sysTrapWinPaintRectangle,
          sysTrapWinPaintRectangleFrame,
          sysTrapWinPaintPolygon,
          sysTrapWinDrawPolygon,
          sysTrapWinErasePolygon,
          sysTrapWinInvertPolygon,
          sysTrapWinFillPolygon,
          sysTrapWinPaintArc,
          sysTrapWinDrawArc,
          sysTrapWinEraseArc,
          sysTrapWinInvertArc,
          sysTrapWinFillArc,
          sysTrapWinPushDrawState,
          sysTrapWinPopDrawState,
          sysTrapWinSetDrawMode,
          sysTrapWinSetForeColor,
          sysTrapWinSetBackColor,
          sysTrapWinSetTextColor,
          sysTrapWinGetPatternType,
          sysTrapWinSetPatternType,
          sysTrapWinPalette,
          sysTrapWinRGBToIndex,
          sysTrapWinIndexToRGB,
          sysTrapWinScreenLock,
          sysTrapWinScreenUnlock,
          sysTrapWinGetBitmap,
  
          // UIColor new traps
          
          sysTrapUIColorInit,
          sysTrapUIColorGetTableEntryIndex,
          sysTrapUIColorGetTableEntryRGB,
          sysTrapUIColorSetTableEntry,
          sysTrapUIColorPushTable,
          sysTrapUIColorPopTable,

          // Misc cleanup and API additions
  
          sysTrapCtlNewGraphicControl,
  
          sysTrapTblGetItemPtr,

          sysTrapUIBrightnessAdjust,
          sysTrapUIPickColor,
    
          sysTrapEvtSetAutoOffTimer,

          // Misc int'l/overlay support.
          
          sysTrapTsmDispatch,
          sysTrapOmDispatch,
          sysTrapDmOpenDBNoOverlay,
          sysTrapDmOpenDBWithLocale,
          sysTrapResLoadConstant,
  
          // New boot-time SmallROM HAL additions
          
          sysTrapHwrPreDebugInit,
          sysTrapHwrResetNMI,
          sysTrapHwrResetPWM,

          sysTrapKeyBootKeys,

          sysTrapDbgSerDrvOpen,
          sysTrapDbgSerDrvClose,
          sysTrapDbgSerDrvControl,
          sysTrapDbgSerDrvStatus,
          sysTrapDbgSerDrvWriteChar,
          sysTrapDbgSerDrvReadChar,

          // New boot-time BigROM HAL additions
          
          sysTrapHwrPostDebugInit,
          sysTrapHwrIdentifyFeatures,
          sysTrapHwrModelSpecificInit,
          sysTrapHwrModelInitStage2,
          sysTrapHwrInterruptsInit,

          sysTrapHwrSoundOn,
          sysTrapHwrSoundOff,

          // Kernel clock tick routine
          
          sysTrapSysKernelClockTick,
  
          // MenuEraseMenu is exposed as of PalmOS 3.5, but there are
          // no public interfaces for it yet.  Perhaps in a later release.
          
          sysTrapMenuEraseMenu,
  
          sysTrapSelectTime,
  
          // Menu Command Bar traps
          
          sysTrapMenuCmdBarAddButton,
          sysTrapMenuCmdBarGetButtonData,
          sysTrapMenuCmdBarDisplay,

          // Silkscreen info
          
          sysTrapHwrGetSilkscreenID,
          sysTrapEvtGetSilkscreenAreaList,
  
          sysTrapSysFatalAlertInit,
          sysTrapDateTemplateToAscii,
  
          // New traps dealing with masking private records
          
          sysTrapSecVerifyPW,
          sysTrapSecSelectViewStatus,
          sysTrapTblSetColumnMasked,
          sysTrapTblSetRowMasked,
          sysTrapTblRowMasked,
  
          // New form trap for dialogs with text entry field
          
          sysTrapFrmCustomResponseAlert,
          sysTrapFrmNewGsi,
  
          // New dynamic menu functions
          
          sysTrapMenuShowItem,
          sysTrapMenuHideItem,
          sysTrapMenuAddItem,
  
          // New form traps for "smart gadgets"
          
          sysTrapFrmSetGadgetHandler,

          // More new control functions
          
          sysTrapCtlSetGraphics,
          sysTrapCtlGetSliderValues,
          sysTrapCtlSetSliderValues,
          sysTrapCtlNewSliderControl,
  
          // Bitmap manager functions
          
          sysTrapBmpCreate,
          sysTrapBmpDelete,  
          sysTrapBmpCompress,
          // sysTrapBmpGetBits defined in Screen driver traps
          sysTrapBmpGetColortable,
          sysTrapBmpSize,
          sysTrapBmpBitsSize,
          sysTrapBmpColortableSize,
          
          // Extra window namager 
          
          sysTrapWinCreateBitmapWindow,
          
          // Ask for a null event sooner (replaces a macro which Poser hated)
          
          sysTrapEvtSetNullEventTick,

          // Exchange manager call to allow apps to select destination categories
          
          sysTrapExgDoDialog,
  
          // This call will remove temporary UI like popup lists
          
          sysTrapSysUICleanup,
  
          // WARNING!! LEAVE THIS AT THE END AND ALWAYS ADD NEW TRAPS TO
          //  THE END OF THE TRAP TABLE BUT RIGHT BEFORE THIS TRAP!!!!!!!!!
          
          sysTrapLastTrapNumber;
};
