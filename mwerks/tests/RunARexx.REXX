/* RunARexx
Look for a Rexx in the RexxDB and execute it
This requires that the Rexx to run starts with a label
that has the same value as its first word
(e.g "RunARexx:" for this script)
*/
RunARexx:
SAY "Enter the name of a Rexx Script:"
PARSE UPPER PULL title
DO i=1 BY 1
   input = "dm:///RexxDB/REXX/" || i
   IF Stream(input) \= "READY" THEN DO
      SAY "Sorry!" title "was not found."
      LEAVE
      END
   line1 = Linein(input)
   IF Translate(Word(line1,2)) = title THEN DO
      SAY "Yes: I know about " title
      SAY "Do you want it run? (Y/N)"
      PULL answer
      IF answer \= 'N' THEN DO
         CALL Load input
         INTERPRET "call " title
         END
      LEAVE
      END
   END
