/* p0a: */
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

/* ======================= ABBREV ============================== */
call notify 'abbrev'
   call ch abbrev('information','info',4)               ,1
   call ch abbrev('information','',0)                   ,1
   call ch abbrev('information','Info',4)               ,0
   call ch abbrev('information','info',5)               ,0
   call ch abbrev('information','info ')                ,0
   call ch abbrev('information','info',3)               ,1
   call ch abbrev('info','information',3)               ,0
   call ch abbrev('info','info',5)                      ,0


/* ========================== ABS ============================== */
call notify 'abs'
   call ch abs(-12.345) ,12.345
   call ch abs(12.345)  ,12.345
   call ch abs(-0.0)    ,0
   call ch abs(0.0)     ,0

/* ======================== ADDRESS ============================== */
call notify 'address'
   select
      when intr = 'regina' then call ch address(), 'SYSTEM'
      when intr = 'unirexx' then call ch address(), 'CMD'
      when intr = 'rexximc' then call ch address(), 'UNIX'
      otherwise nop
   end

/* ========================== ARG ============================== */
call notify 'arg'
   call testarg2 1,,2
   call testarg1
   signal afterarg

testarg1:
   call ch arg()       ,'0'
   call ch arg(1)      ,''
   call ch arg(2)      ,''
   call ch arg(1,'e')  ,'0'
   call ch arg(1,'O')  ,'1'
   return

testarg2:
   call ch arg()      ,'3'
   call ch arg(1)     ,'1'
   call ch arg(2)     ,''
   call ch arg(3)     ,'2'
   call ch arg(4)     ,''
   call ch arg(1,'e') ,'1'
   call ch arg(2,'E') ,'0'
   call ch arg(2,'O') ,'1'
   call ch arg(3,'o') ,'0'
   call ch arg(4,'o') ,'1'
   return

afterarg:



/* ========================= B2X =============================== */
call notify 'b2x'
   call ch b2x(''), ''
   call ch b2x('0'), '0'
   call ch b2x('1'), '1'
   call ch b2x('10'), '2'
   call ch b2x('010'), '2'
   call ch b2x('1010'), 'A'

   call ch b2x('1 0101'), '15'
   call ch b2x('1 01010101'), '155'
   call ch b2x('1 0101 0101'), '155'
   call ch b2x('10101 0101'), '155'
   call ch b2x('0000 00000000 0000'), '0000'
   call ch b2x('11111111 11111111'), 'FFFF'


/* ======================= BITAND ============================== */
call notify 'bitand'
   call ch bitand( '123456'x, '3456'x ),         '101456'x
   call ch bitand( '3456'x, '123456'x, '99'x ),  '101410'x
   call ch bitand( '123456'x,, '55'x),           '101454'x
   call ch bitand( 'foobar' ),                   'foobar'
   call ch bitand( 'FooBar' ,, 'df'x),           'FOOBAR'  /* for ASCII */


/* ======================== BITOR ============================== */
call notify 'bitor'
   call ch bitor( '123456'x, '3456'x ),         '367656'x
   call ch bitor( '3456'x, '123456'x, '99'x ),  '3676df'x
   call ch bitor( '123456'x,, '55'x),           '577557'x
   call ch bitor( 'foobar' ),                   'foobar'
   call ch bitor( 'FooBar' ,, '20'x),           'foobar'  /* for ASCII */


/* ======================= BITXOR ============================== */
call notify 'bitxor'
   call ch bitxor( '123456'x, '3456'x ),         '266256'x
   call ch bitxor( '3456'x, '123456'x, '99'x ),  '2662cf'x
   call ch bitxor( '123456'x,, '55'x),           '476103'x
   call ch bitxor( 'foobar' ),                   'foobar'
   call ch bitxor( 'FooBar' ,, '20'x),           'fOObAR'  /* for ASCII */


