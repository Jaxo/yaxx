/* ReplaceString
Ex: result = ReplaceString(myString, replaced, replacement, caseInsensitive)
    a 5th argument may follow which is the name of a variable
    in which the actual change count will be cumulated.
*/
ReplaceString:
   PARSE ARG source, from, to, casei, countnm
   IF (casei == 1) THEN DO
      from = Translate(from)
      haystack = Translate(source)
   end; else do
      haystack = source
   end

   fromLength = Length(from)
   target = ''
   beg = 1

   DO count=0
      cur = Pos(from, haystack, beg)
      IF (cur=0) THEN DO
         IF (countnm <> "") THEN INTERPRET countnm '=' countnm '+' count
         RETURN target || Substr(source, beg)
         END
      target = target || Substr(source, beg, cur - beg) || to
      beg = cur+fromLength
      END
