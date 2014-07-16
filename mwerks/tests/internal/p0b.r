/* p0b: */
/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992  Anders Christensen <anders@solan.unit.no>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Tests all builtin functions of rexx, except from:
 *
 *   linein, linout, lines, charin, charout, chars
 *     --> tested in fileio.rexx
 *
 *   queued
 *     --> tested in stack.rexx
 *
 *   random
 *     --> tested in trip.rexx
 */


/* trace ?int */
written = 0
options FIND_BIF
parse version intr ver .
parse source os .
If Substr(intr,1,11) = 'REXX-Regina' Then intr = 'regina'
If Substr(intr,1,14) = 'REXX:Open-REXX' Then intr = 'unirexx'
If Substr(intr,1,8) = 'REXX/imc' Then intr = 'rexximc'


/* ======================== MAX ============================== */
call notify 'max'
   call ch max( 10.1 ),               '10.1'
   call ch max( -10.1, 3.8 ),         '3.8'
   call ch max( 10.1, 10.2, 10.3 ),   '10.3'
   call ch max( 10.3, 10.2, 10.3 ),   '10.3'
   call ch max( 10.1, 10.2, 10.3 ),   '10.3'
   call ch max( 10.1, 10.4, 10.3 ),   '10.4'
   call ch max( 10.3, 10.2, 10.1 ),   '10.3'
   call ch max( 1, 2, 4, 5 ),         '5'
   call ch max( -0, 0 ),              '0'
   call ch max( 1,2,3,4,5,6,7,8,7,6,5,4,3,2 ),   '8'


/* ======================== MIN ============================== */
call notify 'min'
   call ch min( 10.1 ),               '10.1'
   call ch min( -10.1, 3.8 ),         '-10.1'
   call ch min( 10.1, 10.2, 10.3 ),   '10.1'
   call ch min( 10.1, 10.2, 10.1 ),   '10.1'
   call ch min( 10.1, 10.2, 10.3 ),   '10.1'
   call ch min( 10.4, 10.1, 10.3 ),   '10.1'
   call ch min( 10.3, 10.2, 10.1 ),   '10.1'
   call ch min( 5, 2, 4, 1 ),         '1'
   call ch min( -0, 0 ),              '0'
   call ch min( 8,2,3,4,5,6,7,1,7,6,5,4,3,2 ),   '1'

/* ====================== OVERLAY ============================== */
call notify 'overlay'
   call ch overlay('foo', 'abcdefghi',3,4,'*'), 'abfoo*ghi'
   call ch overlay('foo', 'abcdefghi',3,2,'*'), 'abfoefghi'
   call ch overlay('foo', 'abcdefghi',3,4,), 'abfoo ghi'
   call ch overlay('foo', 'abcdefghi',3), 'abfoofghi'
   call ch overlay('foo', 'abcdefghi',,4,'*'), 'foo*efghi'
   call ch overlay('foo', 'abcdefghi',9,4,'*'), 'abcdefghfoo*'
   call ch overlay('foo', 'abcdefghi',10,4,'*'), 'abcdefghifoo*'
   call ch overlay('foo', 'abcdefghi',11,4,'*'), 'abcdefghi*foo*'
   call ch overlay('', 'abcdefghi',3), 'abcdefghi'
   call ch overlay('foo', '',3), '  foo'
   call ch overlay('', '',3,4,'*'), '******'
   call ch overlay('', ''), ''

/* ======================== POS ============================== */
call notify 'pos'
   call ch pos('foo','a foo foo b'), 3
   call ch pos('foo','a foo foo',3), 3
   call ch pos('foo','a foo foo',4), 7
   call ch pos('foo','a foo foo b',30), 0
   call ch pos('foo','a foo foo b',1), 3
   call ch pos('','a foo foo b'), 0
   call ch pos('foo',''), 0
   call ch pos('',''), 0
   call ch pos('b','a'), 0
   call ch pos('b','b'), 1
   call ch pos('b','abc'), 2
   call ch pos('b','def'), 0
   call ch pos('foo','foo foo b'), 1

   if intr = 'regina' then do
      call ch index('a foo foo b','foo'), 3
      call ch index('a foo foo','foo',3), 3
      call ch index('a foo foo','foo',4), 7
      call ch index('a foo foo b','foo',30), 0
      call ch index('a foo foo b','foo',1), 3
      call ch index('a foo foo b',''), 0
      call ch index('','foo'), 0
      call ch index('',''), 0
      call ch index('a','b'), 0
      call ch index('b','b'), 1
      call ch index('abc','b'), 2
      call ch index('def','b'), 0
      call ch index('foo foo b','foo'), 1
   end

