/*
* $Id: basio.c,v 1.2 2002-04-04 05:10:59 jlatone Exp $
*
* (C) Copyright Jaxo Inc., 2001
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo Inc.
*
* Author:  Pierre G. Richard
* Written: 4/29/2001
*
*/

/*---------+
| Includes |
+---------*/
#if !defined USE_REAL_JFILEMETHODS
#define USE_REAL_JFILEMETHODS
#endif
#include "stdio.h"

#if defined __sun
#include <unistd.h>

#elif defined _WIN32
#include <io.h>

#elif defined __linux
#include <unistd.h>

#elif defined __hpux
#include <unistd.h>

#else
#error Unknown System

#endif

/*------------------+
| Local definitions |
+------------------*/
#ifdef __GNUC__
#define t_BAS_FILE BAS_FILE
#endif

typedef struct t_BAS_FILE {   /* "extends JFILE" */
   JFILE base;
   /* BAS extensions */
   int fileno;
   int ungot;
   char mode;                 /* 'r' or 'w' - always binary */
}BAS_FILE;

/* resolution of the FILE abstract methods */
static JFILE * bfl_checkOpen(JFILE *, const char *, const char *);
static size_t bfl_fread(void *, size_t, size_t, JFILE *);
static size_t bfl_fwrite(const void *, size_t, size_t, JFILE *);
static int bfl_fgetc(JFILE *);
static int bfl_fputc(int, JFILE *);
static int bfl_ungetc(int, JFILE *);
static int bfl_feof(JFILE *);
static int bfl_fflush(JFILE *);
static int bfl_fclose(JFILE *);
static int bfl_fileno(JFILE *);

/* other local methods */
static BAS_FILE * newBasicFile(int, char);
static void deleteBasicFile(BAS_FILE *);

/*-----------+
| Local Data |
+-----------*/
static char const aszBasicFileName[] = "BasicFile";

/*-------------------------------------------------------makeBasicSystemFiles-+
| Description:                                                                |
|   Make the 3 basic system files: stdin, stdio, stderr                       |
+----------------------------------------------------------------------------*/
void makeBasicSystemFiles() {
   jstdin = (JFILE *)newBasicFile(0, 'r');
   jstdout = (JFILE *)newBasicFile(1, 'w');
   jstderr = (JFILE *)newBasicFile(2, 'w');
}

/*---------------------------------------------------------------newBasicFile-+
| Create a BasicFile and returns its "name".                                  |
+----------------------------------------------------------------------------*/
static BAS_FILE * newBasicFile(int fileno, char mode)
{
   BAS_FILE * pBfl = (BAS_FILE *)malloc(sizeof(BAS_FILE));
   pBfl->base.name = makeNameFromFilePointer((JFILE*)pBfl, aszBasicFileName);
   pBfl->base.info = 0;
   pBfl->base.isError = 0;
   pBfl->base.iOffset = 0;
   pBfl->base.exceptionHandler = 0;
   pBfl->base.checkOpen = bfl_checkOpen;
   pBfl->base.fread = bfl_fread;
   pBfl->base.fwrite = bfl_fwrite;
   pBfl->base.fgetc = bfl_fgetc;
   pBfl->base.fputc = bfl_fputc;
   pBfl->base.ungetc = bfl_ungetc;
   pBfl->base.feof = bfl_feof;
   pBfl->base.fflush = bfl_fflush;
   pBfl->base.fclose = bfl_fclose;
   pBfl->base.fileno = bfl_fileno;
   pBfl->fileno = fileno;
   pBfl->ungot = JEOF;
   pBfl->mode = mode;
   return pBfl;
}

/*------------------------------------------------------------deleteBasicFile-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
static void deleteBasicFile(BAS_FILE * pBfl)
{
   free(pBfl->base.name);
   free(pBfl);
}

/*--------------------------------------------------------------bfl_checkOpen-+
|                                                                             |
+----------------------------------------------------------------------------*/
static JFILE * bfl_checkOpen(
   JFILE * pFile, const char * name, const char * mode
) {
   return 0;          /* forbidden (and cannot be called) */
}

