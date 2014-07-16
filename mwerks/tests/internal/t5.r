/* t5: */
Say "Test a BAD syntax trap hitted during the compilation phase of interpret."
Say "It should produce:"
Say "     9 +++ interpret data '=o0'"
Say 'Error 16 running "D:\u\newdev\rexx\t5.r", line 8: Label not found'
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

syntox:
  say 'Error toto' rc ' at line' sigl':' errortext(rc)
  say '               in:' sourceline(sigl)
