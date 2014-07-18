/* r: */
signal on syntax
say "please, enter one of the following:"
say "interpret 'trace ??'"
say "interpret 'trace I; say """Hi"""'"
say 'interpret '"'"'signal on syntax; say  a*"foo"; syntax: say "Error, your honor"'"'"
say "a=a * 'foo'"
trace O
interpret 'trace A'
trace ?r
where = 'the beach.'
say 'Going to' where
call goto.somewhere
say 'Back from' where
say 'Going to nowhere'
call goto.nowhere
say 'Back from' where
YOYO
say 'After YOYO'
signal on syntax
call foo
say 'After foo'
return;

foo:
  a ='Hi'
  b = a * 5;
  say b;

goto.somewhere:
goto.nowhere: procedure
   say ' Here I am'
   where = "nowhere."
   return

syntax:
  say 'Error toto' rc ' at line' sigl':' errortext(rc)
  say '               in:' sourceline(sigl)
  call foo
  return;
