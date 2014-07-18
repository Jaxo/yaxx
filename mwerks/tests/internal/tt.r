/* tt: */
signal on syntax
say 'Hello'
call foo;
return

foo:
  a ='Hi'
  b = a * 5;
  say b;

syntax:
  say 'Error toto' rc ' at line' sigl':' errortext(rc)
  say '               in:' sourceline(sigl)
  return;
