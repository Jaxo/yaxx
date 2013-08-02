/*
* $Id: jniio.c,v 1.3 2012-04-07 15:25:29 pgr Exp $
*
* (C) Copyright Jaxo Inc., 2001
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo Inc.
*
* Author:  Pierre G. Richard
* Written: 2/2/2001
*
*/

/*---------+
| Includes |
+---------*/
#if !defined USE_REAL_JFILEMETHODS
#define USE_REAL_JFILEMETHODS
#endif
#include "stdio.h"
#include <memory.h>
#include <string.h>
#include "JFileInputStream.jh"
#include "JFileOutputStream.jh"

/*------------------+
| Local definitions |
+------------------*/
typedef enum {
   JFL_READ,       /* int read()                      0 */
   JFL_READ_BA,    /* int read(byte[], int, int)      1 */
   JFL_UNREAD,     /* void unread(int)                2 */
   JFL_UNREAD_BA,  /* void unread(byte[], int, int)   3 */
   JFL_UNREAD_BAS, /* void unread(byte[])             4 */
   JFL_AVAIL,      /* int available()                 5 */
   JFL_SKIP,       /* long skip(long)                 6 */
   JFL_MARKSUP,    /* boolean markSupported();        7 */
   JFL_CLOSE,      /* void close()                    8 */
   JFL_WRITE,      /* void write(int)                 9 */
   JFL_WRITE_BAS,  /* void write(byte[])             10 */
   JFL_WRITE_BA,   /* void write(byte[], int, int)   11 */
   JFL_FLUSH,      /* void flush()                   12 */
   JFL_FILENO      /* int fileno()                   13 */
}JFL_MethodId;

#ifdef __GNUC__
#undef JNI_FILE
#define t_JNI_FILE JNI_FILE
#endif

typedef struct t_JNI_FILE {  /* "extends JFILE" */
   JFILE base;
   /* JNI extensions */
   JNIEnv * pEnv;
   jobject obj;
   jmethodID * midTable;
}JNI_FILE;

/* resolution of the JFILE abstract methods */
static JFILE * jfl_checkOpen(JFILE *, const char *, const char *);
static size_t jfl_fread(void *, size_t, size_t, JFILE *);
static size_t jfl_fwrite(const void *, size_t, size_t, JFILE *);
static int jfl_fgetc(JFILE *);
static int jfl_fputc(int, JFILE *);
static int jfl_ungetc(int, JFILE *);
static int jfl_feof(JFILE *);
static int jfl_fflush(JFILE *);
static int jfl_fclose(JFILE *);
static int jfl_fileno(JFILE *);

/* other local methods */
static JNI_FILE * newJniInputFile(JNIEnv *, jobject, jstring);
static JNI_FILE * newJniOutputFile(JNIEnv *, jobject, jstring);
static JNI_FILE * newJniFile(
   JNIEnv *, jobject, jmethodID *, char const *, jstring
);
static void deleteJniFile(JNI_FILE * pJfl);
static int callJniFileMethod(JNI_FILE *, void *, JFL_MethodId, ...);
static JNI_FILE * getFilePointerFromString(JNIEnv * pEnv, jstring const magic);
static void initClass(JNIEnv *, char const *, jmethodID *);
static int callMethod(JNIEnv *,jobject,char const *,char const *,...);
static int checkException(JNI_FILE *);

