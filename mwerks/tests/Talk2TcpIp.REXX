/* Talk2TcpIP
Simple demo of the tcpip (aka net) stream
*/
Talk2TcpIP:
   Say "What host do you want to talk to?"
   pull host
   if host == "" then host = "10.110.2.36"
   Say "And on what port?"
   pull port
   if port == "" then port = "6410"
   tcpip = "tcpip://" || host || ":" || port
   IF Stream(tcpip) \= "READY" THEN DO
      SAY "Cannot access to " tcpip
      EXIT
      END
   CALL Lineout tcpip, "Hello TcpIP! Say something..."
   SAY "TcpIP said:" Linein(tcpip)
   EXIT