/* ======================== C2D ============================== */
call notify 'c2d'
   call ch c2d( 'ff80'x, 1), '-128'
   call ch c2d( 'foo' ) ,  '6713199'
   call ch c2d( 'bar' ),   '6447474'
   call ch c2d( '' ),      '0'
   call ch c2d( '101'x ),  '257'
   call ch c2d( 'ff'x ),   '255'
   call ch c2d( 'ffff'x),  '65535'
   call ch c2d( 'ffff'x, 2), '-1'
   call ch c2d( 'ffff'x, 1), '-1'
   call ch c2d( 'fffe'x, 2), '-2'
   call ch c2d( 'fffe'x, 1), '-2'
   call ch c2d( 'ffff'x, 3), '65535'
   call ch c2d( 'ff7f'x, 1), '127'
   call ch c2d( 'ff7f'x, 2), '-129'
   call ch c2d( 'ff7f'x, 3), '65407'
   call ch c2d( 'ff80'x, 1), '-128'
   call ch c2d( 'ff80'x, 2), '-128'
   call ch c2d( 'ff80'x, 3), '65408'
   call ch c2d( 'ff81'x, 1), '-127'
   call ch c2d( 'ff81'x, 2), '-127'
   call ch c2d( 'ff81'x, 3), '65409'
   call ch c2d( 'ffffffffff'x, 5), '-1'


/* ======================== C2X ============================== */
call notify 'c2x'
   call ch c2x( 'foobar'),             '666F6F626172'
   call ch c2x( '' ),                  ''
   call ch c2x( '101'x ),              '0101'
   call ch c2x( '0123456789abcdef'x ), '0123456789ABCDEF'
   call ch c2x( 'ffff'x ),             'FFFF'
   call ch c2x( 'ffffffff'x ),         'FFFFFFFF'


/* ======================= CENTER ============================== */
call notify 'center'
   call ch center('****',8,'-')      ,'--****--'
   call ch center('****',7,'-')      ,'-****--'
   call ch center('*****',8,'-')     ,'-*****--'
   call ch center('*****',7,'-')     ,'-*****-'
   call ch center('12345678',4,'-')  ,'3456'
   call ch center('12345678',5,'-')  ,'23456'
   call ch center('1234567',4,'-')   ,'2345'
   call ch center('1234567',5,'-')   ,'23456'


/* ===================== CHANGESTR ============================= */
if ver > 4 Then
  do
    call notify 'changestr'
    Call ch changestr('a','fred','c') , 'fred'
    Call ch changestr('','','x') , ''
    Call ch changestr('a','abcdef','x') , 'xbcdef'
    Call ch changestr('0','0','1') , '1'
    Call ch changestr('a','def','xyz') , 'def'
    Call ch changestr('a','','x') , ''
    Call ch changestr('','def','xyz') , 'def'
    Call ch changestr('abc','abcdef','xyz') , 'xyzdef'
    Call ch changestr('abcdefg','abcdef','xyz') , 'abcdef'
    Call ch changestr('abc','abcdefabccdabcd','z') , 'zdefzcdzd'
  end

/* ====================== COMPARE ============================== */
call notify 'compare'
   call ch compare('foo', 'bar')      , 1
   call ch compare('foo', 'foo')      , 0
   call ch compare('   ', '' )        , 0
   call ch compare('foo', 'f', 'o')   , 0
   call ch compare('foobar', 'foobag'), 6



/* ======================= COPIES ============================== */
call notify 'copies'
   call ch copies('foo',3)     , 'foofoofoo'
   call ch copies('x', 10)     , 'xxxxxxxxxx'
   call ch copies('', 50)      , ''
   call ch copies('', 0)       , ''
   call ch copies('foobar',0 ) , ''

/* ===================== COUNTSTR ============================== */
if ver > 4 Then
  do
    call notify 'countstr'
    Call ch countstr('','') , 0
    Call ch countstr('a','abcdef') , 1
    Call ch countstr(0,0) , 1
    Call ch countstr('a','def') , 0
    Call ch countstr('a','') , 0
/*  Call ch countstr('','def') , 0  */
    Call ch countstr('abc','abcdef') , 1
    Call ch countstr('abcdefg','abcdef') , 0
    Call ch countstr('abc','abcdefabccdabcd') , 3
  end

