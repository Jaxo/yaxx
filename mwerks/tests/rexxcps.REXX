/* RexxCPS
    Rexx Clauses Per Second Benchmark */
rexxcps=2.2
Parse Arg averaging count tracevar
if averaging = '' Then averaging = 10
if count = '' Then count = 30
if tracevar = '' then tracevar = 'Off'
signal on novalue
parse source  source  1 system .
parse version version

say '----- REXXCPS' rexxcps '-- Measuring REXX clauses/second -----'
say ' REXX version is:' version
say '       System is:' system
say '       Averaging:' averaging 'measures of' count 'iterations'

empty=0
do i=1 to averaging
call time 'R'
do count; end
empty=time('R')+empty
end
empty=empty/averaging

noterm=(system='CMS'); if pos('O',tracevar)=1 then noterm=0
if noterm then do
say 'Calibration (empty DO):' empty 'secs (average of' averaging')'
say 'Spooling trace NOTERM'
'CP SPOOL CON * START NOTERM'; 'CP CLOSE CON PUR'
end

full=0
do i=1 to averaging
trace value tracevar
call time 'R'
do count;

flag=0; p0='b'
do loop=1 to 14

key1='Key Bee'
acompound.key1.loop=substr(1234"5678",6,2)
if flag=acompound.key1.loop then say 'Failed1'
do j=1.1 to 2.2 by 1.1
if j>acompound.key1.loop then say 'Failed2'
if 17<length(j)-1        then say 'Failed3'
if j='foobar'            then say 'Failed4'
if substr(1234,1,1)=9    then say 'Failed5'
if word(key1,1)='?'      then say 'Failed6'
if j<5 then do
acompound.key1.loop=acompound.key1.loop+1
if j=2 then leave
end
iterate
end
avar.=1.0''loop
select
when flag='string' then say 'FailedS1'
when avar.flag.2=0 then say 'FailedS2'
when flag=5+99.7   then say 'FailedS3'
when flag          then avar.1.2=avar.1.2*1.1
when flag==0       then flag=0
end
if 1 then flag=1
select
when flag=='ring'  then say 'FailedT1'
when avar.flag.3=0 then say 'FailedT2'
when flag          then avar.1.2=avar.1.2*1.1
when flag==0       then flag=1
end
parse value 'Foo Bar' with v1 +5 v2 .
trace value trace(); address value address()
call subroutine 'with' 2 'args', '(This is the second)'1''1
rc='This is an awfully boring program'; parse var rc p1 (p0) p5
rc='is an awfully boring program This'; parse var rc p2 (p0) p6
rc='an awfully boring program This is'; parse var rc p3 (p0) p7
rc='awfully boring program This is an'; parse var rc p4 (p0) p8
end loop
end

full=time('R')+full
trace off
end
full=full/averaging
if noterm then do
'CP CLOSE CON'; 'CP SPOOL CON * START TERM'
say 'Spooling now back on TERM'
end

looptime=(full-empty)/count

if left(tracevar,1)='O' then nop; else do
say
say 'Total (full DO):' full-empty 'secs (average of' averaging ,
'measures of' count 'iterations)'
say 'Time for one iteration (1000 clauses) was:' looptime 'seconds'
end

if looptime = 0 Then do
say '     The granularity of the system clock appears to be too coarse to'
say '     obtain an effective result.  Re-run this progam and increase the'
say '     number of iterations or the repeat count.'
end
else do
say
say'     Performance:' format(1000/looptime,,0) 'REXX clauses per second'
say
end

exit
subroutine:
parse upper arg a1 a2 a3 ., a4
parse var a3 b1 b2 b3 .
do 1; rc=a1 a2 a3; parse var rc c1 c2 c3; end
return

novalue: if noterm then 'CP SP CON STOP TERM'
say 'novalue triggered'