/*------------------------------------------------------------------bfl_fread-+
|                                                                             |
+----------------------------------------------------------------------------*/
static size_t bfl_fread(void * ptr, size_t size, size_t count, JFILE * pFile)
{
#if defined __linux
   return JEOF;   /* FIXME!!!!! */
#elif defined __sun
   return JEOF;   /* FIXME!!!!! */
#else
   BAS_FILE * pBfl = (BAS_FILE *)pFile;
   if (pBfl->mode != 'r') {
      pBfl->base.isError = 1;
      return 0;
   }else if (pBfl->ungot != JEOF) {
      pBfl->ungot = JEOF;
      return pBfl->ungot;
   }else {
      int i = _read(pBfl->fileno, ptr, size * count);
      if ((i == 0) && (size != 0) && (count != 0)) {
         return JEOF;
      }else if (i < 0) {
         pBfl->base.isError = 1;
         return 0;
      }else {
         return i;
      }
   }
#endif
}

/*-----------------------------------------------------------------bfl_fwrite-+
|                                                                             |
+----------------------------------------------------------------------------*/
static size_t bfl_fwrite(
   const void * ptr, size_t size, size_t count, JFILE * pFile
) {
#if defined __linux
   return size * count;   /* FIXME!!!!! */
#elif defined __sun
   return size * count;   /* FIXME!!!!! */
#else
   BAS_FILE * pBfl = (BAS_FILE *)pFile;
   int const iTotalLength = size * count;
   if (pBfl->mode != 'w') {
      pBfl->base.isError = 1;
      return 0;
   }else {
      int i = _write(pBfl->fileno, ptr, size * count);
      if (i < 0) {
         return JEOF;
      }else {
         return i;
      }
   }
#endif
}

/*------------------------------------------------------------------bfl_fgetc-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int bfl_fgetc(JFILE * pFile)
{
#if defined __linux
   return JEOF;   /* FIXME!!!!! */
#elif defined __sun
   return JEOF;   /* FIXME!!!!! */
#else
   char ch;
   return (bfl_fread(&ch, 1, 1, pFile) <= 0)? JEOF : ch & 0xff;
#endif
}

/*------------------------------------------------------------------bfl_fputc-+
| Implementation: late malloc.                                                |
|    By design, what_park is not created until it is required.                |
|    Note the difference with read: here, what_park can have no more room.    |
+----------------------------------------------------------------------------*/
static int bfl_fputc(int c, JFILE * pFile)
{
#if defined __linux
   return c;   /* FIXME!!!!! */
#elif defined __sun
   return c;   /* FIXME!!!!! */
#else
   char ch = (char)c;
   return (bfl_fwrite(&ch, 1, 1, pFile) <= 0)? JEOF : c;
#endif
}

/*-----------------------------------------------------------------bfl_ungetc-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int bfl_ungetc(int ungot, JFILE * pFile)
{
#if defined __linux
   return ungot;   /* FIXME!!!!! */
#elif defined __sun
   return ungot;   /* FIXME!!!!! */
#else
   BAS_FILE * pBfl = (BAS_FILE *)pFile;
   if (pBfl->mode != 'r') {
      pBfl->base.isError = 1;
      return JEOF;
   }else {
      pBfl->ungot = ungot;
      return ungot;
   }
#endif
}

/*-------------------------------------------------------------------bfl_feof-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int bfl_feof(JFILE * pFile) {
#if defined __linux
   return 1;   /* FIXME!!!!! */
#elif defined __sun
   return 1;   /* FIXME!!!!! */
#else
   return _eof(((BAS_FILE *)pFile)->fileno);
#endif
}

/*------------------------------------------------------------------bfl_flush-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int bfl_fflush(JFILE * pFile) {
   return 0;           /* No real need for */
}

/*-----------------------------------------------------------------bfl_fclose-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int bfl_fclose(JFILE * pFile)
{
   return JEOF;  /* forbidden */
}

/*-----------------------------------------------------------------bfl_fileno-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int bfl_fileno(JFILE * pFile) {
   return ((BAS_FILE *)pFile)->fileno;
}

/*===========================================================================*/