/*-----------+
| Local Data |
+-----------*/
static struct {
   char const * name;
   char const * signature;
   JFL_MethodId const mtid;
}const methodsList[] = {
   {"read",      "()I",     JFL_READ       }, /* int read() */
   {"read",      "([BII)I", JFL_READ_BA    }, /* int read(byte[], int, int) */
   {"unread",    "(I)V",    JFL_UNREAD     }, /* void unread(int) */
   {"unread",    "([BII)V", JFL_UNREAD_BA  }, /* void unread(byte[], int, int) */
   {"unread",    "([B)V",   JFL_UNREAD_BAS }, /* void unread(byte[]) */
   {"available", "()I",     JFL_AVAIL      }, /* int available() */
   {"skip",      "(J)J",    JFL_SKIP       }, /* long skip(long) */
   {"markSupported", "()Z", JFL_MARKSUP    }, /* boolean markSupported() */
   {"close",     "()V",     JFL_CLOSE      }, /* void close() */
   {"write",     "(I)V",    JFL_WRITE      }, /* void write(int) */
   {"write",     "([B)V",   JFL_WRITE_BAS  }, /* void write(byte[]) */
   {"write",     "([BII)V", JFL_WRITE_BA   }, /* void write(byte[], int, int) */
   {"flush",     "()V",     JFL_FLUSH      }, /* void flush() */
   {"fileno",    "()I",     JFL_FILENO     }  /* int fileno() */
};
static char const aszJniInputStreamClassName[] =
"com/jaxo/io/JFileInputStream";

static char const aszJniOutputStreamClassName[] =
"com/jaxo/io/JFileOutputStream";

static jmethodID midInputTable[(sizeof methodsList / sizeof methodsList[0])];
static jmethodID midOutputTable[(sizeof methodsList / sizeof methodsList[0])];
static char midReturnType[(sizeof methodsList / sizeof methodsList[0])];

/*-------------------------------------Java_com_jaxo_io_JFileInputStream_init-+
|                                                                             |
+----------------------------------------------------------------------------*/
JNIEXPORT void JNICALL Java_com_jaxo_io_JFileInputStream_init(
   JNIEnv * pEnv, jclass clazz
) {
   initClass(pEnv, aszJniInputStreamClassName, midInputTable);
}

/*--------------------------------Java_com_jaxo_io_JFileInputStream_setBridge-+
|                                                                             |
+----------------------------------------------------------------------------*/
JNIEXPORT jstring JNICALL Java_com_jaxo_io_JFileInputStream_setBridge(
   JNIEnv * pEnv, jobject obj, jstring infos
) {
   JNI_FILE * pJfl = newJniInputFile(pEnv, obj, infos);
   return (*pEnv)->NewStringUTF(pEnv, pJfl->base.name);
}

/*-----------------------------Java_com_jaxo_io_JFileInputStream_removeBridge-+
|                                                                             |
+----------------------------------------------------------------------------*/
JNIEXPORT void JNICALL Java_com_jaxo_io_JFileInputStream_removeBridge(
  JNIEnv * pEnv, jobject obj, jstring name
) {
   deleteJniFile(getFilePointerFromString(pEnv, name));
}

/*------------------------------------Java_com_jaxo_io_JFileOutputStream_init-+
|                                                                             |
+----------------------------------------------------------------------------*/
JNIEXPORT void JNICALL Java_com_jaxo_io_JFileOutputStream_init(
   JNIEnv * pEnv, jclass clazz
) {
   initClass(pEnv, aszJniOutputStreamClassName, midOutputTable);
}

/*-------------------------------Java_com_jaxo_io_JFileOutputStream_setBridge-+
|                                                                             |
+----------------------------------------------------------------------------*/
JNIEXPORT jstring JNICALL Java_com_jaxo_io_JFileOutputStream_setBridge(
   JNIEnv * pEnv, jobject obj, jstring infos
) {
   JNI_FILE * pJfl = newJniOutputFile(pEnv, obj, infos);
   return (*pEnv)->NewStringUTF(pEnv, pJfl->base.name);
}

/*----------------------------Java_com_jaxo_io_JFileOutputStream_removeBridge-+
|                                                                             |
+----------------------------------------------------------------------------*/
JNIEXPORT void JNICALL Java_com_jaxo_io_JFileOutputStream_removeBridge(
  JNIEnv * pEnv, jobject obj, jstring name
) {
   deleteJniFile(getFilePointerFromString(pEnv, name));
}

