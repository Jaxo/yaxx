/*
* $Id: memio.c,v 1.3 2012-04-07 15:25:29 pgr Exp $
*
* (C) Copyright Jaxo Inc., 2001
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo Inc.
*
* Author:  Pierre G. Richard
* Written: 4/24/2001
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

/*------------------+
| Local definitions |
+------------------*/
#define PARKING_LOT_SIZE 0x7FFF

#ifdef __GNUC__
#define t_MEM_FILE MEM_FILE
#define t_PARKING PARKING
#endif

typedef struct t_PARKING {
   struct t_PARKING * link;   /* previous parking */
   int real_length;
   unsigned char lot[PARKING_LOT_SIZE];
}PARKING;

typedef struct t_MEM_FILE {   /* "extends JFILE" */
   JFILE base;
   /* MEM extensions */
   PARKING * what_park;
   int posBeg;
   int ungot;
   char mode;                 /* 'r' or 'w' - always binary */
}MEM_FILE;

/* resolution of the JFILE abstract methods */
static JFILE * mfl_checkOpen(JFILE *, const char *, const char *);
static size_t mfl_fread(void *, size_t, size_t, JFILE *);
static size_t mfl_fwrite(const void *, size_t, size_t, JFILE *);
static int mfl_fgetc(JFILE *);
static int mfl_fputc(int, JFILE *);
static int mfl_ungetc(int, JFILE *);
static int mfl_feof(JFILE *);
static int mfl_fflush(JFILE *);
static int mfl_fclose(JFILE *);
static int mfl_fileno(JFILE *);

/* other local methods */
static MEM_FILE * newMemoryFile();
static void deleteMemoryFile(MEM_FILE *);
static void * makeNewParking(MEM_FILE *);
static void cleanUpParkings(MEM_FILE *);

/*-----------+
| Local Data |
+-----------*/
static char const aszMemoryFileName[] = "MemoryFile";

/*------------------------------------------------------acquireMemoryFileName-+
| Description:                                                                |
|   Obtain a name for accessing a MemoryFile through the standard stdio.      |
+----------------------------------------------------------------------------*/
char const * acquireMemoryFileName() {
   MEM_FILE * pMfl = newMemoryFile();
   if (pMfl) return pMfl->base.name;
   return 0;
}

/*------------------------------------------------------releaseMemoryFileName-+
| Description:                                                                |
|   Release the resources attached to a MeoryFile via its name                |
+----------------------------------------------------------------------------*/
void releaseMemoryFileName(char const * name) {
   deleteMemoryFile((MEM_FILE *)getFilePointerFromName(name));
}

/*--------------------------------------------------------------newMemoryFile-+
| Create a MemoryFile and returns its "name".                                 |
+----------------------------------------------------------------------------*/
static MEM_FILE * newMemoryFile()
{
   MEM_FILE * pMfl = (MEM_FILE *)malloc(sizeof(MEM_FILE));
   if (!pMfl) return 0;
   pMfl->base.name = makeNameFromFilePointer((JFILE*)pMfl, aszMemoryFileName);
   pMfl->base.info = 0;
   pMfl->base.isError = 0;
   pMfl->base.iOffset = 0;
   pMfl->base.exceptionHandler = 0;
   pMfl->base.checkOpen = mfl_checkOpen;
   pMfl->base.fread = mfl_fread;
   pMfl->base.fwrite = mfl_fwrite;
   pMfl->base.fgetc = mfl_fgetc;
   pMfl->base.fputc = mfl_fputc;
   pMfl->base.ungetc = mfl_ungetc;
   pMfl->base.feof = mfl_feof;
   pMfl->base.fflush = mfl_fflush;
   pMfl->base.fclose = mfl_fclose;
   pMfl->base.fileno = mfl_fileno;
   pMfl->what_park = 0;
   pMfl->posBeg = PARKING_LOT_SIZE;
   pMfl->ungot = JEOF;
   pMfl->mode = '?';
   return pMfl;
}

/*-----------------------------------------------------------deleteMemoryFile-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
static void deleteMemoryFile(MEM_FILE * pMfl)
{
   switch (pMfl->mode) {
   case '?' :               /* empty new file */
      break;
   case 'w' :               /* close me first! */
      mfl_fclose((JFILE *)pMfl);
      /* fall thru */
   default:
/* case 'r' :               -* opened for reading */
/* case 1+'w' :             -* written file not yet read */
      cleanUpParkings(pMfl);
      break;
   }
   free(pMfl->base.name);
   free(pMfl);
}

