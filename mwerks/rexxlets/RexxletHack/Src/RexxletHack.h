#ifndef __RexxletHack_H__
#define __RexxletHack_H__

#pragma warn_a5_access on

#include "Rexxlet.h"

#if __ide_target("NoHack Debug") || __ide_target("NoHack Release")

#define CREATORID 'RX50'

#else

#define CREATORID 'RX  '	// Registered Rexxlet Hack CRID
						// NOTE:  This must match the creator id
						// that's specified in the build options'
						// PalmRex Post Linker!!!  I don't think
						// we can get/set it programmatically
						// so that it appears in only one place.
#endif

#define TRAPID_PROCESSSOFTKEYSTROKE	1000  // CAREFUL!!!  
// see hack doc
// http://www.daggerware.com/hackapi.htm
// http://www.linkesoft.com/english/xmaster/developer.html

#define CurrentPrefsVersion 	1

#define PREFS_FLAGSIDv1			1	// preferenced id (flags & urls)
#define PREFS_URLSIDv1			2	

#define PREFS_FLAGSIDv2			3	// preferenced id (flags & urls)
#define PREFS_URLSIDv2			4	

#define FEATURE_ISREXXLETRUNNING		1
#define FEATURE_CACHEDPREFS			2

#define NUMBEROFSTROKESv1		8
#define MAXLENGTHURLv1			80+1

enum StrokeEnum {
   STROKEFROM_APPS  	= 1<<0,
   STROKEFROM_MENU  	= 1<<1,
   STROKEFROM_CALC  	= 1<<2,
   STROKEFROM_FIND  	= 1<<3,
   STROKETO_APPS  	= 1<<4,
   STROKETO_MENU  	= 1<<5,
   STROKETO_CALC  	= 1<<6,
   STROKETO_FIND  	= 1<<7,
   STROKETO_GRAF  	= 1<<8,
   STROKETO_SCRN  	= 1<<9,

   STROKE_NOOP     	= 0,
   STROKE_APPS2GRAF = STROKEFROM_APPS|STROKETO_GRAF,
   STROKE_APPS2SCRN = STROKEFROM_APPS|STROKETO_SCRN,
   STROKE_MENU2GRAF = STROKEFROM_MENU|STROKETO_GRAF,
   STROKE_MENU2SCRN = STROKEFROM_MENU|STROKETO_SCRN,
   STROKE_CALC2GRAF = STROKEFROM_CALC|STROKETO_GRAF,
   STROKE_CALC2SCRN = STROKEFROM_CALC|STROKETO_SCRN,
   STROKE_FIND2GRAF = STROKEFROM_FIND|STROKETO_GRAF,
   STROKE_FIND2SCRN = STROKEFROM_FIND|STROKETO_SCRN
};

typedef struct {
	enum StrokeEnum stroke;
	bool isEnabled;
	RexxletInFlags flags;
	UInt32 reserved1; // since prefs are saved in a database, 
	UInt32 reserved2; // this will help migration for future enhancements 
	UInt32 reserved3; // e.g., x,y and h/w of rexxlet console
} StrokeDescriptor;

typedef struct {
	UInt16 numberOfStrokes;
	StrokeDescriptor descriptors[NUMBEROFSTROKESv1];
} StrokeDescriptors;

typedef struct  {
	char * m_urls;
	int m_urlsLength;
	bool m_urlsMallocd;
	MemHandle * m_urls4Config;
	StrokeDescriptors m_descriptors;
} RexxletHackPrefs;   

