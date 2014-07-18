/* Talk2Beamer
Simple demo of the beamer (aka IrDA / exg) stream
*/
Talk2Beamer:
   beamer = "beamer:?bauds=57600"
   IF Stream(beamer) \= "READY" THEN DO
      SAY "Cannot access the Beamer port"
      EXIT
      END
   CALL Lineout beamer, "Hello Beamer! Say something..."
   SAY "Beamer said:" Linein(beamer)
   exit
