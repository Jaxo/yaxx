/* GetARexx
Look for a Rexx in the RexxDB and copy it to the MemoDB
*/
GetARexx:
output = "dm:///MemoDB/REXX"
SAY "Enter the name of a Rexx Script:"
PARSE UPPER PULL title
DO i=1 BY 1
   input = "dm:///RexxDB/REXX/" || i
   IF Stream(input) \= 'READY' THEN DO
      SAY "Sorry!" title "was not found."
      LEAVE
      END
   line1 = Linein(input)
   IF Translate(Word(line1,2)) = title THEN DO  /* Title found! */
      CALL Lineout output, line1
      DO Lines(input) - 1
         CALL Lineout output, Linein(input)
         END
      CALL Charout output, Linein(input)        /* write the last line */
      LEAVE
      END
   END