/*--------------------------------------------------------------mfl_checkOpen-+
|                                                                             |
+----------------------------------------------------------------------------*/
static JFILE * mfl_checkOpen(
   JFILE * pFile, const char * name, const char * mode
) {
   MEM_FILE * pMfl = (MEM_FILE *)pFile;
   switch (mode[0]) {
   case 'w':
      switch (pMfl->mode) {
      case '?' :               /* empty new file */
         break;
      case 1+'w' :
         cleanUpParkings(pMfl);
         break;                /* OK - reusing this MemoryFile */
      default:
   /* case 'r' :               -* already opened! */
   /* case 'w' :               -* close me first! */
         return 0;
      }
      pMfl->posBeg = PARKING_LOT_SIZE;
      break;
   case 'r':
      switch (pMfl->mode) {
      case '?' :               /* empty file */
      case 1+'w' :             /* written file not yet read */
         break;
      default:
   /* case 'r' :               -* already opened! */
   /* case 'w' :               -* close me first! */
         return 0;
      }
      pMfl->posBeg = 0;
      break;
   default:
      return 0;                /* unknown mode */
   }
   pMfl->ungot = -1;
   pMfl->mode = mode[0];
   return pFile;
}

/*-------------------------------------------------------------makeNewParking-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void * makeNewParking(MEM_FILE * pMfl)
{
   PARKING * next_park = (PARKING *)malloc(sizeof(PARKING));
   if (next_park) {
      next_park->link = pMfl->what_park;
      next_park->real_length = 0;
      pMfl->what_park = next_park;
      pMfl->posBeg = 0;
   }else {
      pMfl->base.isError = 1;
   }
   return next_park;
}

/*------------------------------------------------------------cleanUpParkings-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void cleanUpParkings(MEM_FILE * pMfl)
{
   PARKING * what_park = pMfl->what_park;
   pMfl->what_park = 0;
   while (what_park) {
      PARKING * temp_park = what_park->link;
      free(what_park);
      what_park = temp_park;
   }
}

/*------------------------------------------------------------------mfl_fread-+
| Implementation: early free.                                                 |
|    By design, what_park always points a PARKING that                        |
|    has a positive number of available bytes.                                |
+----------------------------------------------------------------------------*/
static size_t mfl_fread(void * ptr, size_t size, size_t count, JFILE * pFile)
{
   MEM_FILE * pMfl = (MEM_FILE *)pFile;
   if (pMfl->mode != 'r') {
      pMfl->base.isError = 1;
      return 0;
   }else if (pMfl->ungot != JEOF) {
      pMfl->ungot = JEOF;
      return pMfl->ungot;
   }else if (pMfl->what_park == 0) {
      return 0;
   }else {
      int iTotalLength = size * count;
      int iRemainLength = iTotalLength;
      while (iRemainLength >= 0) {
         int iAvailableLength = pMfl->what_park->real_length - pMfl->posBeg;
         if (iAvailableLength > iRemainLength) {  /* don't do >= ! */
            memcpy(ptr, pMfl->what_park->lot + pMfl->posBeg, iRemainLength);
            pMfl->posBeg += iRemainLength;
            break;
         }else {                                   /* if ==, change PARKING */
            PARKING * next_park = pMfl->what_park->link;
            memcpy(ptr, pMfl->what_park->lot + pMfl->posBeg, iAvailableLength);
            iRemainLength -= iAvailableLength;
            free(pMfl->what_park);
            pMfl->what_park = next_park;
            pMfl->posBeg = 0;
            if (!next_park) {
               iTotalLength -= iRemainLength;
               break;
            }
            ptr = (char *)ptr + iAvailableLength;
         }
      }
      return iTotalLength;
   }
}

/*-----------------------------------------------------------------mfl_fwrite-+
| Implementation: late malloc.                                                |
|    By design, what_park is not created until it is required.                |
|    Note the difference with read: here, what_park can have no more room.    |
+----------------------------------------------------------------------------*/
static size_t mfl_fwrite(
   const void * ptr, size_t size, size_t count, JFILE * pFile
) {
   MEM_FILE * pMfl = (MEM_FILE *)pFile;
   int const iTotalLength = size * count;
   if (pMfl->mode != 'w') {
      pMfl->base.isError = 1;
      return 0;
   }else if (iTotalLength > 0) {
      int iRemainLength = iTotalLength;
      for (;;) {
         int iAvailableLength = PARKING_LOT_SIZE - pMfl->posBeg;
         if (iAvailableLength >= iRemainLength) {
            memcpy(pMfl->what_park->lot + pMfl->posBeg, ptr, iRemainLength);
            pMfl->posBeg += iRemainLength;
            pMfl->what_park->real_length = pMfl->posBeg;
            break;
         }else {
            if (pMfl->what_park) {
               memcpy(pMfl->what_park->lot+pMfl->posBeg, ptr, iAvailableLength);
               pMfl->posBeg = PARKING_LOT_SIZE;
               pMfl->what_park->real_length = PARKING_LOT_SIZE;
               ptr = (char *)ptr + iAvailableLength;
               iRemainLength -= iAvailableLength;  /* guaranteed to be > 0 */
            }
            if (!makeNewParking(pMfl)) {
               return iTotalLength - iRemainLength;
            }
         }
      }
   }
   return iTotalLength;
}