/* ====================== REVERSE ============================== */
call notify 'reverse'
   call ch reverse('foobar'),    'raboof'
   call ch reverse(''),          ''
   call ch reverse('fubar'),     'rabuf'
   call ch reverse('f'),         'f'
   call ch reverse('  foobar '), ' raboof  '



/* ======================= RIGHT ============================== */
call notify 'right'
   call ch right('',4),           '    '
   call ch right('foobar',0),     ''
   call ch right('foobar',3),     'bar'
   call ch right('foobar',6),     'foobar'
   call ch right('foobar',8),     '  foobar'
   call ch right('foobar',8,'*'), '**foobar'
   call ch right('foobar',4,'*'), 'obar'



/* ======================== SIGN ============================== */
call notify 'sign'
   call ch sign('0'),    0
   call ch sign('-0'),   0
   call ch sign('0.4'),  1
   call ch sign('-10'), -1
   call ch sign('15'),   1



/* ===================== SOURCELINE ============================== */
call notify 'sourceline'
   parse source . . srcfile .
   call ch sourceline(),   lines(srcfile,'C')   /* don't work for stdin */
   call ch sourceline(whatline()-2), '   parse source . . srcfile .'
   call ch sourceline(whatline()-4), "call notify 'sourceline'"

   signal aftersrc
   whatline: return sigl
aftersrc:


/* ======================== SPACE ============================== */
call notify 'space'
   call ch space(' foo ')               ,'foo'
   call ch space('  foo')               ,'foo'
   call ch space('foo  ')               ,'foo'
   call ch space('  foo  ')             ,'foo'
   call ch space(' foo bar ')           ,'foo bar'
   call ch space('  foo  bar  ')        ,'foo bar'
   call ch space(' foo bar ',2)         ,'foo  bar'
   call ch space(' foo bar ',,'-')      ,'foo-bar'
   call ch space('  foo  bar  ',2,'-')  ,'foo--bar'
   call ch space(' f-- b-- ',2,'-')     ,'f----b--'
   call ch space(' f o o   b a r ',0)   ,'foobar'


/* ======================= STRIP ============================== */
call notify 'strip'
   call ch strip('  foo   bar   '),          'foo   bar'
   call ch strip('  foo   bar   ','L'),      'foo   bar   '
   call ch strip('  foo   bar   ','T'),      '  foo   bar'
   call ch strip('  foo   bar   ','B'),      'foo   bar'
   call ch strip('  foo   bar   ','B','*'),  '  foo   bar   '
   call ch strip('  foo   bar',,'r'),        '  foo   ba'
   call ch strip('  foo' ),         'foo'




/* ======================= SUBSTR ============================== */
call notify 'substr'
   call ch substr('foobar',2,3),  'oob'
   call ch substr('foobar',3),  'obar'
   call ch substr('foobar',3,6),  'obar  '
   call ch substr('foobar',3,6,'*'),  'obar**'
   call ch substr('foobar',6,3),  'r  '
   call ch substr('foobar',8,3),  '   '



/* ======================= SUBWORD ============================== */
call notify 'subword'
   call ch subword(' to be or not to be ',5),  'to be'
   call ch subword(' to be or not to be ',6),  'be'
   call ch subword(' to be or not to be ',7),  ''
   call ch subword(' to be or not to be ',8,7),  ''
   call ch subword(' to be or not to be ',6,0),  ''
   call ch subword(' to be or not to be ',3,2),  'or not'
   call ch subword(' to be or not to be ',1,2),  'to be'
   call ch subword(' to be or not   to be ',4,2),  'not   to'
   call ch subword('abc de f', 3), 'f'



/* ======================= SYMBOL ============================== */

call notify 'symbol'
   parse value 'foobar' with alpha 1 beta 1 omega 1 gamma.foobar
   omega = 'FOOBAR'
   call ch symbol('HEPP'),         'LIT'
   call ch symbol('ALPHA'),        'VAR'
   call ch symbol('Un*x'),         'BAD'
   call ch symbol('gamma.delta'),  'LIT'
   call ch symbol('gamma.FOOBAR'), 'VAR'
   call ch symbol('gamma.alpha'),  'LIT'
   call ch symbol('gamma.omega'),  'VAR'
   call ch symbol('gamma.Un*x'),   'BAD'
   call ch symbol('Un*x.gamma'),   'BAD'
   call ch symbol('!!'),           'LIT'
   call ch symbol(''),             'BAD'
   call ch symbol('00'x),          'BAD'
   call ch symbol('foo-bar'),      'BAD'


