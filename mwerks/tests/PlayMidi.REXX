/* PlayMidi
Play a Standard Midi FIle (SMF) out of a MIDI sound DB
*/
PlayMidi:
PARSE ARG dbName songNumber
if dbName == "" THEN DO
  SAY "Name the MIDI Database: (default System MIDI Sounds)"
  PARSE PULL dbName rest
END
IF dbName == "" THEN dbName = "System MIDI Sounds"
if songNumber == "" THEN DO
  SAY "What song number? (default gives a list)"
  PARSE PULL songNumber rest
END

IF songNumber <> "" THEN DO
   maxSong = 1
   input = "dm:///" || dbName || "//" || songNumber
   IF Stream(input) == "READY" THEN maxSong = songNumber + 1
   CALL PlayASong
   END
ELSE DO
   /* Collect titles */
   DO maxSong=1
      input = "dm:///" || dbName || "//" || maxSong
      IF Stream(input) \= "READY" THEN LEAVE
      SAY maxSong "->" GetTitle()
      END

   DO UNTIL answer <> 'Y'
      SAY "Please, choose a title number:"
      PULL songNumber
      CALL PlayASong
      SAY "Play another? (Y/N)"
      PULL answer
      END
   END

RETURN

PlayASong: PROCEDURE EXPOSE dbName songNumber input
   IF Datatype(songNumber) <> 'NUM' |,
      songNumber < 1 | songNumber >= maxSong THEN DO
      SAY songNumber "is not valid."
      EXIT
      END
   input = "dm:///" || dbName || "//" || songNumber
   SAY "Now playing..." GetTitle()
   CALL Charin input, 1+C2d(Charin(input,5)), 0  /* start of the Midi rec */
   CALL Charout "midi:", Charin(input,,Chars(input)) /* play it */
   RETURN

GetTitle: PROCEDURE EXPOSE input
   IF Charin(Input, 1, 4) <> "PMrc" THEN DO
      SAY dbName "doesn't appear to be a Midi DataBase."
      EXIT
      END
   CALL Charin input, 7, 0
   DO titleLen = 0 WHILE Charin(input) \= '00'x; END
   RETURN Charin(input, 7, titleLen)