extern "C" {

// Why are these here, you ask?  Good question.  
// It seems to be the pattern of development for hacks,
// because the code segments (the hack and the prefs) have to be separate and
// I don't think we can call across segments...so, code is duplicated.

RexxletHackPrefs * RexxletHackPrefs__ctor(bool useAppPrefs = true);
void RexxletHackPrefs__dtor(RexxletHackPrefs * self);
void RexxletHackPrefs__setDefaultPreferences(RexxletHackPrefs * self);
char * RexxletHackPrefs__urlz(char * s, int l, int n);

#if ((__ide_target("NoHack Debug") || __ide_target("NoHack Release")) && defined(PREFS_DEFINE_ONCE)) || (!__ide_target("NoHack Debug") && !__ide_target("NoHack Release"))

RexxletHackPrefs * RexxletHackPrefs__ctor(bool useAppPrefs) {
	RexxletHackPrefs * self = (RexxletHackPrefs *)MemPtrNew(sizeof(RexxletHackPrefs)); 
	if (!useAppPrefs) {
	   RexxletHackPrefs__setDefaultPreferences(self);
	   return self;
	}
	
   UInt16 prefsSize = 0;
	Int16 prefsVersion = PrefGetAppPreferences(
   	CREATORID, PREFS_FLAGSIDv1, 0, &prefsSize, true);
   if (prefsVersion == noPreferenceFound) {
	   RexxletHackPrefs__setDefaultPreferences(self);
	   return self;
	}

	//<<<JAL TODO:  if (prefsVersion != CurrentPrefsVersion) migrate...
	// See "Updating Preferences Upon a New Release" in Palm OS Companion.
   if (prefsSize != sizeof(StrokeDescriptors)) { SndPlaySystemSound(sndError); /* YIKES! JAL TODO */ }
   prefsVersion = PrefGetAppPreferences(
      CREATORID, PREFS_FLAGSIDv1, &self->m_descriptors, &prefsSize, true);	
   prefsSize = 0;   
   prefsVersion = PrefGetAppPreferences(
   	CREATORID, PREFS_URLSIDv1, 0, &prefsSize, true);
   if (prefsVersion == noPreferenceFound) { SndPlaySystemSound(sndError); /* YIKES! JAL TODO */ }
  	self->m_urls = (char *)MemPtrNew(prefsSize); 
   prefsVersion = PrefGetAppPreferences(
      CREATORID, PREFS_URLSIDv1, self->m_urls, &prefsSize, true);
   self->m_urlsLength = prefsSize;
   self->m_urlsMallocd = true;
  	self->m_urls4Config = 0;
   return self;
}

void RexxletHackPrefs__dtor(RexxletHackPrefs * self)
{
	if (self->m_urlsMallocd) { MemPtrFree(self->m_urls); }
	if (self->m_urls4Config) { // Assumes we're in the config form.
		for (int i = 0; i < self->m_descriptors.numberOfStrokes; ++i) {
			MemHandleFree(self->m_urls4Config[i]);
		}
		MemPtrFree(self->m_urls4Config);
	}
	MemPtrFree(self);
	return;
}

void RexxletHackPrefs__setDefaultPreferences(RexxletHackPrefs * self)
{
	StrokeDescriptors prefsDefaultV1;// = {  Causes a "hidden" global to be created.
//		NUMBEROFSTROKESv1,	
//		{
//	//	    stroke            enable  pause  c/p    in   out  err  arg x
//	/*1*/	{STROKE_APPS2GRAF, true , {true , true , IOC, IOC, IOC, A0, 0}, 0,0,0},
//	/*2*/	{STROKE_APPS2SCRN, true , {true , true , IOC, IOC, IOC, A0, 0}, 0,0,0},
//	/*3*/	{STROKE_MENU2GRAF, true , {true , false, IOC, IOC, IOC, A0, 0}, 0,0,0},
//	/*4*/	{STROKE_MENU2SCRN, false, {false, false, IOC, IOC, IOC, A0, 0}, 0,0,0},
//	/*5*/	{STROKE_CALC2GRAF, false, {false, false, IOC, IOC, IOC, A0, 0}, 0,0,0},
//	/*6*/	{STROKE_CALC2SCRN, false, {false, false, IOC, IOC, IOC, A0, 0}, 0,0,0},
//	/*7*/	{STROKE_FIND2GRAF, false, {false, false, IOC, IOC, IOC, A0, 0}, 0,0,0},
//	/*8*/	{STROKE_FIND2SCRN, false, {false, false, IOC, IOC, IOC, A0, 0}, 0,0,0}
//		}
//	};
   #define DEFAULT_URLS  					\
   /*1*/ 	"console:in\x00" 				\
   /*2*/	"?\x00" 						\
   /*3*/	"dm:///RexxletDB/Rexx/1\x00" 	\
   /*4*/	"dm:///RexxletDB/Rexx/2\x00" 	\
   /*5*/	"dm:///RexxletDB/Rexx/3\x00" 	\
   /*6*/	"dm:///RexxletDB/Rexx/4\x00" 	\
   /*7*/	"dm:///RexxletDB/Rexx/5\x00" 	\
   /*8*/	"dm:///RexxletDB/Rexx/6\x00" 	

   prefsDefaultV1.numberOfStrokes = NUMBEROFSTROKESv1;
   
   prefsDefaultV1.descriptors[0].stroke                     = STROKE_APPS2GRAF;
   prefsDefaultV1.descriptors[0].isEnabled                  = true;
   prefsDefaultV1.descriptors[0].flags.pauseWhenDone        = true;
   prefsDefaultV1.descriptors[0].flags.copyAndPaste         = true;
   prefsDefaultV1.descriptors[0].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[0].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[0].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[0].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_NONE;
   prefsDefaultV1.descriptors[0].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[0].reserved1                  = 0;
   prefsDefaultV1.descriptors[0].reserved2                  = 0;
   prefsDefaultV1.descriptors[0].reserved3                  = 0;

   prefsDefaultV1.descriptors[1].stroke                     = STROKE_APPS2SCRN;
   prefsDefaultV1.descriptors[1].isEnabled                  = true;
   prefsDefaultV1.descriptors[1].flags.pauseWhenDone        = true;
   prefsDefaultV1.descriptors[1].flags.copyAndPaste         = true;
   prefsDefaultV1.descriptors[1].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[1].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[1].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[1].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_NONE;
   prefsDefaultV1.descriptors[1].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[1].reserved1                  = 0;
   prefsDefaultV1.descriptors[1].reserved2                  = 0;
   prefsDefaultV1.descriptors[1].reserved3                  = 0;

   prefsDefaultV1.descriptors[2].stroke                     = STROKE_MENU2GRAF;
   prefsDefaultV1.descriptors[2].isEnabled                  = true;
   prefsDefaultV1.descriptors[2].flags.pauseWhenDone        = false;
   prefsDefaultV1.descriptors[2].flags.copyAndPaste         = true;
   prefsDefaultV1.descriptors[2].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[2].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[2].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[2].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_NONE;
   prefsDefaultV1.descriptors[2].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[2].reserved1                  = 0;
   prefsDefaultV1.descriptors[2].reserved2                  = 0;
   prefsDefaultV1.descriptors[2].reserved3                  = 0;

   prefsDefaultV1.descriptors[3].stroke                     = STROKE_MENU2SCRN;
   prefsDefaultV1.descriptors[3].isEnabled                  = true;
   prefsDefaultV1.descriptors[3].flags.pauseWhenDone        = true;
   prefsDefaultV1.descriptors[3].flags.copyAndPaste         = true;
   prefsDefaultV1.descriptors[3].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[3].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[3].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[3].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_NONE;
   prefsDefaultV1.descriptors[3].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[3].reserved1                  = 0;
   prefsDefaultV1.descriptors[3].reserved2                  = 0;
   prefsDefaultV1.descriptors[3].reserved3                  = 0;

   prefsDefaultV1.descriptors[4].stroke                     = STROKE_CALC2GRAF;
   prefsDefaultV1.descriptors[4].isEnabled                  = true;
   prefsDefaultV1.descriptors[4].flags.pauseWhenDone        = true;
   prefsDefaultV1.descriptors[4].flags.copyAndPaste         = true;
   prefsDefaultV1.descriptors[4].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[4].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[4].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[4].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_SELECTION;
   prefsDefaultV1.descriptors[4].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[4].reserved1                  = 0;
   prefsDefaultV1.descriptors[4].reserved2                  = 0;
   prefsDefaultV1.descriptors[4].reserved3                  = 0;

   prefsDefaultV1.descriptors[5].stroke                     = STROKE_CALC2SCRN;
   prefsDefaultV1.descriptors[5].isEnabled                  = true;
   prefsDefaultV1.descriptors[5].flags.pauseWhenDone        = true;
   prefsDefaultV1.descriptors[5].flags.copyAndPaste         = false;
   prefsDefaultV1.descriptors[5].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[5].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[5].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[5].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_NONE;
   prefsDefaultV1.descriptors[5].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[5].reserved1                  = 0;
   prefsDefaultV1.descriptors[5].reserved2                  = 0;
   prefsDefaultV1.descriptors[5].reserved3                  = 0;

   prefsDefaultV1.descriptors[6].stroke                     = STROKE_FIND2GRAF;
   prefsDefaultV1.descriptors[6].isEnabled                  = true;
   prefsDefaultV1.descriptors[6].flags.pauseWhenDone        = true;
   prefsDefaultV1.descriptors[6].flags.copyAndPaste         = false;
   prefsDefaultV1.descriptors[6].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[6].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[6].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[6].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_SELECTION;
   prefsDefaultV1.descriptors[6].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[6].reserved1                  = 0;
   prefsDefaultV1.descriptors[6].reserved2                  = 0;
   prefsDefaultV1.descriptors[6].reserved3                  = 0;

   prefsDefaultV1.descriptors[7].stroke                     = STROKE_FIND2SCRN;
   prefsDefaultV1.descriptors[7].isEnabled                  = true;
   prefsDefaultV1.descriptors[7].flags.pauseWhenDone        = false;
   prefsDefaultV1.descriptors[7].flags.copyAndPaste         = false;
   prefsDefaultV1.descriptors[7].flags.defaultInput         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[7].flags.defaultOutput        = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[7].flags.defaultError         = RexxAppLaunchRexxletParamType::IO_CONSOLE;
   prefsDefaultV1.descriptors[7].flags.args                 = RexxAppLaunchRexxletParamType::ARGS_NONE;
   prefsDefaultV1.descriptors[7].flags.reservedForFutureUse = 0;
   prefsDefaultV1.descriptors[7].reserved1                  = 0;
   prefsDefaultV1.descriptors[7].reserved2                  = 0;
   prefsDefaultV1.descriptors[7].reserved3                  = 0;

	MemMove(&self->m_descriptors, &prefsDefaultV1, sizeof(StrokeDescriptors));
   self->m_urls = DEFAULT_URLS;
   self->m_urlsLength = sizeof(DEFAULT_URLS);
   self->m_urlsMallocd = false;
  	self->m_urls4Config = 0;
	return;
}

char * RexxletHackPrefs__urlz(char * s, int l, int n)
{
	int i, j, c; 
    
	if (s == 0 || l == 0 || n == 0) { return 0; }
   
	c = 0;
	for (i = 0; i < l; ++i) {
		if (s[i] == '\0') { 
			if (++c == n) { return 0; }
			continue; 
		}
		for (j = i + 1; j < l; ++j) { 
			if (s[j] == '\0' || j + 1 == l) {  
				if (++c == n) { return s + i; } 
				i = j;
				break;
			}
		}
	}
	return 0;
}

#endif

inline UInt16 RexxletHackPrefs__numberOfStrokes(RexxletHackPrefs * self) { return self->m_descriptors.numberOfStrokes; }
inline enum StrokeEnum RexxletHackPrefs__stroke(RexxletHackPrefs * self, UInt16 i) { return self->m_descriptors.descriptors[i].stroke; }
inline bool RexxletHackPrefs__strokeIsEnabled(RexxletHackPrefs * self, UInt16 i) { return self->m_descriptors.descriptors[i].isEnabled; }
inline StrokeDescriptor * RexxletHackPrefs__strokeDescriptor(RexxletHackPrefs * self, UInt16 i) { return &self->m_descriptors.descriptors[i]; }
inline char * RexxletHackPrefs__url(RexxletHackPrefs * self, UInt16 i) { return RexxletHackPrefs__urlz(self->m_urls, self->m_urlsLength, i); }

}

#endif
