/* t7: */
Say "Test a syntax trap hitted during the interpretation phase of interpret."
Say "It should produce:"
Say 'Error toto 16  at line 9: Label not found'
Say "               in: interpret 'signal foo'"
Say '--------------------------------------------------------------------------'

signal on syntax
data='*Y'
interpret 'signal foo'
near.1=a
near.2=b
near.3=c
mod=2
say near.mod
return

syntax:
  say 'Error toto' rc ' at line' sigl':' errortext(rc)
  say '               in:' sourceline(sigl)
