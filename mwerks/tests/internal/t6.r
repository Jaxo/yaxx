/* t6: */
Say "Test a syntax error during the interpretation phase of interpret."
Say "It should produce:"
Say 'Error toto 16  at line 9: Label not found'
Say "               in: interpret 'signal foo'"
Say '--------------------------------------------------------------------------'

data='*Y'
interpret 'signal foo'
near.1=a
near.2=b
near.3=c
mod=2
say near.mod
return
