/* keywd: */
parse version intr ver .
If Substr(intr,1,11) = 'REXX_BNV' Then intr = 'BREXX'
a=0
if a=1 | ,        /* Hi! */
b=2 then exit

interpret "if a=0 then say 'OK'"

if intr \= BREXX then do  /* BREXX chokes! */
   interpret "do forever while(nn<5); say Bof; end"
end

if intr \= BREXX then do  /* BREXX chokes! */
   interpret "do while(a<5); a=a+1; Say 'Ha'; end"
end

/* BREXX doesn't understand! */
interpret "do nn=1 by 1 while(nn<5); if nn=8 then exit; say Hi!; end"

say (12.3 (5))
say TRUNC/**/(12.3)
say TRUNC/**/ (12.3)
say 10 //*uu*// 3
say 10 / ,
 3
say 10 / / 3
call foo

interpret "do forever(a<5); a=a+1; Say 'Ha'; end"
exit

foo,
:
say HI!
return
/*
say 10 / ,
 / 3
*/
