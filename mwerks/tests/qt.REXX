/* QT
    Query the time in English */
/*----------------------------------------------------------*/
/* Mike Cowlishaw, December 1979 - January 1985             */
/*----------------------------------------------------------*/
/* QT. This program displays the time in natural English.   */
/* Two argument strings may be supplied. If "?" is given    */
/* as the first argument then the program displays a        */
/* description of itself.  If a second argument is supplied */
/* it is used as a test value to check the operation of the */
/* program.  This second value must be a time in the format */
/* HH:MM:SS, and does not have its syntax checked.          */
/*----------------------------------------------------------*/
qt:
parse arg parm .
select
  when parm="?" then do; call tell; exit 100; end
  when parm=""  then nop
  end

testtime = parm
if testtime='' then now=time()
               else now=testtime
near.0=''
near.1=' just gone';  near.2=' just after'
near.3=' nearly';     near.4=' almost'

parse var now hour':'min':'sec

if sec>29 then min=min+1
mod=trunc(min//5)
out="It's"near.mod
if min>32 then hour=hour+1
min=min+2

if hour//12=0 & min//60<=4 then do
  if hour=12 then say out 'Noon.'
             else say out 'Midnight.'
  exit
  end

min=min-(min//5)
if hour>12
  then hour=hour-12
  else
    if hour=0 then hour=12

select
  when min= 0 then nop
  when min=60 then min=0
  when min= 5 then out=out 'five past'
  when min=10 then out=out 'ten past'
  when min=15 then out=out 'a quarter past'
  when min=20 then out=out 'twenty past'
  when min=25 then out=out 'twenty-five past'
  when min=30 then out=out 'half past'
  when min=35 then out=out 'twenty-five to'
  when min=40 then out=out 'twenty to'
  when min=45 then out=out 'a quarter to'
  when min=50 then out=out 'ten to'
  when min=55 then out=out 'five to'
  end

numbers='one two three four five six',
  'seven eight nine ten eleven twelve'
out=out word(numbers,hour)
if min=0 then out=out "o'clock"

say out'.'
exit

Tell:
  say 'QT will display the current time in natural English.'
  say 'Call without any arguments to display the time, or with'
  say '"?" to display this information. A second argument (in'
  say 'the format "HH:MM:SS") will be used as a test value.'
  say 'British English idioms are used in this program.'
  say
  return
