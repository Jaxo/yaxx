/* WhoIs
   Find and display selected name in the address book
*/
WhoIs:
PARSE UPPER ARG name
needle=Word(name,Words(name))
null='00'x
DO i=1 BY 1
   input = "dm:///AddressDB//" || i
   IF Stream(input) \= 'READY' THEN DO
      SAY "Sorry!" name "was not found."
      LEAVE
      END
   rec=Substr(Linein(input),10)
   PARSE UPPER VALUE rec WITH last '00'x first '00'x company '00'x phone '00'x email '00'x street '00'x city '00'x zip '00'x country '00'x 
   IF needle = last THEN DO  /* Name found! */
      SAY "Name:" first last
      SAY "Company:" company
      SAY "Phone:" phone
      SAY "Email:"   email
      LEAVE
      END
   END

