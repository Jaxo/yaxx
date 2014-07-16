#ifndef __Rexxapp_H__
#define __Rexxapp_H__

#include "RexxLaunchCodes.h"

typedef struct {
	UInt32
		defaultInput:3,		  
		defaultOutput:3,    
		defaultError:3,		               
		args:3,				
	   reservedForFutureUse:20;
} RexxappInFlags;

struct RexxAppLaunchRexxappParamType { 
	enum {
		IO_NULL=0x00,		
		IO_CONSOLE=0x01,	
		IO_FILESTREAM=0x02	
	};
	enum {
		ARGS_NONE=0x00,	 
		ARGS_CLIPBOARD=0x01,
		ARGS_POPUP=0x02,  
		ARGS_SELECTION=0x03 
	};
	
	char * script;  	
	char * args;    
	RexxappInFlags iflags;
};

#endif