/* SearchARexx
Look for a Rexx in the RexxDB.
*/
SearchARexx:
input = "dm:///RexxDB/REXX/"
DO i=1 BY 1
   path = input || i
   IF Stream(path) \= 'READY' THEN DO
      SAY "Sorry! ShowMemo was not found."
      LEAVE
      END
   IF Linein(path) = "/* ShowMemo" THEN DO
      SAY "Yes: I know about ShowMemo"
      LEAVE
      END
   END
