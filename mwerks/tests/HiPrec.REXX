/* HighPrec
High Precision Calculator
*/
numeric digits 40 /* 40 digits of prec */
Say "Precision set to 40 digits."
signal on syntax name tooBad
arg = arg(1)
again:
  interpret "res=" arg "+0"  /* make sure it's valid */
  Say res
  return
redo:
  Say "Please, re-enter"
  arg = linein()
  if arg == '' then return; else signal again


tooBad:
  Say "This is not a valid equation!"
  signal redo

