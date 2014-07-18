/* babar: */
call BADFUZZ
call FOO
say 1 ADDRESS()
SIGNAL ON SYNTAX NAME FOOBAR
NUMERIC FUZZ 30
return

FOO:
ADDRESS FooFoo
call BAR
say 2 ADDRESS()
return

BAR:
ADDRESS BarBar
call TOMTOM
say 3 ADDRESS()
return

TOMTOM:
say 4 ADDRESS()
return

BADFUZZ:
SIGNAL ON SYNTAX
NUMERIC FUZZ 30

SYNTAX:
   Say Fuzzy!
   return

FOOBAR:
   Say 'A fuzzy syntax error was found by foobar.'
   Say 'More precisely: error ' RC 'at line ' sigl':' errortext(rc)
   NUMERIC FUZZ 30
