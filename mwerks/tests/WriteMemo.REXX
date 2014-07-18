/* WriteMemo
Add a new record with "Hello World!" in the Biz cat.
*/
WriteMemo:
output="dm:///MemoDB/Business"
CALL Lineout output, "Hello World!"
CALL Charout output, '00'x
