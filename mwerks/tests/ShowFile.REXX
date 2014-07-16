/* ShowFile
Display a File
*/
ShowFile:
input = "file:///TempFile"
DO Lines(input)
  SAY Linein(input)
  END
