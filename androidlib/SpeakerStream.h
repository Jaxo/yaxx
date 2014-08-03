/*
* $Id: SpeakerStream.h,v 1.1 2011-08-24 09:38:35 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/23/2011
*/
#ifndef COM_JAXO_ANDROID_SPEAKERSTREAM_H_INCLUDED
#define COM_JAXO_ANDROID_SPEAKERSTREAM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <jni.h>
#include <iostream>
#include "../toolslib/ucstring.h"
#include "../toolslib/URI.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-------------------------------------------------------SpeakerSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class SpeakerSchemeHandler : public URI::SchemeHandler {
   friend class SpeakerStream;
   friend class SpeakerStreamBuf;
public:
   SpeakerSchemeHandler(JNIEnv * env, jobject speaker);

private:
   class Rep : public URI::SchemeHandler::Rep {
      friend class SpeakerStreamBuf;
   public:
      Rep(JNIEnv * env, jobject speaker);
      virtual ~Rep();
   private:
      jobject m_speaker;
      jmethodID m_sayMethod;

      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
      void say(UnicodeString what, UnicodeString language);
   };
};

/* -- INLINES -- */
inline SpeakerSchemeHandler::SpeakerSchemeHandler(
   JNIEnv * env, jobject speaker
) : SchemeHandler(new Rep(env, speaker)) {
}

/*-----------------------------------------------------------SpeakerStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
class SpeakerStreamBuf : public streambuf {
   friend class SpeakerStream;
private:
   SpeakerStreamBuf(SpeakerSchemeHandler::Rep * rep, UnicodeString locale);
   SpeakerSchemeHandler::Rep * m_handlerRep;
   UnicodeString m_locale;

   streamsize xsputn(char const *, streamsize);
   int overflow(int);
   int underflow();
   int pbackfail(int);
   int sync();
   streampos seekoff(streamoff, ios::seekdir, ios::openmode);
   streampos seekpos(streampos, ios::openmode);
};

/* -- INLINES -- */
inline SpeakerStreamBuf::SpeakerStreamBuf(
   SpeakerSchemeHandler::Rep * handlerRep, UnicodeString locale
) :
   m_handlerRep(handlerRep), m_locale(locale)
{}
inline int SpeakerStreamBuf::overflow(int = EOF) { return EOF; }
inline int SpeakerStreamBuf::underflow()         { return EOF; }
inline int SpeakerStreamBuf::pbackfail(int)      { return EOF; }
inline int SpeakerStreamBuf::sync()              { return 0; }
inline streampos SpeakerStreamBuf::seekoff(streamoff, ios::seekdir, ios::openmode) {
   return EOF;
}
inline streampos SpeakerStreamBuf::seekpos(streampos, ios::openmode) {
   return EOF;
}

/*--------------------------------------------------------------SpeakerStream-+
| SpeakerStream IS-A iostream to manipulate a SpeakerStreamBuf.               |
+----------------------------------------------------------------------------*/
class SpeakerStream: public iostream {
public:
   SpeakerStream(SpeakerSchemeHandler::Rep * handlerRep, UnicodeString locale);
private:
   SpeakerStreamBuf m_buf;
   SpeakerStream & operator=(SpeakerStream const & source); // no!
   SpeakerStream(SpeakerStream const & source);             // no!
};

/* -- INLINES -- */
inline SpeakerStream::SpeakerStream(
   SpeakerSchemeHandler::Rep * handlerRep, UnicodeString locale
) : iostream(&m_buf), m_buf(handlerRep, locale)
{
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