/* ======================= TRACE ============================== */


/* ====================== TRANSLATE ============================== */
call notify 'translate'
   call ch translate('Foo Bar'), 'FOO BAR'
   call ch translate('Foo Bar',,''), 'Foo Bar'
   call ch translate('Foo Bar','',), '       '
   call ch translate('Foo Bar','',,'*'), '*******'
   call ch translate('Foo Bar',xrange('01'x,'ff'x)), 'Gpp!Cbs'
   call ch translate('','klasjdf','woieruw'), ''
   call ch translate('foobar','abcdef','fedcba'), 'aooefr'



/* ======================= TRUNC ============================== */
call notify 'trunc'
   call ch trunc(1234.5678, 2), '1234.56'
   call ch trunc(-1234.5678), '-1234'
   call ch trunc(.5678), '0'
   call ch trunc(.00123), '0'
   call ch trunc(.00123,4), '0.0012'
   call ch trunc(.00127,4), '0.0012'
   call ch trunc(.1678), '0'
   call ch trunc(1234.5678), '1234'
   call ch trunc(4.5678, 7), '4.5678000'

   call ch trunc(10000005.0,2), 10000005.00
   call ch trunc(10000000.5,2), 10000000.50
/*   call ch trunc(10000000.05,2), 10000000.10
   call ch trunc(10000000.005,2), 10000000.00 */

   call ch trunc(10000005.5,2), 10000005.50
   call ch trunc(10000000.55,2), 10000000.60
   call ch trunc(10000000.055,2), 10000000.10
   call ch trunc(10000000.0055,2), 10000000.00

   call ch trunc(10000000.04,2), 10000000.00
   call ch trunc(10000000.045,2), 10000000.00
   call ch trunc(10000000.45,2), 10000000.50

   call ch trunc(10000000.05,2), 10000000.10
   call ch trunc(10000000.05,2), 10000000.10
   call ch trunc(10000000.05,2), 10000000.10

   call ch trunc(99999999.,2), 99999999.00
   call ch trunc(99999999.9,2), 99999999.90
   call ch trunc(99999999.99,2), 100000000.00

   call ch trunc(1E2,0), 100
   call ch trunc(12E1,0), 120
   call ch trunc(123.,0), 123
   call ch trunc(123.1,0), 123
   call ch trunc(123.12,0), 123
   call ch trunc(123.123,0), 123
   call ch trunc(123.1234,0), 123
   call ch trunc(123.12345,0), 123

   call ch trunc(1E2,1), 100.0
   call ch trunc(12E1,1), 120.0
   call ch trunc(123.,1), 123.0
   call ch trunc(123.1,1), 123.1
   call ch trunc(123.12,1), 123.1
   call ch trunc(123.123,1), 123.1
   call ch trunc(123.1234,1), 123.1
   call ch trunc(123.12345,1), 123.1

   call ch trunc(1E2,2), 100.00
   call ch trunc(12E1,2), 120.00
   call ch trunc(123.,2), 123.00
   call ch trunc(123.1,2), 123.10
   call ch trunc(123.12,2), 123.12
   call ch trunc(123.123,2), 123.12
   call ch trunc(123.1234,2), 123.12
   call ch trunc(123.12345,2), 123.12

   call ch trunc(1E2,3), 100.000
   call ch trunc(12E1,3), 120.000
   call ch trunc(123.,3), 123.000
   call ch trunc(123.1,3), 123.100
   call ch trunc(123.12,3), 123.120
   call ch trunc(123.123,3), 123.123
   call ch trunc(123.1234,3), 123.123
   call ch trunc(123.12345,3), 123.123

   call ch trunc(1E2,4), 100.0000
   call ch trunc(12E1,4), 120.0000
   call ch trunc(123.,4), 123.0000
   call ch trunc(123.1,4), 123.1000
   call ch trunc(123.12,4), 123.1200
   call ch trunc(123.123,4), 123.1230
   call ch trunc(123.1234,4), 123.1234
   call ch trunc(123.12345,4), 123.1234

   call ch trunc(1E2,5), 100.00000
   call ch trunc(12E1,5), 120.00000
   call ch trunc(123.,5), 123.00000
   call ch trunc(123.1,5), 123.10000
   call ch trunc(123.12,5), 123.12000
   call ch trunc(123.123,5), 123.12300
   call ch trunc(123.1234,5), 123.12340
   call ch trunc(123.12345,5), 123.12345



