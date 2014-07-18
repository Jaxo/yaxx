/* a2: */
trace ?A
Say 'Console input: "trace Wrong"'
Say 'Console input: "signal on syntax; syntaxerror=5*foo"'
Say 'Console input: "signal syntax"'
Say 'Console input: "call syntax"'
Say 'Console input: "trace R"'
foo=bar//foo
interpret 'signal on syntax'
data='*Y'
interpret 'signal foo'
near.1=a
near.2=b
near.3=c
mod=2
say near.mod
exit

syntax:
  say 'My Syntax handler found error' rc ' at line' sigl':' errortext(rc),
      'in:' sourceline(sigl)
  return