/* ===================== DATATYPE ============================== */
call notify 'datatype'
   call ch datatype('foobar'), 'CHAR'
   call ch datatype('foo bar'), 'CHAR'
   call ch datatype('123.456.789'), 'CHAR'
   call ch datatype('123.456'), 'NUM'
   call ch datatype(''), 'CHAR'
   call ch datatype('DeadBeef','A'), '1'
   call ch datatype('Dead Beef','A'), '0'
   call ch datatype('1234ABCD','A'), '1'
   call ch datatype('','A'), '0'
   call ch datatype('foobar','B'), '0'
   call ch datatype('01001101','B'), '1'
   call ch datatype('0110 1101','B'), '1'
   call ch datatype('0110 110','B'), '0'
   call ch datatype('','B'), '1'
   call ch datatype('foobar','L'), '1'
   call ch datatype('FooBar','L'), '0'
   call ch datatype('foo bar','L'), '0'
   call ch datatype('','L'), '0'
   call ch datatype('foobar','M'), '1'
   call ch datatype('FooBar','M'), '1'
   call ch datatype('foo bar','M'), '0'
   call ch datatype('FOOBAR','M'), '1'
   call ch datatype('','M'), '0'
   call ch datatype('foo bar','N'), '0'
   call ch datatype('1324.1234','N'), '1'
   call ch datatype('123.456.789','N'), '0'
   call ch datatype('','N'), '0'
   call ch datatype('foo bar','S'), '0'
   call ch datatype('??@##_Foo$Bar!!!','S'), '1'
   call ch datatype('','S'), '0'
   call ch datatype('foo bar','U'), '0'
   call ch datatype('Foo Bar','U'), '0'
   call ch datatype('FOOBAR','U'), '1'
   call ch datatype('','U'), '0'

   numeric digits 9
   call ch datatype('Foobar','W'), '0'
   call ch datatype('123','W'), '1'
   call ch datatype('12.3','W'), '0'
   call ch datatype('','W'), '0'
   call ch datatype('123.123','W'), '0'
   call ch datatype('123.123E3','W'), '1'
   call ch datatype('123.0000003','W'), '0'
   call ch datatype('123.0000004','W'), '0'
   call ch datatype('123.0000005','W'), '0'
   call ch datatype('123.0000006','W'), '0'
   call ch datatype(' 23','W'), '1'
   call ch datatype(' 23 ','W'), '1'
   call ch datatype('23 ','W'), '1'
   call ch datatype('123.00','W'), '1'
   call ch datatype('123000E-2','W'), '1'
   call ch datatype('123000E+2','W'), '1'

   call ch datatype('Foobar','X'), '0'
   call ch datatype('DeadBeef','X'), '1'
   call ch datatype('A B C','X'), '0'
   call ch datatype('A BC DF','X'), '1'
   call ch datatype('123ABC','X'), '1'
   call ch datatype('123AHC','X'), '0'
   call ch datatype('','X'), '1'

   call ch datatype('0.000E-2','w'), '1'
   call ch datatype('0.000E-1','w'), '1'
   call ch datatype('0.000E0','w'), '1'
   call ch datatype('0.000E1','w'), '1'
   call ch datatype('0.000E2','w'), '1'
   call ch datatype('0.000E3','w'), '1'
   call ch datatype('0.000E4','w'), '1'
   call ch datatype('0.000E5','w'), '1'
   call ch datatype('0.000E6','w'), '1'

   call ch datatype('0E-1','w'), '1'
   call ch datatype('0E0','w'), '1'
   call ch datatype('0E1','w'), '1'
   call ch datatype('0E2','w'), '1'



/* ====================== DELSTR ============================== */
call notify 'delstr'
   call ch delstr('Med lov skal land bygges', 6), 'Med l'
   call ch delstr('Med lov skal land bygges', 6,10), 'Med lnd bygges'
   call ch delstr('Med lov skal land bygges', 1), ''
   call ch delstr('Med lov skal', 30), 'Med lov skal'
   call ch delstr('Med lov skal', 8,8), 'Med lov'
   call ch delstr('Med lov skal', 12), 'Med lov ska'
   call ch delstr('Med lov skal', 13), 'Med lov skal'
   call ch delstr('Med lov skal', 14), 'Med lov skal'
   call ch delstr('', 30), ''

