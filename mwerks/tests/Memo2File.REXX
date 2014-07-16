/* Memo2File
Read a memo and write it to a file.
*/
Memo2File:
input = "dm:///RexxDB/Personal/2"
output = "file:///MemoPers2"
CALL Charout output, '', 1  /* technique to reset output */
linesCount = Lines(input)
SAY input "has" Chars(input) "bytes in" linesCount "lines."
DO linesCount-1             /* forget the sentinell! */
   CALL Lineout output, Linein(input)
   END
