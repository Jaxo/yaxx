/* t4: */
Say "Test a syntax trap hitted during the compilation phase of interpret."
Say "It should produce:"
Say "Error toto 35  at line 8: Invalid expression"
Say "               in: interpret data '=o0'"
Say '--------------------------------------------------------------------------'

signal on syntax
data='*Y'
interpret data '=o0'
near.1=a
near.2=b
near.3=c
mod=2
say near.mod
return

syntax:
  say 'Error toto' rc ' at line' sigl':' errortext(rc)
  say '               in:' sourceline(sigl)