/* ====================== DELWORD ============================== */
call notify 'delword'
   call ch delword('Med lov skal land bygges', 3), 'Med lov '
   call ch delword('Med lov skal land bygges', 1), ''
   call ch delword('Med lov skal land bygges', 1,1), 'lov skal land bygges'
   call ch delword('Med lov skal land bygges', 2,3), 'Med bygges'
   call ch delword('Med lov skal land bygges', 2,10), 'Med '
   call ch delword('Med lov   skal land bygges', 3,2), 'Med lov   bygges'
   call ch delword('Med lov   skal land   bygges', 3,2), 'Med lov   bygges'
   call ch delword('Med lov skal land   bygges', 3,2), 'Med lov bygges'
   call ch delword('Med lov skal land bygges', 3,0), 'Med lov skal land bygges'
   call ch delword('Med lov skal land bygges', 10), 'Med lov skal land bygges'
   call ch delword('Med lov skal land bygges', 9,9), 'Med lov skal land bygges'
   call ch delword('Med lov skal land bygges', 1,0), 'Med lov skal land bygges'
   call ch delword('  Med lov skal', 1,0), '  Med lov skal'
   call ch delword('  Med lov skal   ', 4), '  Med lov skal   '
   call ch delword('', 1), ''



/* ====================== DIGITS ============================== */
call notify 'digits'
   call ch digits(), '9'


/* ======================= D2C ============================== */
call notify 'd2c'
   call ch d2c(127), '7f'x
   call ch d2c(128), '80'x
   call ch d2c(129), '81'x
   call ch d2c(1), '01'x
   call ch d2c(-1,1), 'FF'x
   call ch d2c(-127,1), '81'x
   call ch d2c(-128,1), '80'x
   call ch d2c(-129,1), '7F'x
   call ch d2c(-1,2), 'FFFF'x
   call ch d2c(-127,2), 'FF81'x
   call ch d2c(-128,2), 'FF80'x
   call ch d2c(-129,2), 'FF7F'x
   call ch d2c(129,0), ''
   call ch d2c(129,1), '81'x
   call ch d2c(256+129,2), '0181'x
   call ch d2c(256*256+256+129,3), '010181'x


/* ======================= D2X ============================== */
call notify 'd2x'
/*   call ch d2x(0), '0' */
   call ch d2x(127), '7F'
   call ch d2x(128), '80'
   call ch d2x(129), '81'
   call ch d2x(1), '1'
   call ch d2x(-1,2), 'FF'
   call ch d2x(-127,2), '81'
   call ch d2x(-128,2), '80'
   call ch d2x(-129,2), '7F'
   call ch d2x(-1,3), 'FFF'
   call ch d2x(-127,3), 'F81'
   call ch d2x(-128,4), 'FF80'
   call ch d2x(-129,5), 'FFF7F'
   call ch d2x(129,0), ''
   call ch d2x(129,2), '81'
   call ch d2x(256+129,4), '0181'
   call ch d2x(256*256+256+129,6), '010181'



/* ===================== ERRORTEXT ============================== */
call notify 'errortext'
   call ch errortext(10), 'Unexpected or unmatched END'
   call ch errortext(40), 'Incorrect call to routine'
   call ch errortext(50), 'Unrecognized reserved symbol'
   call ch errortext( 1), ''



/* ======================== FORM ============================== */
call notify 'form'
   call ch form(), 'SCIENTIFIC'


/* ======================= FORMAT ============================== */
call notify 'format'
   call ch format(12.34), '12.34'
   call ch format(12.34,4), '  12.34'
   call ch format(12.34,4,4), '  12.3400'
   call ch format(12.34,4,1), '  12.3'
   call ch format(12.35,4,1), '  12.4'
   call ch format(12.34,,4), '12.3400'
   call ch format(12.34,4,0), '  12'

   call ch format(99.995,3,2), '100.00'
   call ch format(0.111,,4), '0.1110'
   call ch format(0.0111,,4), '0.0111'
   call ch format(0.00111,,4), '0.0011'
   call ch format(0.000111,,4), '0.0001'
   call ch format(0.0000111,,4), '0.0000'
   call ch format(0.00000111,,4), '0.0000'
   call ch format(0.555,,4), '0.5550'
   call ch format(0.0555,,4), '0.0555'
   call ch format(0.00555,,4), '0.0056'
   call ch format(0.000555,,4), '0.0006'
   call ch format(0.0000555,,4), '0.0001'
   call ch format(0.00000555,,4), '0.0000'
   call ch format(0.999,,4), '0.9990'
   call ch format(0.0999,,4), '0.0999'
   call ch format(0.00999,,4), '0.0100'
   call ch format(0.000999,,4), '0.0010'
   call ch format(0.0000999,,4), '0.0001'
   call ch format(0.00000999,,4), '0.0000'
   call ch format(0.455,,4), '0.4550'
   call ch format(0.0455,,4), '0.0455'
   call ch format(0.00455,,4), '0.0046'
   call ch format(0.000455,,4), '0.0005'
   call ch format(0.0000455,,4), '0.0000'
   call ch format(0.00000455,,4), '0.0000'

   call ch format(1.00000045,,6), '1.000000'