/*------------------------------------------------------------newJniInputFile-+
| Construct a JniInputFile                                                    |
+----------------------------------------------------------------------------*/
static JNI_FILE * newJniInputFile(JNIEnv * pEnv, jobject obj, jstring infos) {
   return newJniFile(
      pEnv, obj, midInputTable, aszJniInputStreamClassName, infos
   );
}

/*-----------------------------------------------------------newJniOutputFile-+
| Construct a JniOutputFile                                                   |
+----------------------------------------------------------------------------*/
static JNI_FILE * newJniOutputFile(JNIEnv * pEnv, jobject obj, jstring infos) {
   return newJniFile(
      pEnv, obj, midOutputTable, aszJniOutputStreamClassName, infos
   );
}

/*-----------------------------------------------------------------newJniFile-+
| Construct a JniFile (must be seen as a private base class)                  |
+----------------------------------------------------------------------------*/
static JNI_FILE * newJniFile(
   JNIEnv * pEnv,
   jobject obj,
   jmethodID * midTable,
   char const * pszClassName,
   jstring infos
) {
   JNI_FILE * pJfl = malloc(sizeof(JNI_FILE));
   pJfl->base.name = makeNameFromFilePointer((JFILE*)pJfl, pszClassName);
   if (infos) {
      char const * pInfos = (*pEnv)->GetStringUTFChars(pEnv, infos, 0);
      int len = 1+strlen(pInfos);
      pJfl->base.info = (char *)malloc(len);
      memcpy(pJfl->base.info, pInfos, len);
      (*pEnv)->ReleaseStringUTFChars(pEnv, infos, pInfos);
   }else {
      pJfl->base.info = 0;
   }
   pJfl->base.isError = 0;
   pJfl->base.iOffset = 0;
   pJfl->base.exceptionHandler = 0;
   pJfl->base.checkOpen = jfl_checkOpen;
   pJfl->base.fread = jfl_fread;
   pJfl->base.fwrite = jfl_fwrite;
   pJfl->base.fgetc = jfl_fgetc;
   pJfl->base.fputc = jfl_fputc;
   pJfl->base.ungetc = jfl_ungetc;
   pJfl->base.feof = jfl_feof;
   pJfl->base.fflush = jfl_fflush;
   pJfl->base.fclose = jfl_fclose;
   pJfl->base.fileno = jfl_fileno;
   pJfl->pEnv = pEnv;
   pJfl->obj = (*pEnv)->NewGlobalRef(pEnv, obj);
   pJfl->midTable = midTable;
   return pJfl;
}

/*--------------------------------------------------------------deleteJniFile-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
static void deleteJniFile(JNI_FILE * pJfl)
{
   (*pJfl->pEnv)->DeleteGlobalRef(pJfl->pEnv, pJfl->obj);
   free(pJfl->base.name);
   if (pJfl->base.info) free(pJfl->base.info);
   free(pJfl);
}

/*--------------------------------------------------------------jfl_checkOpen-+
|                                                                             |
+----------------------------------------------------------------------------*/
static JFILE * jfl_checkOpen(
   JFILE * pFile, const char * name, const char * mode
) {
   /*>>>PGR: FIXME
   | Here I should check that the C-File mode is compatible with the type
   | of Java Stream.  e.g "r" <-> InputStream, "w" <-> OutputStream
   */
   return pFile;
}

/*------------------------------------------------------------------jfl_fread-+
|                                                                             |
+----------------------------------------------------------------------------*/
static size_t jfl_fread(void * ptr, size_t size, size_t count, JFILE * pFile)
{
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   JNIEnv * pEnv = pJfl->pEnv;
   int iLength = size * count;
   jbyteArray ba = (*pEnv)->NewByteArray(pEnv, iLength);
   int iPos = 0;
   int iRead;
   jbyte * pByte;

   do {
      callJniFileMethod(pJfl, &iRead, JFL_READ_BA, ba, iPos, iLength);
      if (checkException(pJfl)) {
        return 0;
      }
      if (iRead == -1) {
         iLength = 0;
      }else {
         iPos += iRead;
         iLength -= iRead;
      }
   }while (iLength);
   pByte = (*pEnv)->GetByteArrayElements(pEnv, ba, 0);
   memcpy(ptr, pByte, iPos);
   (*pEnv)->ReleaseByteArrayElements(pEnv, ba, pByte, 0);
   return iPos;
}