/* ======================= VALUE ============================== */
call notify 'value'
   x.a = 'asdf'
   x.b = 'foo'
   x.c = 'A'
   a = 'B'
   b = 'C'
   c = 'A'

   call ch value('a'), 'B'
   call ch value(a), 'C'
   call ch value(c), 'B'
   call ch value('c'), 'A'
   call ch value('x.A'), 'foo'
   call ch value(x.B), 'B'
   call ch value('x.B'), 'A'
   call ch value('x.'||a), 'A'
   call ch value(value(x.b)), 'C'

   xyzzy = 'foo'
   call ch value('xyzzy'),       'foo'
   call ch value('xyzzy','bar'), 'foo'
   call ch value('xyzzy'),       'bar'
   call ch value('xyzzy','bar'), 'bar'
   call ch value('xyzzy'),       'bar'
   call ch value('xyzzy','foo'), 'bar'
   call ch value('xyzzy'),       'foo'

   xyzzy = 'void'
   if os = 'UNIX' | os = 'AMIGA' Then
      envvar = '$xyzzy'
   else
      envvar = '%xyzzy%'
/*
   call value 'xyzzy', 'bar', 'ENVIRONMENT'
   call ch value('xyzzy', 'bar', 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy',,       'ENVIRONMENT'), 'bar'
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'echo'(envvar)
   call ch value('xyzzy', 'foo', 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy', 'bar', 'ENVIRONMENT'), 'foo'
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'echo'(envvar)
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy', 'foo', 'ENVIRONMENT'), 'bar'
   call ch value('xyzzy',      , 'ENVIRONMENT'), 'echo'(envvar)
*/

/* ======================= VERIFY ============================== */
call notify 'verify'
   call ch verify('foobar', 'barfo', N, 1), 0
   call ch verify('foobar', 'barfo', M, 1), 1
   call ch verify('', 'barfo'), 0
   call ch verify('foobar', ''), 1
   call ch verify('foobar', 'barf', N, 3), 3
   call ch verify('foobar', 'barf', N, 4), 0
   call ch verify('', ''), 0



/* ======================== WORD ============================== */
call notify 'word'
   call ch word('This is certainly a test',1), 'This'
   call ch word('   This is certainly a test',1), 'This'
   call ch word('This    is certainly a test',1), 'This'
   call ch word('This    is certainly a test',2), 'is'
   call ch word('This is    certainly a test',2), 'is'
   call ch word('This is certainly a    test',5), 'test'
   call ch word('This is certainly a test   ',5), 'test'
   call ch word('This is certainly a test',6), ''
   call ch word('',1), ''
   call ch word('',10), ''
   call ch word('test ',2), ''



/* ====================== WORDINDEX ============================== */
call notify 'wordindex'
   call ch wordindex('This is certainly a test',1), '1'
   call ch wordindex('  This is certainly a test',1), '3'
   call ch wordindex('This   is certainly a test',1), '1'
   call ch wordindex('  This   is certainly a test',1), '3'
   call ch wordindex('This is certainly a test',2), '6'
   call ch wordindex('This   is certainly a test',2), '8'
   call ch wordindex('This is   certainly a test',2), '6'
   call ch wordindex('This   is   certainly a test',2), '8'
   call ch wordindex('This is certainly a test',5), '21'
   call ch wordindex('This is certainly a   test',5), '23'
   call ch wordindex('This is certainly a test  ',5), '21'
   call ch wordindex('This is certainly a test  ',6), '0'
   call ch wordindex('This is certainly a test',6), '0'
   call ch wordindex('This is certainly a test',7), '0'
   call ch wordindex('This is certainly a test  ',7), '0'


/* ===================== WORDLENGTH ============================== */
call notify 'wordlength'
   call ch wordlength('This is certainly a test',1), '4'
   call ch wordlength('This   is   certainly a test',2), '2'
   call ch wordlength('This is certainly a test',5), '4'
   call ch wordlength('This is certainly a test ',5), '4'
   call ch wordlength('This is certainly a test',6), '0'
   call ch wordlength('',1), '0'
   call ch wordlength('',10), '0'


