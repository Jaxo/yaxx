/*
* $Id: stdio.c,v 1.4 2012-04-07 15:25:30 pgr Exp $
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
#ifndef USE_INPROC
#include "JFileSystem.jh"
#endif

/*------------------+
| Local definitions |
+------------------*/
extern void makeBasicSystemFiles();  // in basic io

/*---------------------------------Java_com_jaxo_io_JFileSystem_initIOLibrary-+
|                                                                             |
+----------------------------------------------------------------------------*/
#ifndef USE_INPROC
JNIEXPORT void JNICALL Java_com_jaxo_io_JFileSystem_initIOLibrary(
   JNIEnv * pEnv, jclass clazz
) {
   makeBasicSystemFiles();
}
#endif

/*---------------------------------------------------------------------jfopen-+
|                                                                             |
+----------------------------------------------------------------------------*/
JFILE * jfopen(const char * name, const char * mode)
{
   JFILE * pFile = getFilePointerFromName(name);
   return pFile->checkOpen(pFile, name, mode);
}

/*---------------------------------------------------------------------jfread-+
|                                                                             |
+----------------------------------------------------------------------------*/
size_t jfread(void * ptr, size_t size, size_t count, JFILE * pFile)
{
   if (pFile) {
      int iPos = pFile->fread(ptr, size, count, pFile);
      pFile->iOffset += iPos;
      return iPos;
   }else {
      return JEOF;
   }
}

/*--------------------------------------------------------------------jfwrite-+
|                                                                             |
+----------------------------------------------------------------------------*/
size_t jfwrite(const void * ptr, size_t size, size_t count, JFILE * pFile) {
   if (pFile) {
      return pFile->fwrite(ptr, size, count, pFile);
   }else {
      return 0;
   }
}

/*---------------------------------------------------------------------jfgetc-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfgetc(JFILE * pFile)
{
   if (pFile) {
      int c = pFile->fgetc(pFile);
      if (c != JEOF) ++pFile->iOffset;
      return c;
   }else {
      return JEOF;
   }
}

/*----------------------------------------------------------------------jgetc-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jgetc(JFILE * pFile) {
   return jfgetc(pFile);
}

/*---------------------------------------------------------------------jfputc-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfputc(int c, JFILE * pFile) {
   if (pFile) {
      return pFile->fputc(c, pFile);
   }else {
      return JEOF;
   }
}

/*----------------------------------------------------------------------jputc-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jputc(int c, JFILE * pFile) {
   return jfputc(c, pFile);
}

/*---------------------------------------------------------------------jfputs-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfputs(const char * psz, JFILE * pFile)
{
   int ret = 0;
   int length = strlen(psz);

   if ((length > 0) && (jfwrite(psz, 1, length, pFile) == 0)) ret = JEOF;
   return ret;
}

/*---------------------------------------------------------------------jfgets-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * jfgets(char * psz, int n, JFILE * pFile)
{
   int c;
   char *c_p = psz;

   while ((n--) && (JEOF != (c=jfgetc(pFile)))) {
      *c_p++ = c;
      if (c == '\n') break;
   }
   *c_p = '\0';
   return psz;
}

/*---------------------------------------------------------------------jftell-+
|                                                                             |
+----------------------------------------------------------------------------*/
/* >>>PGR: TODO? only the bytes read from the C side are taken into */
/*               account!                                           */
long jftell(JFILE * pFile) {
   if (pFile) {
      return pFile->iOffset;
   }else {
      return 0;
   }
}

/*--------------------------------------------------------------------jungetc-+
|
+----------------------------------------------------------------------------*/
int jungetc(int ungot, JFILE * pFile)
{
   if (pFile) {
      int c = pFile->ungetc(ungot, pFile);
      if (c != JEOF) --pFile->iOffset;
      return c;
   }else {
      return JEOF;
   }
}

/*----------------------------------------------------------------------jfeof-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfeof(JFILE * pFile) {
   if (pFile) {
      return pFile->feof(pFile);
   }else {
      return 1;
   }
}

/*--------------------------------------------------------------------jfflush-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfflush(JFILE * pFile) {
   if (pFile) {
      return pFile->fflush(pFile);
   }else {
      return 0;
   }
}

/*--------------------------------------------------------------------jfclose-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfclose(JFILE * pFile) {
   if (pFile) {
      return pFile->fclose(pFile);
   }else {
      return 0;
   }
}

/*--------------------------------------------------------------------jferror-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jferror(JFILE * pFile) {
   if (pFile) {
      return pFile->isError;
   }else {
      return 0;
   }
}

/*------------------------------------------------------------------jclearerr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void jclearerr(JFILE * pFile) {
   if (pFile) {
      pFile->isError = 0;
   }
}

/*--------------------------------------------------------------------jfileno-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfileno(JFILE * pFile)
{
   /* >>>PGR: could make this better? (not easy because of reentrancy) */
   if (pFile) {
      return pFile->fileno(pFile);
   }else {
      return 0;
   }
}

/*--------------------------------------------------------setExceptionHandler-+
| To reset, exceptionHandler must be 0.                                       |
+----------------------------------------------------------------------------*/
void setExceptionHandler(
   JFILE * pFile, jmp_buf * exceptionHandler, int exceptionUserData
) {
   if (pFile) {
      pFile->exceptionHandler = exceptionHandler;
      pFile->exceptionUserData = exceptionUserData;
   }
}

/*-------------------------------------------------------------------getInfos-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * getInfos(JFILE * pFile) {
   return pFile->info;
}

/*----------------------------------------------------makeNameFromFilePointer-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * makeNameFromFilePointer(
   JFILE const * pFile,
   char const * pszClassName
) {
   char * name = (char *)malloc(strlen(pszClassName) + 1 + 8 + 1);
   jsprintf(name, "%s@%x", pszClassName, pFile);
   return name;
}

/*-----------------------------------------------------getFilePointerFromName-+
|                                                                             |
+----------------------------------------------------------------------------*/
JFILE * getFilePointerFromName(char const * name)
{
   char const * c_p;

   for (c_p=name; *c_p; ++c_p) {
      if (*c_p == '@') {
         int p;
         if (sscanf(c_p+1, "%x", &p) > 0) {
            return (JFILE *)p;
         }
         return 0;
      }
   }
   return 0;
}

/*===========================================================================*/

