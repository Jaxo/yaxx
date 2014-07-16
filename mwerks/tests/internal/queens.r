call time 'R'
nq=10
if arg(1)<>'' then nq=arg(1)
n=0; i=1; sym='0123456789ABCDEFGHIJKLMNOPQ'; end=0; d.=0; d.1=1
do nn=1 by 1 while end<>1
  further=0
  do j=1 to i-1
    if d.i=d.j | abs(d.i-d.j)=i-j then leave
    end
  if j=i then do
    if i=nq then do
      n=n+1
      end
    else do
      i=i+1
      d.i=1
      further=1
      end
    end
  if further=0 then do
    do i=i by -1 while(d.i=nq)
      end
    if i<1 then end=1
    d.i=d.i+1
    end
  end
  elapsed = time('E')
say 'time elapsed:' elapsed'. found' n 'solutions'
exit 0
