/* c2d2c: */
written=0
call notify 'c2d'
   NUMERIC DIGITS 20
   Say d2c(c2d( 'helloyou'));

   NUMERIC DIGITS 39
   call ch c2d('salutmonde'), '544869294985900767208549'
   call ch c2d( 'hellosof'), '7522537965574385510'
   call ch d2c(127), '7f'x
   call ch c2d( 'ffff'x),  '65535'
   call ch c2d( 'ffffff'x, 3), '-1'
   call ch c2d( 'ffffffff'x, 4), '-1'
   call ch c2d( 'ffffffff'x),  '4294967295'
   call ch c2d( 'fefdfcfb'x, 4), '-16909061'
   call ch c2d( 'fffefdfcfb'x, 5), '-16909061'
   call ch c2d( 'ffffffffffffffff'x), '18446744073709551615'
   call ch c2d( 'ffffffffffffffffffffffffffffffff'x), '340282366920938463463374607431768211455'
              /* ----+----1----+----2----+----3-- */
   call ch c2d( 'ffffffffffffffffffffffffffffffff'x, 17), '340282366920938463463374607431768211455'
   call ch c2d( 'ffffffffffffffffffffffffffffffff'x, 16), '-1'
   call ch c2d( 'fffffffffffffffffffffffffffffffe'x, 16), '-2'
   call ch c2d( 'fffffffffffffffffffffffffffffffd'x, 16), '-3'
   call ch c2d( 'fffffffffffffffffffffffffffffffc'x, 16), '-4'
   call ch c2d( 'fffffffffffffffffffffffffffffffb'x, 16), '-5'
   call ch c2d( 'ffffffffffffffffffffffffffffffff'x, 10), '-1'
   call ch c2d( 'ffffffffffffffffffffffffffffffff'x, 5), '-1'
   call ch c2d( 'fof'), '6713190'
   call ch c2d( 'fofo'), '1718576751'
   call ch c2d( 'ffffffffff'x, 5), '-1'
   call ch c2d( 'ff80'x, 1), '-128'
   call ch c2d( 'foo' ) ,  '6713199'
   call ch c2d( 'foofoof' ) ,  '28832971875118950'
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
   call ch d2c(c2d('salutmonde')), 'salutmonde'
   call ch d2c('7522537965574385510'), 'hellosof'


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
