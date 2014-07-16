/* LoadRexxDB
Load the Rexx database from the REXX category in Jaxo's MemoDB
*/
LoadRexxDB:
output = "dm://Jaxo@/RexxDB/REXX"
DO i=1
   input = "dm:///MemoDB/REXX/" || i
   IF Stream(input) \= 'READY' THEN LEAVE /* all done! */
   linesCount = Lines(input)
   IF linesCount=0 THEN ITERATE
   SAY "loading" input "(" chars(input) "bytes in" linesCount "lines.) ..."
   DO linesCount-1
      CALL Lineout output, Linein(input)
      END
   CALL Charout output, Linein(input)
   CALL Stream output, 'C', 'CLOSE' /* done with this record */
   END
