/* File2Memo
Read a file and write it to a memo.
*/
File2Memo:
input = "file:///MemoPers2"
output = "dm:///RexxDB/Personal/2"
linesCount = Lines(input)
SAY input "has" chars(input) "bytes in" linesCount "lines."
CALL Charout output, '', 1    /* technique to reset output */
DO linesCount
   CALL lineout output, linein(input)
   END
CALL Charout output, '00'x    /* append the sentinel */
CALL Charout input, '', 1     /* technique to delete input */


