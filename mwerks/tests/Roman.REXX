/* RomanNumeral
Convert a number so Catilina would be able to read it?
"Quousque tandem, Catilina, abutere patientia nostra?"
*/
RomanNumeral:
parse arg number
if datatype(number) \== 'NUM' then Say 'Invalid Number'
else if (number > 10000) then Say 'Number too big'
else do
  lead.1 = 0;  cnt.1 = 0;
  lead.2 = 0;  cnt.2 = 1;  trail.2 = 0;
  lead.3 = 0;  cnt.3 = 2;  trail.3 = 0;
  lead.4 = 0;  cnt.4 = 1;  trail.4 = 4;
  lead.5 = 4;  cnt.5 = 0;
  lead.6 = 4;  cnt.6 = 1;  trail.6 = 0;
  lead.7 = 4;  cnt.7 = 2;  trail.7 = 0;
  lead.8 = 4;  cnt.8 = 3;  trail.8 = 0;
  lead.9 = 0;  cnt.9 = 1;  trail.9 = 8;
  symbols = "ixcmvldFxcmT"
  iNb = 1
  res=''
  do iSy = length(number) to 0 by -1
    n = substr(number,iNb,1)
    iNb = iNb+1
    if (n > 0) then do
      res = res || substr(symbols, lead.n+iSy, 1)
      do (cnt.n)
         res = res || substr(symbols, trail.n+iSy, 1)
         end
      end
    end
  Say number '=>' res
  end
