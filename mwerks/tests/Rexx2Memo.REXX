/* Rexx2Memo
Read the entire Rexx database into the Memo database.
*/

Rexx2Memo:
output = "dm:///MemoDB/REXX"
DO i=1
   input = "dm:///RexxDB/REXX/" || i
   IF Stream(input) \= 'READY' THEN LEAVE /* all done! */
   DO Lines(input)-1
      CALL Lineout output, Linein(input)
      END
   CALL Charout output, LineIn(input)     /* write the last line */
   CALL Stream output, 'C', 'CLOSE'       /* done with this record */
   END