/*------------------------------------------------------------------mfl_fgetc-+
| Implementation: early free.                                                 |
|    By design, what_park always points a PARKING that                        |
|    has a positive number of available bytes.                                |
+----------------------------------------------------------------------------*/
static int mfl_fgetc(JFILE * pFile)
{
   MEM_FILE * pMfl = (MEM_FILE *)pFile;
   if (pMfl->mode != 'r') {
      pMfl->base.isError = 1;
      return JEOF;
   }else if (pMfl->ungot != JEOF) {
      int uc = pMfl->ungot;
      pMfl->ungot = JEOF;
      return uc;
   }else if (pMfl->what_park == 0) {
      return JEOF;
   }else {
      int c = pMfl->what_park->lot[pMfl->posBeg];
      if (++pMfl->posBeg >= pMfl->what_park->real_length) {
         PARKING * next_park = pMfl->what_park->link;
         free(pMfl->what_park);
         pMfl->what_park = next_park;
         pMfl->posBeg = 0;
      }
      return c & 0xFF;
   }
}

/*------------------------------------------------------------------mfl_fputc-+
| Implementation: late malloc.                                                |
|    By design, what_park is not created until it is required.                |
|    Note the difference with read: here, what_park can have no more room.    |
+----------------------------------------------------------------------------*/
static int mfl_fputc(int c, JFILE * pFile)
{
   MEM_FILE * pMfl = (MEM_FILE *)pFile;
   if (pMfl->mode != 'w') {
      pMfl->base.isError = 1;
      return JEOF;
   }
   if ((pMfl->posBeg >= PARKING_LOT_SIZE) && (!makeNewParking(pMfl))) {
      return JEOF;
   }
   pMfl->what_park->lot[pMfl->posBeg++] = (unsigned char)c;
   pMfl->what_park->real_length = pMfl->posBeg;
   return c;
}

/*-----------------------------------------------------------------mfl_ungetc-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int mfl_ungetc(int ungot, JFILE * pFile)
{
   MEM_FILE * pMfl = (MEM_FILE *)pFile;
   if (pMfl->mode != 'r') {
      pMfl->base.isError = 1;
      return JEOF;
   }else {
      pMfl->ungot = ungot;
      return ungot;
   }
}

/*-------------------------------------------------------------------mfl_feof-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int mfl_feof(JFILE * pFile) {
   return (((MEM_FILE *)pFile)->what_park == 0)? 1 : 0;
}

/*------------------------------------------------------------------mfl_flush-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int mfl_fflush(JFILE * pFile) {
   return 0;           /* No real need for */
}

/*-----------------------------------------------------------------mfl_fclose-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int mfl_fclose(JFILE * pFile)
{
   MEM_FILE * pMfl = (MEM_FILE *)pFile;
   switch (pMfl->mode) {
   case 'w' :
      /* reverse the parkings'order */
      {
         PARKING * w1_park = pMfl->what_park;
         PARKING * w2_park = (PARKING *)0;
         while (w1_park) {              /* Inverse the pointer chain */
            PARKING* w3_park = w2_park; /* w3 is the next park */
            w2_park = w1_park;          /* w2 is the current park */
            w1_park = w2_park->link;    /* w1 is the previous park */
            w2_park->link = w3_park;    /* so "prev" is changed to "next" */
         }
         pMfl->what_park = w2_park;
      }
      pMfl->mode = 1 + 'w';
      return 0;
   case 'r' :               /* already opened! */
      cleanUpParkings(pMfl);
      pMfl->mode = '?';
      return 0;
   default:
/* case '?' :               /* empty new file (not even opened) */
/* case 1+'w' :             /* already closed */
      return JEOF;
   }
}

/*-----------------------------------------------------------------mfl_fileno-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int mfl_fileno(JFILE * pFile) {
   return 100;  /*>>>PGR: FIXME? */
}

/*===========================================================================*/

