/* $Id: MakeInternalPDB.REXX,v 1.3 2002/11/18 08:02:05 pgr Exp $ */

/*
| Here is an interesting example showing how Rexx deals with binaries.
|
| This REXX Script built a PalmOS PDB from a list of files on
| Linux, or Windoze.
| Note that the file extension will become the Palm Record Category
*/

/* Required Data */
dbName = 'RexxDB'   /* 'MemoDB' */
dbType = 'data'     /* 'DATA'   */         /* 4 letters, please! */
dbCreator = 'Jaxo'  /* 'memo'   */         /* 4 letters, please! */
dmRecNumCategories = 16
dmCategoryLength   = 16

list =,
  'AddCommas.REXX',
  'Alice.Personal',
  'Append2File.REXX',
  'Calc.REXX',
  'DeleteFile.REXX',
  'DeleteRecord.REXX',
  'File2Memo.REXX',
  'GetARexx.REXX',
  'HiPrec.REXX',
  'LoadRexxDB.REXX',
  'Memo2File.REXX',
  'Mortgage.REXX',
  'PlayMidi.REXX',
  'qpi.REXX',
  'RabbitHole.Personal',
  'Redmond.Business',
  'ReplaceString.REXX',
  'ReplaceWordsInFile.REXX',
  'Rexx2Memo.REXX',
  'RomanNumeral.REXX',
  'RunARexx.REXX',
  'SearchARexx.REXX',
  'ShowFile.REXX',
  'ShowFirstMemo.REXX',
  'ShowMemo.REXX',
  'Talk2Beamer.REXX',
  'Talk2Serial.REXX',
  'Talk2TcpIp.REXX',
  'WhenAreYouBorn.REXX',
  'WhoIs.REXX',
  'WriteMemo.REXX',
  'qheure.REXX',
  'qt.REXX',
  'rexxcps.REXX',
  'rexxtry.REXX',
  'internal/3dplot.r',
  'internal/a.r',
  'internal/a2.r',
  'internal/a3.r',
  'internal/animal.r',
  'internal/arith.r',
  'internal/babar.r',
  'internal/bench.r',
  'internal/block.r',
  'internal/c2d2c.r',
  'internal/cps.r',
  'internal/keywd.r',
  'internal/lexical.r',
  'internal/limits.r',
  'internal/p0a.r',
  'internal/p0b.r',
  'internal/p1.r',
  'internal/p2.r',
  'internal/p2_r.r',
  'internal/qt.r',
  'internal/queens.r',
  'internal/r.r',
  'internal/t1.r',
  'internal/t2.r',
  'internal/t4.r',
  'internal/t5.r',
  'internal/t6.r',
  'internal/t7.r',
  'internal/t8.r',
  'internal/t9.r',
  'internal/t11.r',
  'internal/tmpl.r',
  'internal/trip.r',
  'internal/tt.r',
  'internal/tutu.r',
  'internal/w.r'

/* Find out the path for all files (in's and out) */
recpath = Value('YAXX_HOME',, 'SYSTEM')
IF recpath = '' THEN DO
   SAY 'Please enter the YAXX_HOME directory'
   PARSE PULL recpath
   END
IF (recpath='') THEN recPath = 'd:\palmdev\yaxx' /* Joe's prefs */
recpath = recpath || '\mwerks\tests\'
output = recPath || "RexxDB.pdb"

/* Pre-process the list of files, setting up the categories */
catName.1 = 'Unfiled'
catName.count = 1
DO i=1 WHILE list <> ''
   PARSE VAR list recFilename.i list    /* break the list into recFileName.x */
   input = recPath || recFilename.i
   IF Stream(input) \= 'READY' THEN DO
      SAY "File" input "not found!"
      EXIT
      END
   ext = substr(recFilename.i,pos('.', recFilename.i)+1)
   if ext == 'r' THEN ext = 'REXX'       /* internal REXX */
   DO j=1 WHILE Symbol("catName.j") == 'VAR'
      IF ext == catName.j THEN ITERATE i /* category already found */
      END
   IF (j > dmRecNumCategories) THEN DO
      SAY "Too many categories! ('" || j || "' exceeds" dmRecNumCategories ||")"
      EXIT
      END
   catName.j = ext
   catName.count = j
   END
recFilename.count = i-1

