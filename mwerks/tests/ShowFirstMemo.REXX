/* ShowFirstMemo
Displays the first Record in the category Personal
*/
ShowFirstMemo:
input = "dm:///MemoDB/Personal"
DO Lines(input)
  SAY Linein(input)
  END
