/* tmpl: */
a = 'This is  the text which, I think,  is scanned.'
parse var a w1 ',' w2 ',' rest; Say "W1   = '"w1"'"; Say "W2   = '"w2"'"; Say "REST = '"rest"'";
parse var a w1 ',' w2 ',' w3 ',' rest; Say "W1   = '"w1"'"; Say "W2   = '"w2"'"; Say "W3   = '"w3"'"; Say "REST = '"rest"'";
parse var a w1 w2 w3 rest ','; Say "W1   = '"w1"'"; Say "W2   = '"w2"'"; Say "W3   = '"w3"'"; Say "REST = '"rest"'";
parse var a w1 10 w2 20 w3; Say "W1   = '"w1"'"; Say "W2   = '"w2"'"; Say "W3   = '"w3"'";
a = "123456789"
parse var a 3 w1 +3 w2 3 w3; Say W1 '    ' w2 '     ' w3
parse var a 3 w1 -3 w2 3 w3; Say W1 '    ' w2 '     ' w3
parse var a 4 w1 -1 w2 -2 w3; Say W1 '    ' w2 '     ' w3
parse var a -10 w1 -1 w2 -2 w3; Say W1 '    ' w2 '     ' w3
a = "L/look for/1 10"
parse var a verb 2 delim +1 string (delim) rest;
Say "VERB   = '"verb "'"; Say "DELIM  = '"delim"'"; Say "STRING = '"string"'"; Say "REST   = '"rest"'"
a = "Hello, World!"; parse var a ',' -1 x +1; Say "X = '"||x"'"
do I=6 BY 3.1416 to 12; Say "Hello"; End; Say I
do I=6 /*BY 3.1416*/ to 12; Say "Hi"; End; Say I
do I=1 BY 2 to 4; Say "Glouglou"; End; Say I
do I=4 BY -2 to 1; Say "Roudoudou"; End; Say I

exit 'Hello World'
call hello
say Result
exit 5

hello:
return 'Hello World'
