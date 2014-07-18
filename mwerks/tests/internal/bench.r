/* bench: */
/* http://www.winikoff.net/palm/dev.html */
call time 'R'
x=10000;y=0; do while x>0; x=x-1; y=y+2; end;
elapsed = time('E')
say 'y=' y
say 'seconds: ' elapsed

