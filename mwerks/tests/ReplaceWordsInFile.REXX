/* ReplaceWordsInFile
Replace words within a "file" (FileStream, Memo record, etc.)
*/
ReplaceWordsInFile:
   /* find ReplaceString in the RexxDB - and load it */
   DO i=1 BY 1
      input = "dm:///RexxDB/REXX/" || i
      IF Stream(input) \= "READY" THEN DO
         SAY "Sorry! ReplaceString was not found within the RexxDB."
         EXIT
         END
      IF Word(Linein(input), 2) = "ReplaceString" THEN DO
         CALL Load input
         LEAVE
         END
      END

   /* check for a valid input file */
   SAY "What file? (URL, please)"
   PARSE PULL file
   IF file = "" | Stream(file) \= "READY" THEN DO
      SAY "Sorry!" file "was not found."
      EXIT
      END

   /* replaced, replacement? case? */
   SAY "Replace what?"
   PARSE PULL from
   SAY "To?"
   PARSE PULL to
   SAY "Case dependent? (Y / N)?"
   PULL casei
   casei = (casei == 'Y')

   /* Now, replace... */
   temp = "TempFile"
   CALL Charout temp, '', 1
   cnt = 0;
   DO Lines(file)-1
      CALL Lineout temp, ReplaceString(Linein(file), from, to, casei, "cnt")
      END
   IF (cnt = 0) THEN DO
      SAY "Nothing was changed!"
      END
   ELSE DO
      SAY cnt "replacements pending: do you confirm (Y / N)"
      PULL answer
      IF answer = 'Y' THEN DO
         CALL Charout file, '', 1
         DO Lines(temp)
            CALL lineout file, linein(temp)
            END
         END
      END
   CALL Charout temp, '', 1   /* Remove the temporary file */