/*-----------------------------------------------------------------jfl_fwrite-+
|                                                                             |
+----------------------------------------------------------------------------*/
static size_t jfl_fwrite(
   const void * ptr, size_t size, size_t count, JFILE * pFile
) {
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   JNIEnv * pEnv = pJfl->pEnv;
   int iLength = size * count;
   jbyteArray ba = (*pEnv)->NewByteArray(pEnv, iLength);

   (*pEnv)->SetByteArrayRegion(pEnv, ba, 0, iLength, (jbyte *)ptr);
   callJniFileMethod(pJfl, 0, JFL_WRITE_BAS, ba);
   if (checkException(pJfl)) {
     return 0;
   }
   return iLength;
}

/*------------------------------------------------------------------jfl_fgetc-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int jfl_fgetc(JFILE * pFile)
{
   int c;
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   JNIEnv * pEnv = pJfl->pEnv;

   callJniFileMethod(pJfl, &c, JFL_READ);
   if (checkException(pJfl)) {
     return JEOF;
   }
   return c;   /* assume that the C EOF is -1 (which is true) */
}

/*------------------------------------------------------------------jfl_fputc-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int jfl_fputc(int c, JFILE * pFile)
{
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   JNIEnv * pEnv = pJfl->pEnv;

   callJniFileMethod(pJfl, 0, JFL_WRITE, c);
   if (checkException(pJfl)) {
     return JEOF;
   }
   return c;
}

/*-----------------------------------------------------------------jfl_ungetc-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int jfl_ungetc(int ungot, JFILE * pFile)
{
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   JNIEnv * pEnv = pJfl->pEnv;

   callJniFileMethod(pJfl, 0, JFL_UNREAD, ungot);
   if (checkException(pJfl)) {
     return JEOF;
   }
   return ungot;
}

/*-------------------------------------------------------------------jfl_feof-+
|                                                                             |
+----------------------------------------------------------------------------*/
/* >>>PGR not too sure this is the best thing to do... */
static int jfl_feof(JFILE * pFile)
{
   int c = jfl_fgetc(pFile);
   if (c == JEOF) {
      return 1;
   }else {
      jfl_ungetc(c, pFile);
      return 0;
   }
}

