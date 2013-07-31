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
#include "SpeakerStream.h"
#include "../toolslib/ucstring.h"
#include "../toolslib/Encoder.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------SpeakerStreamBuf::xsputn-+
|                                                                             |
+----------------------------------------------------------------------------*/
streamsize SpeakerStreamBuf::xsputn(char const * buf, streamsize length)
{
   UnicodeString temp(buf, Encoder(EncodingModule::UTF_8), length);
   m_env->CallVoidMethod(
      m_speaker,
      m_sayMethod,
      m_env->NewString(temp, temp.length())
   );
   return length;
}

/*---------------------------------------------SpeakerSchemeHandler::Rep::Rep-+
|                                                                             |
+----------------------------------------------------------------------------*/
SpeakerSchemeHandler::Rep::Rep(JNIEnv * env, jobject speaker) {
   m_env = env;
   jclass clazz = env->GetObjectClass(speaker);
   if (!clazz) {
      m_sayMethod = 0;
      m_speaker = 0;
   }else {
      m_sayMethod = env->GetMethodID(clazz, "say", "(Ljava/lang/String;)V");
      if (!m_sayMethod) {
         m_speaker = 0;
      }else {
         m_speaker = speaker;
      }
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