/* ====================== WORDPOS ============================== */
call notify 'wordpos'
   call ch wordpos('This','This is a small test'), 1
   call ch wordpos('test','This is a small test'), 5
   call ch wordpos('foo','This is a small test'), 0
   call ch wordpos('  This  ','This is a small test'), 1
   call ch wordpos('This','  This is a small test'), 1
   call ch wordpos('This','This   is a small test'), 1
   call ch wordpos('This','this is a small This'), 5
   call ch wordpos('This','This is a small This'), 1
   call ch wordpos('This','This is a small This',2), 5
   call ch wordpos('is a ','This  is a small test'), 2
   call ch wordpos('is   a ','This  is a small test'), 2
   call ch wordpos('  is a ','This  is  a small test'), 2
   call ch wordpos('is a ','This  is a small test',2), 2
   call ch wordpos('is a ','This  is a small test',3), 0
   call ch wordpos('is a ','This  is a small test',4), 0
   call ch wordpos('test  ','This  is a small test'), 5
   call ch wordpos('test  ','This  is a small test',5), 5
   call ch wordpos('test  ','This  is a small test',6), 0
   call ch wordpos('test  ','This  is a small test   '), 5
   call ch wordpos('  test','This  is a small test   ',6), 0
   call ch wordpos('test  ','This  is a small test   ',5), 5
   call ch wordpos('      ','This  is a small test'), 0
   call ch wordpos('      ','This  is a small test',3), 0
   call ch wordpos('','This  is a small test',4), 0
   call ch wordpos('test  ',''), 0
   call ch wordpos('',''), 0
   call ch wordpos('','  '), 0
   call ch wordpos('  ',''), 0
   call ch wordpos('  ','',3), 0
   call ch wordpos(' a ',''), 0
   call ch wordpos(' a ','a'), 1

/*
   call ch find('This is a small test','This'), 1
   call ch find('This is a small test','test'), 5
   call ch find('This is a small test','foo'), 0
   call ch find('This is a small test','  This  '), 1
   call ch find('  This is a small test','This'), 1
   call ch find('This   is a small test','This'), 1
   call ch find('this is a small This','This'), 5
   call ch find('This is a small This','This'), 1
   call ch find('This is a small This','This',2), 5
   call ch find('This  is a small test','is a '), 2
   call ch find('This  is a small test','is   a '), 2
   call ch find('This  is  a small test','  is a '), 2
   call ch find('This  is a small test','is a ',2), 2
   call ch find('This  is a small test','is a ',3), 0
   call ch find('This  is a small test','is a ',4), 0
   call ch find('This  is a small test','test  '), 5
   call ch find('This  is a small test','test  ',5), 5
   call ch find('This  is a small test','test  ',6), 0
   call ch find('This  is a small test   ','test  '), 5
   call ch find('This  is a small test   ','  test',6), 0
   call ch find('This  is a small test   ','test  ',5), 5
   call ch find('This  is a small test','      '), 0
   call ch find('This  is a small test','      ',3), 0
   call ch find('This  is a small test','',4), 0
   call ch find('','test  '), 0
   call ch find('',''), 0
   call ch find('  ',''), 0
   call ch find('','  '), 0
   call ch find('','  ',3), 0
   call ch find('',' a '), 0
   call ch find('a',' a '), 1
*/

/* ======================= WORDS ============================== */
call notify 'words'
   call ch words('This is certainly a test'), 5
   call ch words('   This is certainly a test'), 5
   call ch words('This    is certainly a test'), 5
   call ch words('This is certainly a test   '), 5
   call ch words('  hepp '), 1
   call ch words('  hepp        hepp   '), 2
   call ch words(''), 0
   call ch words('   '), 0


/* ======================= XRANGE ============================== */
call notify 'xrange'
   call ch xrange('f','r'), 'fghijklmnopqr'
   call ch xrange('7d'x,'83'x), '7d7e7f80818283'x
   call ch xrange('a','a'), 'a'



/* ======================== X2B ============================== */
   call ch x2b('416263'), '010000010110001001100011'
   call ch x2b('DeadBeef'), '11011110101011011011111011101111'
   call ch x2b('1 02 03'), '00010000001000000011'
   call ch x2b('102 03'), '00010000001000000011'
   call ch x2b('102'), '000100000010'
   call ch x2b('11 2F'), '0001000100101111'
   call ch x2b(''), ''




