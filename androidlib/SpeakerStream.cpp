/*
* $Id: SpeakerStream.cpp,v 1.3 2011-08-28 17:44:38 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/23/2011
*/

/*---------+
| Includes |
+---------*/
#include <string.h>
#include <alloca.h>
#include "SpeakerStream.h"
#include "../toolslib/ucstring.h"
#include "../toolslib/Encoder.h"

extern JNIEnv * g_jniEnv;

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------SpeakerStreamBuf::xsputn-+
|                                                                             |
+----------------------------------------------------------------------------*/
streamsize SpeakerStreamBuf::xsputn(char const * buf, streamsize length) {
   m_handlerRep->say(
      UnicodeString(buf, Encoder(EncodingModule::UTF_8), length),
      m_locale
   );
   return length;
}

/*---------------------------------------------SpeakerSchemeHandler::Rep::Rep-+
|                                                                             |
+----------------------------------------------------------------------------*/
SpeakerSchemeHandler::Rep::Rep(JNIEnv * env, jobject speaker) {
   jclass clazz = (speaker)? env->GetObjectClass(speaker) : 0;
   if (!clazz) {
      m_sayMethod = 0;
      m_speaker = 0;
   }else {
      m_sayMethod = env->GetMethodID(
         clazz, "say", "(Ljava/lang/String;Ljava/lang/String;)V"
      );
      if (!m_sayMethod) {
         m_speaker = 0;
      }else {
//       m_speaker = speaker;
         m_speaker = env->NewGlobalRef(speaker);
      }
   }
}

/*--------------------------------------------SpeakerSchemeHandler::Rep::~Rep-+
|                                                                             |
+----------------------------------------------------------------------------*/
SpeakerSchemeHandler::Rep::~Rep() {
   g_jniEnv->DeleteGlobalRef(m_speaker);
}

/*--------------------------------------SpeakerSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * SpeakerSchemeHandler::Rep::makeStream(
   URI const & uri, ios__openmode
) {
   /*
   | extract the value of `lang' form the uri:
   | speaker:lang=fr-FR
   */
   char const * path = uri.getPath();
   char * options;
   strcpy((options=(char *)alloca(1+strlen(path))), path);
   char * s;
   char const * lang = "";
   int langLen = 0;
   for (
      char * option = strtok_r(options, "=", &s), * val=0;
      option && ((val = strtok_r(0, "&", &s)) != 0);
      option = strtok_r(0, "=", &s)
   ) {
      if (strcmp(option, "lang") == 0) {
         lang = val;
         break;
      }
   }
   // create the stream
   return new SpeakerStream(
      this,
      UnicodeString(lang, Encoder(EncodingModule::UTF_8))
   );
}

/*---------------------------------------------SpeakerSchemeHandler::Rep::say-+
|                                                                             |
+----------------------------------------------------------------------------*/
void SpeakerSchemeHandler::Rep::say(UnicodeString what, UnicodeString how)
{
   if (m_speaker) {
      g_jniEnv->CallVoidMethod(
         m_speaker,
         m_sayMethod,
         g_jniEnv->NewString(what, what.length()),
         g_jniEnv->NewString(how, how.length())
      );
   }
}

/*-------------------------------------------SpeakerSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * SpeakerSchemeHandler::Rep::getID() const {
   return "SPEAKER";
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