call ch format(1.000000045,,7), '1.0000001'  /* this is an error in TRL */
call ch format(1.0000000045,,8), '1.00000000'

   call ch format(12.34,,,,0), '1.234E+1'
   call ch format(12.34,,,3,0), '1.234E+001'
   call ch format(12.34,,,3,),  '12.34'
   call ch format(1.234,,,3,0), '1.234     '
   call ch format(12.34,3,,,0), '  1.234E+1'
   call ch format(12.34,,2,,0), '1.23E+1'
   call ch format(12.34,,3,,0), '1.234E+1'
   call ch format(12.34,,4,,0), '1.2340E+1'
   call ch format(12.345,,3,,0), '1.235E+1'

   call ch format(99.999,,,,), '99.999'
   call ch format(99.999,,2,,), '100.00'
   call ch format(99.999,,2,,2), '1.00E+2'
   call ch format(.999999,,4,2,2), '1.0000'
   call ch format(.999999,,5,2,2),  '9.99999E-01'
   call ch format(.9999999,,5,2,2), '1.00000    '
   call ch format(.999999,,6,2,2), '9.999990E-01'
   call ch format(90.999,,0), '91'
   call ch format(0099.999,5,3,,), '   99.999'

   call ch format(0.0000000000000000001,4), '   1E-19'
   call ch format(0.0000000000000000001,4,4), '   1.0000E-19'
   call ch format(0.0000001,4,,,3), '   1E-7'
   call ch format(0.0000001,4,4,,3), '   1.0000E-7'
   call ch format(0.000001,4,4,,3), '   0.0000'
   call ch format(0.0000001,4,5,,2), '   1.00000E-7'
   call ch format(0.0000001,4,4,4,3), '   1.0000E-0007'
   call ch format(1000,4,4,,3), '   1.0000E+3'

   call ch format(0.0000000000000000000001), '1E-22'
   call ch format(0.0000000000000000000001,,,0,), '0.0000000000000000000001'
   call ch format(0.0000001,,,0,3), '0.0000001'



/* ======================== FUZZ ============================== */
call notify 'fuzz'
   call ch fuzz(), '0'


/* ======================= INSERT ============================== */
call notify 'insert'
   call ch insert('abc','def'), 'abcdef'
   call ch insert('abc','def',2), 'deabcf'
   call ch insert('abc','def',3), 'defabc'
   call ch insert('abc','def',5), 'def  abc'
   call ch insert('abc','def',5,,'*'), 'def**abc'
   call ch insert('abc','def',5,4,'*'), 'def**abc*'
   call ch insert('abc','def',,0), 'def'
   call ch insert('abc','def',2,1), 'deaf'


/* ====================== LASTPOS ============================== */
call notify 'lastpos'
   call ch lastpos('b', 'abc abc'), 6
   call ch lastpos('b', 'abc abc',5), 2
   call ch lastpos('b', 'abc abc',6), 6
   call ch lastpos('b', 'abc abc',7), 6
   call ch lastpos('x', 'abc abc'), 0
   call ch lastpos('b', 'abc abc',20), 6
   call ch lastpos('b', ''), 0
   call ch lastpos('', 'c'), 0
   call ch lastpos('', ''), 0
   call ch lastpos('b', 'abc abc',20), 6
   call ch lastpos('bc', 'abc abc'), 6
   call ch lastpos('bc ', 'abc abc',20), 2
   call ch lastpos('abc', 'abc abc',6), 1
   call ch lastpos('abc', 'abc abc'), 5
   call ch lastpos('abc', 'abc abc',7), 5


/* ======================== LEFT ============================== */
call notify 'left'
   call ch left('foobar',1),     'f'
   call ch left('foobar',0),     ''
   call ch left('foobar',6),     'foobar'
   call ch left('foobar',8),     'foobar  '
   call ch left('foobar',8,'*'), 'foobar**'
   call ch left('foobar',1,'*'), 'f'



/* ======================= LENGTH ============================== */
call notify 'length'
   call ch length(''),            0
   call ch length('a'),           1
   call ch length('abc'),         3
   call ch length('abcdefghij'), 10

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