/* ======================== X2C ============================== */
call notify 'x2c'
   call ch x2c('416263'), 'Abc'
   call ch x2c('DeadBeef'), 'deadbeef'x
   call ch x2c('1 02 03'), '010203'x
   call ch x2c('11 0222 3333 044444'), '1102223333044444'x
   call ch x2c(''), ''
   call ch x2c('2'), '02'x
   call ch x2c('1   02   03'), '010203'x



/* ======================== X2D ============================== */
call notify 'x2d'
   call ch x2d( 'ff80', 2), '-128'
   call ch x2d( 'ff80', 1), '0'
   call ch x2d( 'ff 80', 1), '0'
   call ch x2d( '' ),      '0'
   call ch x2d( '101' ),  '257'
   call ch x2d( 'ff' ),   '255'
   call ch x2d( 'ffff'),  '65535'

   call ch x2d( 'ffff', 2), '-1'
   call ch x2d( 'ffff', 1), '-1'
   call ch x2d( 'fffe', 2), '-2'
   call ch x2d( 'fffe', 1), '-2'
   call ch x2d( 'ffff', 4), '-1'
   call ch x2d( 'ffff', 2), '-1'
   call ch x2d( 'fffe', 4), '-2'
   call ch x2d( 'fffe', 2), '-2'

   call ch x2d( 'ffff', 3), '-1'
   call ch x2d( '0fff'), '4095'
   call ch x2d( '0fff', 4), '4095'
   call ch x2d( '0fff', 3), '-1'
   call ch x2d( '07ff'), '2047'
   call ch x2d( '07ff', 4), '2047'
   call ch x2d( '07ff', 3), '2047'

   call ch x2d( 'ff7f', 1), '-1'
   call ch x2d( 'ff7f', 2), '127'
   call ch x2d( 'ff7f', 3), '-129'
   call ch x2d( 'ff7f', 4), '-129'
   call ch x2d( 'ff7f', 5), '65407'

   call ch x2d( 'ff80', 1), '0'
   call ch x2d( 'ff80', 2), '-128'
   call ch x2d( 'ff80', 3), '-128'
   call ch x2d( 'ff80', 4), '-128'
   call ch x2d( 'ff80', 5), '65408'

   call ch x2d( 'ff81', 1), '1'
   call ch x2d( 'ff81', 2), '-127'
   call ch x2d( 'ff81', 3), '-127'
   call ch x2d( 'ff81', 4), '-127'
   call ch x2d( 'ff81', 5), '65409'

   call ch x2d( 'ffffffffffff', 12), '-1'

/* ======================= JUSTIFY ============================= */
   if intr = 'regina' Then Do
      call notify 'justify'
      call ch justify('Dette er en test',20,'-'), 'Dette--er---en--test'
      call ch justify('Dette er en test',10,'-'), 'Dette-er-e'

      call ch justify('  Dette er en test',25), 'Dette    er    en    test'
      call ch justify('Dette   er en test',24), 'Dette    er    en   test'
      call ch justify('Dette er   en test',23), 'Dette   er    en   test'
      call ch justify('Dette er en   test',22), 'Dette   er   en   test'
      call ch justify('Dette er en test  ',21), 'Dette   er   en  test'
      call ch justify('  Dette er en test',20), 'Dette  er   en  test'
      call ch justify('Dette   er en test',19), 'Dette  er  en  test'
      call ch justify('Dette er   en test',18), 'Dette  er  en test'
      call ch justify('Dette er en   test',17), 'Dette er  en test'
      call ch justify('Dette er en test  ',16), 'Dette er en test'
      call ch justify('  Dette er en test',15), 'Dette er en tes'
      call ch justify('Dette   er en test',14), 'Dette er en te'
      call ch justify('Dette er   en test',13), 'Dette er en t'
      call ch justify('Dette er en   test',12), 'Dette er en '
      call ch justify('Dette er en test  ',11), 'Dette er en'
      call ch justify('Dette er en test',10), 'Dette er e'

      call ch justify('Dette er en test',0), ''
      call ch justify('foo',10), 'foo       '
      call ch justify('',10), '          '
   End

   say ' '

exit 0


ch: procedure expose sigl
   parse arg first, second
   if first \== second then do
      say
      say 'FuncTrip: error in ' sigl':' sourceline(sigl)
      say '          expecting: /'second'/, found: /'first'/'
      say
   end
   return


notify:
   parse arg word .
   written = written + length(word) + 2
   if written>75 then do
      written = length(word)
      say ' '
   end
   call charout , word || ', '
   return


error:
   say 'Error discovered in function insert()'
   return
