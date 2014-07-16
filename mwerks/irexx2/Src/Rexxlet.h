#ifndef __Rexxlet_H__
#define __Rexxlet_H__

#include "RexxLaunchCodes.h"

typedef struct {
	UInt32
		pauseWhenDone:1,	
		copyAndPaste:1,
		defaultInput:3,		// stdio/cin     
		defaultOutput:3,	// stdoutcout       
		defaultError:3,		// stderr/cerr                
		args:3,				// how to get the args (PARSE ARG), if any
	    reservedForFutureUse:18;
} RexxletInFlags;

typedef struct {
	UInt32
	    appLauncher:1,      // was the app button pressed?
	    reservedForFutureUse:31;
} RexxletOutFlags;

struct RexxAppLaunchRexxletParamType { 
	enum {
		IO_NULL=0x00,		// bit bucket (shitcan it)
		IO_CONSOLE=0x01,	// smart console (use it only if it's used!)
		IO_FILESTREAM=0x02	// file stream, File Stream API, file://...
//		IO_DATABASE=0x03	// database, Database Manager API, dm://...
//		IO_VFS=0x04			// virtual file system, VFS API, vfs://...
//		IO_ECHAANGE=0x05	// exchange, Exchange Manager API, exg://...
//		IO_TELEPHONY=0x06	// telephony, Telephony API, tele://...
//		see "Communications" in the Palm OS Reference
	};
	enum {
		ARGS_NONE=0x00,		// no args
		ARGS_CLIPBOARD=0x01,// clipboard
		ARGS_POPUP=0x02,    // popup
		ARGS_SELECTION=0x03 // current selection
	};
	
	// IN
	char * script;  	// url that points to the script
	char * args;        // calling-app supplied args
	RexxletInFlags iflags;
	
	// OUT
	UInt32 rc;			// return from interpret (THIS IS NOT REXX'S RC?!)
	void * result;  	// REXX'S RESULT Rexx-allocated owner-0 memory returned to caller
	RexxletOutFlags oflags;
};

extern "C" {

FieldType * MyFrmGetActiveField();
bool handleProcessSoftKeyStroke(PointType * start, PointType * end);
Err myEvtProcessSoftKeyStroke(PointType * start, PointType * end);
Err Rexxlet(struct RexxAppLaunchRexxletParamType & r);

}

#endif