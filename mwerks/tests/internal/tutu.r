/* tutu: */
call testarg 1,2,3,4,5
call testarg ,,,4,5
call testarg 1,,,4,5
call testarg 1,,3,, ;
call testarg ,,,,   ;
call testarg 1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6
exit

testarg:
  Say 'These were' arg() 'arguments'
  return
