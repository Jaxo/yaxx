/* Mortgage
    Mortgage Calculator */
parse source sys . .

do forever
 call CL
 say 'Loan Amount?'
 pull v1
 if v1=''then
  do
   call CL
   exit
  end
 say 'Annual Interest Rate'
 pull v2
 say 'Duration (Years)'
 pull v3

 v2 = v2/1200
 v3 = v3*12
 v4 = (1+v2)**v3
 v5 = format(v1*v2*v4/(v4-1),8,2)
 v5 = strip(v5,'l',' ')

 say 'Payment = $'v5 'Per Month'
 say '{enter} To Continue'
 pull
end

CL:
if sys='PalmOS' then
 say d2c(27)"[2J"
else
 'cls'
return
