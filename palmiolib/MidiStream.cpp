/*
* $Id: MidiStream.cpp,v 1.1 2002-08-23 15:29:12 pgr Exp $
*
* Midi streams
*/

/*---------+
| Includes |
+---------*/
#include "MidiStream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------MidiStreamBuf::xsputn-+
|                                                                             |
+----------------------------------------------------------------------------*/
streamsize MidiStreamBuf::xsputn(char const * pchBuf, streamsize iLen) {
   SndSmfOptionsType smfOpt;       // SMF play options
   smfOpt.dwStartMilliSec = 0;
   smfOpt.dwEndMilliSec = sndSmfPlayAllMilliSec;
   smfOpt.amplitude = PrefGetPreference(prefAlarmSoundVolume);
   smfOpt.interruptible = true;
   smfOpt.reserved = 0;
   SndPlaySmf(0, sndSmfCmdPlay, (UInt8 *)pchBuf, &smfOpt, 0, 0, false);
   return iLen;
}

/*----------------------------------------------MidiSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * MidiSchemeHandler::Rep::getID() const {
   return "MIDI";
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