/* Politely, tell user what we are about to do to him */
SAY 'Creating database "' || dbName || '" of type "' || dbType || '",'
SAY  'in "' || output || '".'
SAY 'It will contain' recFilename.count 'records in' catName.count-1 'categories.'
SAY "OK? (Y / N)"
PULL answer
IF answer \= 'Y' THEN DO
  SAY "Au revoir!"
  EXIT
  END

CALL Charout output, '', 1                      /* reset the file */
CALL Stream output, 'C', 'CLOSE'                /* really! */
/* this doesn't work on NT (FIXME) */
"del" output
"touch" output

/* Database Header */
CALL Charout output, Left(dbName, 32, '00'x)    /* DB name */
CALL Charout output, '0008'x                    /* Attributes */
CALL Charout output, '0000'x                    /* Version */
i = (Date('B') - 695055) * 86400                /* # of sec since 1904 */
CALL Charout output, D2c(i,4)                   /* Created */
CALL Charout output, D2c(i,4)                   /* modified */
CALL Charout output, D2c(0,4)                   /* Last backup */
CALL Charout output, D2c(0,4)                   /* Modification Number */
startAppInfoOffset = getCurPos(output)          /* AppInfo offset */
CALL Charout output, '????'                     /* (place holder)*/
CALL Charout output, D2c(0,4)                   /* SortInfo offset */
CALL Charout output, Left(dbType,4)             /* Database Type */
CALL Charout output, Left(dbCreator,4)          /* Creator ID */
CALL Charout output, D2c(0,4)                   /* Unique ID Seed */
CALL Charout output, D2c(0,4)                   /* Next record list */
CALL Charout output, D2c(recFilename.count,2)   /* Number of records */

/* Record List */
DO i=1 TO recFilename.count
   startRecOffset.i = getCurPos(output)         /* Offset of this record */
   CALL Charout output, '????'                  /* (place holder) */
   CALL Charout output, D2c(,
       C2d('40'x) + getCatNo(i))                /* Record Attributes */
   CALL Charout output, D2c(0,3)                /* Unique ID */
   END

/* Filler */
CALL Charout output, D2c(0,2)

/* resolve the AppInfoOffset */
CALL setAtPos output, startAppInfoOffset, D2c(getCurPos(output), 4)

/* AppInfo starts here */
CALL Charout output, D2c(0,2)                   /* renamed categories */
DO i=1 TO catName.count                         /* category labels */
   CALL Charout output, Left(catName.i, dmCategoryLength, '00'x)
END
i = (1+dmRecNumCategories - i) * dmCategoryLength;
CALL Charout output, Left('', i, '00'x)
CALL Charout output, D2c(0,dmRecNumCategories)  /* category unique ID */
CALL Charout output, D2c(0,1)                   /* last unique ID */
CALL Charout output, D2c(0,1)                   /* padding */

/* MemoDB extensions */
CALL Charout output, D2c(0,2)                   /* reserved2 */
CALL Charout output, D2c(0,2)                   /* reserved3 */
CALL Charout output, D2c(0,1)                   /* sort order */
CALL Charout output, D2c(0,1)                   /* reserved */

/* Write the records! */
DO i=1 TO recFilename.count
   /* resolve the RecOffset */
   CALL setAtPos output, startRecOffset.i, D2c(getCurPos(output), 4)
   input = recPath || recFilename.i
   SAY "Writing record" Format(i,3) "  " input
   DO Lines(input)
      CALL Lineout output, LineIn(input)
   END
   CALL Charout output, '00'x
END

SAY "Database created."
EXIT

/* Procedures */

getCatNo: PROCEDURE EXPOSE recFilename. catName.
   i = arg(1)
   ext = Substr(recFilename.i,pos('.',recFilename.i)+1)
   IF (ext == 'r') THEN ext = 'REXX'            /* Internal REXX */
   DO i=1 TO catName.count
      IF ext == catName.i THEN RETURN i-1
      END
   RETURN 0 /* that should not occur! */
   EXIT

getCurPos: PROCEDURE
   output = Arg(1)
   CALL Charout output, '', 1           /* seek to origin */
   here = chars(output)+1               /* where was here? */
   CALL Charout output, '', here        /* back to here */
   RETURN here-1

setAtPos: PROCEDURE
   PARSE ARG output, there, value
   CALL Charout output, '', 1           /* seek to origin */
   here = chars(output)+1               /* where was here? */
   CALL Charout output, value, there+1  /* write at this pos */
   CALL Charout output, '', here        /* back to here */
   RETURN

/* Et c'est tout pour aujourd'hui */
