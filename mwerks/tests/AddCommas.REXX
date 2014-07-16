/* AddCommas
Ex: addCommas(myNumber, minimumDigits)

Original source by Dennis Bareis
http://www.labyrinth.net.au/~dbareis
dbareis@labyrinth.net.au
*/
AddCommas:
/* Get parameters */
parse arg Value, MinDigits;
Value = strip(Value);
if  MinDigits = '' then
    MinDigits = 4;

/* Separate number into components */
parse var Value Number 1 Integer . '.' +0 Fraction .;

/* If Number does not look valid then don't touch it! */
if  length(Integer) < MinDigits |,
    0 <> verify(strip(Number),'0123456789-+.') |,
    \Datatype(Number,'Number') then
    return Value;

/* Set up some values we need for the comma addition */
MaskCondensed = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWX';
MaskSpread    = 'abc,def,ghi,jkl,mno,pqr,stu,vwx,yzA,BCD,EFG,HIJ,KLM,NOP,QRS,TUV,WX'
MaxDigits     = length(MaskCondensed);

/* Separate Sign from Digits */
Initial = substr(Integer, verify(Integer,'-+'), 1);
parse var Integer Sign (Initial) +0 Integer;
if  length(Integer) < MinDigits | length(Integer) > MaxDigits then
    return Value;

/* Format the result */
return Sign || strip(reverse(translate(MaskSpread, reverse(Integer), MaskCondensed, ',')),'L',',') || Fraction
