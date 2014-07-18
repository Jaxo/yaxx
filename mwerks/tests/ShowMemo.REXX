/* ShowMemo
Display the 2nd memo of the Personal category
*/
ShowMemo:
input = "dm:///RexxDB/Personal/2"
DO Lines(input)
  SAY Linein(input)
  END
