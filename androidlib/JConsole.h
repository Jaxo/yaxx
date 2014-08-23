/*
* $Id: JConsole.h,v 1.3 2011-08-31 07:26:34 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/5/2011
*/

#ifndef COM_JAXO_TOOLS_JCONSOLE_H_INCLUDED
#define COM_JAXO_TOOLS_JCONSOLE_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <jni.h>
#include <iostream>

#include "../toolslib/toolsgendef.h"
#include "../toolslib/migstream.h"
#include "../toolslib/ConsoleSchemeHandler.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------------------- class JConsole -+
|                                                                             |
+----------------------------------------------------------------------------*/
class JConsole : private streambuf {
public:
   JConsole(JNIEnv * env, jobject console);
   ~JConsole();
   bool isValid();
   void setResult(char const * result);
   void setMessage(char const * result);
   int system(char const * command);

private:
   int overflow(int);
   int underflow();
   int sync() { return 0; }
   streampos seekoff(streamoff, ios::seekdir, ios__openmode) { return EOF; }
   streampos seekpos(streampos sp, ios__openmode om) { return EOF; }
   jobjectArray buildArgs(char const * input);

   char _M_buf[1];
// jweak m_console;
   jobject m_console;
   jmethodID m_getMethod;
   jmethodID m_putMethod;
   jmethodID m_systemMethod;
   jfieldID m_resultField;
   jfieldID m_messageField;
   streambuf * m_previousInStreambuf;
   streambuf * m_previousOutStreambuf;
   streambuf * m_previousErrStreambuf;
};

class K_SchemeHandler : public ConsoleSchemeHandler {
public:
   class Rep : public ConsoleSchemeHandler::Rep {
   public:
      Rep(JNIEnv * env, jobject console);
      virtual int system(char const * command);
      void setResult(char const * result);
      void setMessage(char const * message);
   private:
      JConsole m_console;
   };
   K_SchemeHandler(JNIEnv * env, jobject console);
   void setResult(char const * result);
   void setMessage(char const * message);
};

/* -- INLINES -- */
inline K_SchemeHandler::K_SchemeHandler(JNIEnv * env, jobject console) :
   ConsoleSchemeHandler(new K_SchemeHandler::Rep(env, console)) {
}
inline void K_SchemeHandler::setResult(char const * result) {
   ((Rep *)inqData())->setResult(result);
}
inline void K_SchemeHandler::Rep::setResult(char const * result) {
   m_console.setResult(result);
}
inline void K_SchemeHandler::setMessage(char const * message) {
   ((Rep *)inqData())->setMessage(message);
}
inline void K_SchemeHandler::Rep::setMessage(char const * message) {
   m_console.setMessage(message);
}
inline int K_SchemeHandler::Rep::system(char const * command) {
   return m_console.system(command);
}
inline K_SchemeHandler::Rep::Rep(JNIEnv * env, jobject console) :
m_console(env, console) {
   if (!m_console.isValid()) {
//    LOGI("The console is a bit rotten");
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