/*------------------------------------------------------------------jfl_flush-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int jfl_fflush(JFILE * pFile)
{
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   JNIEnv * pEnv = pJfl->pEnv;

   callJniFileMethod(pJfl, 0, JFL_FLUSH);
   if (checkException(pJfl)) {
     return JEOF;
   }
   return 0;
}

/*-----------------------------------------------------------------jfl_fclose-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int jfl_fclose(JFILE * pFile)
{
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   callJniFileMethod(pJfl, 0, JFL_CLOSE);
   if (checkException(pJfl)) {
      return JEOF;
   }
   return 0;
}

/*-----------------------------------------------------------------jfl_fileno-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int jfl_fileno(JFILE * pFile)
{
   JNI_FILE * pJfl = (JNI_FILE *)pFile;
   int no;
   callJniFileMethod(pJfl, &no, JFL_FILENO);
   return no;
}

/*-------------------------------------------------------------checkException-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int checkException(JNI_FILE * pJfl)
{
   JNIEnv * pEnv = pJfl->pEnv;
   if (!(*pEnv)->ExceptionOccurred(pEnv)) {
      return 0;
   }
   pJfl->base.isError = 1;
   (*pEnv)->ExceptionClear(pEnv);
   if (0 != pJfl->base.exceptionHandler) {
      longjmp(*pJfl->base.exceptionHandler, pJfl->base.exceptionUserData);
   }
   return 1;
}

/*----------------------------------------------------------callJniFileMethod-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int callJniFileMethod(
   JNI_FILE * pJfl,
   void * retval,      /* the way C does it. */
   JFL_MethodId mtid,  /* enum associated to the method */
   ...                 /* method arguments */
) {
   int isOk;
   JNIEnv * pEnv = pJfl->pEnv;
   jmethodID mid = pJfl->midTable[mtid];
   va_list arg;

   va_start(arg, mtid);
   if (mid == 0) {
      isOk = 0;       /* aka false */
   }else {
      isOk = 1;       /* be optimistic! */
      switch (midReturnType[mtid]) {
      case 'B':
         *(jbyte*)retval = (*pEnv)->CallByteMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'C':
         *(jchar*)retval = (*pEnv)->CallCharMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'D':
         *(jdouble*)retval = (*pEnv)->CallDoubleMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'F':
         *(jfloat*)retval = (*pEnv)->CallFloatMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'I':
         *(jint*)retval = (*pEnv)->CallIntMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'J':
         *(jlong*)retval = (*pEnv)->CallLongMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'L':
         *(jobject*)retval = (*pEnv)->CallObjectMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'S':
         *(jshort*)retval = (*pEnv)->CallShortMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'V':
         (*pEnv)->CallVoidMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      case 'Z':
         *(jboolean*)retval = (*pEnv)->CallBooleanMethodV(
            pEnv, pJfl->obj, mid, arg
         );
         break;
      default:
         isOk = 0;
         break;
      }
   }
   va_end(arg);
   return isOk;
}

/*---------------------------------------------------getFilePointerFromString-+
|                                                                             |
+----------------------------------------------------------------------------*/
static JNI_FILE * getFilePointerFromString(JNIEnv * pEnv, jstring const name)
{
   jbyte const * psz = (*pEnv)->GetStringUTFChars(pEnv, name, 0);
   JNI_FILE * pJfl = (JNI_FILE *)getFilePointerFromName(psz);
   (*pEnv)->ReleaseStringUTFChars(pEnv, name, psz);
   return pJfl;
}

/*------------------------------------------------------------------initClass-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void initClass(
   JNIEnv * pEnv,
   char const * className,
   jmethodID * midTable
) {
   int i, j;
   jclass clazz = (*pEnv)->FindClass(pEnv, className);

   if (clazz == 0) {
      jfprintf(jstderr, "*** can't locate %s class.\n", className);
   }
   /*
   | The GlobalRef below is never deleted, but this is somehow "normal".
   | - initClass is called 2 times at most (for JFileOutputStream and
   |   JFileInputStream) - 2 references won't eat all the memory!
   | - the life time of these classes is the same as stdin, stdout
   |   and stderr, e.g. infinite.
   */
   clazz = (*pEnv)->NewGlobalRef(pEnv, clazz);
   for (i=0; i < (sizeof methodsList / sizeof methodsList[0]); ++i) {
      midTable[methodsList[i].mtid] = (*pEnv)->GetMethodID(
         pEnv, clazz, methodsList[i].name, methodsList[i].signature
      );
      /*>>>PGR: FIXME
      | I should check the exception is: java.lang.NoSuchMethodError
      */
      if ((*pEnv)->ExceptionOccurred(pEnv)) {
         (*pEnv)->ExceptionClear(pEnv);
      }
   }
   for (i=0; i < (sizeof methodsList / sizeof methodsList[0]); ++i) {
      char const * sig = methodsList[i].signature;
      for (j=0; ; ++j) {
         if (sig[j] == ')') {
            midReturnType[methodsList[i].mtid] = sig[j+1];
            break;
         }
      }
   }
}

/*===========================================================================*/

