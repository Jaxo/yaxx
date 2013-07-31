/*
* $Id: JConsole.cpp,v 1.4 2011-08-31 07:26:34 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/5/2011
*/

#include "JConsole.h"
#include "../toolslib/SystemContext.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------------JConsole::JConsole-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
JConsole::JConsole(JNIEnv * env, jobject console)
{
   m_env = env;
   m_console = 0;
   /*
   | Hear this!  MacroMerde got a new pal in Shitland.
   | GNUC violated streambuf rules with proprietary stincking changes,
   | forcing us to create a buffer (for putback)
   */
   _M_in_beg = _M_buf;
   _M_in_end = _M_in_cur = _M_in_beg + 1;

   jclass clazz = env->GetObjectClass(console);
   if (clazz) {
      m_putMethod = env->GetMethodID(clazz, "put", "(I)V");
      m_getMethod = env->GetMethodID(clazz, "get", "()I");
      m_systemMethod = env->GetMethodID(clazz, "system", "([Ljava/lang/String;)I");
      m_resultField = env->GetFieldID(clazz, "m_result", "Ljava/lang/String;");
      m_errMessageField = env->GetFieldID(clazz, "m_errMessage", "Ljava/lang/String;");
      if (
         m_putMethod && m_getMethod && m_systemMethod &&
         m_resultField && m_errMessageField
      ) {
         m_console = console; // env->NewWeakGlobalRef(console);
         m_previousInStreambuf = SystemContext::cin().rdbuf();
         m_previousOutStreambuf = SystemContext::cout().rdbuf();
         m_previousErrStreambuf = SystemContext::cerr().rdbuf();
         SystemContext::cin().rdbuf(this);
         SystemContext::cout().rdbuf(this);
         SystemContext::cerr().rdbuf(&m_errBuf);
      }
      // env->DeleteLocalRef(clazz);
   }
}

/*--------------------------------------------------------JConsole::~JConsole-+
|                                                                             |
+----------------------------------------------------------------------------*/
JConsole::~JConsole() {
   if (m_console) {
      // m_env->DeleteWeakGlobalRef(m_console);
      SystemContext::cin().rdbuf(m_previousInStreambuf);
      SystemContext::cout().rdbuf(m_previousOutStreambuf);
      SystemContext::cerr() << (char)0;
      SystemContext::cerr().rdbuf(m_previousErrStreambuf);
      char const * errMessage = m_errBuf.str();
      if (errMessage) {
         m_env->SetObjectField(
            m_console,
            m_errMessageField,
            m_env->NewStringUTF(errMessage)
         );
         delete[] errMessage;
      }
      m_console = 0;
   }
}

/*----------------------------------------------------------JConsole::isValid-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool JConsole::isValid() {
   return (m_console != 0);
}

/*-----------------------------------------------------------JConsole::system-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::system(char const * command) {
  return m_env->CallIntMethod(m_console, m_systemMethod, buildArgs(command));
}

/*--------------------------------------------------------JConsole::underflow-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::underflow() {
   int c = m_env->CallIntMethod(m_console, m_getMethod);
   *_M_buf = c;
   _M_in_cur = _M_buf;
   return c;
}

/*---------------------------------------------------------JConsole::overflow-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::overflow(int c) {
   m_env->CallVoidMethod(m_console, m_putMethod, c);
}

/*--------------------------------------------------------JConsole::buildArgs-+
|                                                                             |
+----------------------------------------------------------------------------*/
jobjectArray JConsole::buildArgs(char const * cmdLine)
{
   int argsCount = 0;
   jstring * args = 0;
   jobjectArray array;

   if (cmdLine) {
      char * tempBuf;
      char ch;
      bool isBkslash = false;
      char chQuoteStart = 0;
      int argsLength = 0;
      while ((*cmdLine) && (*cmdLine <= 0x20)) ++cmdLine;
      tempBuf = (char *)malloc(1+strlen(cmdLine));
      do {
         char * arg = tempBuf;
         if (argsCount >= argsLength) {
            argsLength += 8;
            args = (jstring *)realloc(args, argsLength * sizeof (jstring));
         }
         for (
            char ch = *cmdLine;
            (ch && ((ch > 0x20) || chQuoteStart || isBkslash));
            ch = *++cmdLine
         ) {
            if (isBkslash) {
               isBkslash = false;
               *arg++ = ch;
            }else if (ch == '\\') {
               isBkslash = true;
            }else if (chQuoteStart) {
               if (ch == chQuoteStart) {
                  chQuoteStart = 0;
               }else {
                  *arg++ = ch;
               }
            }else if ((ch == '\'') || (ch == '"')) {
               chQuoteStart = ch;
            }else {
               *arg++ = ch;
            }
         }
         *arg = 0;
         args[argsCount++] = m_env->NewStringUTF(tempBuf);
         while ((*cmdLine) && (*cmdLine <= 0x20)) ++cmdLine;
      }while (*cmdLine);
      free(tempBuf);
   }
   array = m_env->NewObjectArray(
      argsCount, m_env->FindClass("java/lang/String"), 0
   );
   for (int i=0; i < argsCount; ++i) {
      m_env->SetObjectArrayElement(array, i, args[i]);
   }
   free(args);
   return array;
}

/*--------------------------------------------------------JConsole::setResult-+
|                                                                             |
+----------------------------------------------------------------------------*/
void JConsole::setResult(char const * result) {
   if (result) {
      m_env->SetObjectField(
         m_console,
         m_errMessageField,
         m_env->NewStringUTF(result)
      );
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
