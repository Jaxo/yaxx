/* Calc
High Precision Calculator
*/
numeric digits 40 /* 40 digits of prec */
signal on syntax name tooBad
arg = arg(1)
if arg = '' then signal redo
again:
  /* if the user simply entered a series of number
    separated by spaces, just add them */
  if verify(arg, " 0123456789.()-") = 0 
    then arg = translate(arg, "+", " ")
  interpret "res=" arg "+0"  /* make sure it's valid */
  Say res
  return res
redo:
  Say "Please enter expression:"
  arg = linein()
  if arg == '' then return; else signal again

tooBad: 
  Say "This is not a valid equation:" arg
  signal redo
