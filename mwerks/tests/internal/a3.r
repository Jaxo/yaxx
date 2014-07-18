/* a3: */
Signal on syntax
foo=bar//foo
exit

syntax:
  say 'My Syntax handler found error' rc ' at line' sigl':' errortext(rc),
      'in:' sourceline(sigl)
  foo = foo*bar
  return
