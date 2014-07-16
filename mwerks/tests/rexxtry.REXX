/* RexxTry

This procedure lets you interactively try REXX statements.
Owned by IBM SAA REXX Development, Endicott, New York.
Loosely derived from an ancient formulation of Mike Cowlishaw.
Modified to fit in a Palm MemoPad!
*/
parse arg argrx
 call house
 select
  when argrx = '?' then call tell
  when argrx = '' then do
 call intro ; call main ; end
  otherwise push argrx ; call main
 end
done: exit

house:
 parse version version
 parse source source
 parse source sysrx . procrx .
 remindrx = "Enter 'exit' to end."
 helprx=''
 promptrx=''
 if argrx<>'' then promptrx=procrx' '
 temprx = ' 'procrx' on 'sysrx
 posrx = 49-length(temprx)
 bordrx = copies('.',48)
 bordrx =,
 overlay(temprx,bordrx,posrx)
 save = ''
 trace = 'Off'
 return result

tell: call clear ; do irx = 1 to 7
 say sourceline(irx) ; end
 return result

clear:
 say
 return result

intro:
 say ' 'procrx' lets you',
  'interactively try REXX',
  'statements.'
 say '  Each string is executed',
  'when you hit Enter.'
 say " Enter 'call tell' for",
  "a description of the features."
 say ' Go on - try a few...  ',
  ' 'remindrx
 return result

sub: say ' ...test subroutine',
 "'sub' ...returning 1234..."
 return 1234

main: signal on syntax
 do foreverrx = 1
  prev = inputrx
  parse pull inputrx
  current = inputrx
  if save <> '' then call save
  if inputrx = '=' then inputrx=prev
  select
 when inputrx = '' then say ' ',
  procrx': 'remindrx helprx
 when inputrx='?' then call help
 otherwise
  rc = 'X'
  call set2 ; trace (trace)
  interpret inputrx
  trace 'Off'
  call border
  end
  if argrx <> '' & queued() = 0
 then leave
 end ; return result

set1: siglrx1 = sigl
 return result

set2: siglrx2 = sigl
 return result

save:
 call set1;rcrx=lineout(save,inputrx)
 if rcrx <> 0 then
  say " Error on save="save
 return result

help: say ' 'sysrx' has',
  'no online help for REXX.'
  rc = 'Sorry !' ;
 call border ; return result

border: if rc = 'X' then
  say ' 'bordrx
 else say ' ',
  overlay('rc = 'rc' ',bordrx)
 return result

syntax: trace 'Off'
 select
  when sigl = siglrx1 then do
 say " Invalid 'save' value",
  "'"save"', resetting to ''."
 save='' ; end
  when sigl = siglrx2 then do
 say " Invalid 'trace' value",
  "'"trace"', resetting to",
  "'Off'." ; trace='Off' ; end
  otherwise
 say ' Oooops ! ... try',
 'again. 'errortext(rc)
 end ; call border
 if argrx <> '' & queued() = 0 then
  signal done
 signal main

show: trace 'Off' ; call clear
 say ' 'procrx' provides',
  'these user variables.'
 say ' The current values are...'
 say
 say "  'version'  = '"version"'"
 say "  'source'  = '"source"'"
 say "  'result'  = '"result"'"
 say
 say ' Previous line entered by',
  'user. Initial value=INPUTRX.'
 say "  'prev' = '"prev"'"
 say "  'current'  = '"current"'"
 say
 say ' Save your input with',
  'save=filespec.',
  " Stop saving with save=''."
 say "  'save' = '"save"'"
 say
 say ' Enter trace=i, trace=o',
  ' etc. to control tracing.'
 say "  'trace' = '"trace"'"
 return result
