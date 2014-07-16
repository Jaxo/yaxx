/* Talk2Serial
Simple demo of the serial (aka rs232) stream
*/
Talk2Serial:
   serial = "serial:?bauds=19200"
   IF Stream(serial) \= "READY" THEN DO
      SAY "Cannot access the Serial port"
      EXIT
      END
   CALL Lineout serial, "Hello Serial! Say something..."
   SAY "Serial said:" Linein(serial)
   exit
