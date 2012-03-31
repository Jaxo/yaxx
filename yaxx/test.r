/* $Id: test.r,v 1.7 2002-01-27 11:35:44 pgr Exp $ */
/* Second example of the integration of XML in REXX (project YAXX.)        */
/* It uses the YASP3 parser now integrated thru the XmlIn built-in         */
/* The file 'd:\u\newdev\yaxx\yaxx\Simple.sgml' looks something like:      */
/* <p>Hello <q>World</q> and <r>bye bye</r> !!!</p>                        */

indent = 0
call applyTemplates
return

p:
q:
r:
  SAY Right("", indent, '-') || "Start " value " " type; indent = indent + 1
  IF (type \= "V") THEN DO WHILE applyTemplates(); END
  indent = indent - 1;  SAY Right("", indent, '-') || "End " value
  RETURN

#text:
  SAY Right("", indent, '-') || 'Text "' || value || '"'
  RETURN

#DefaultProc:
  SAY Right("", indent, '*') || "Start " value " " type; indent = indent + 1
  IF (type \= "V") THEN DO WHILE applyTemplates(); END
  indent = indent - 1;  SAY Right("", indent, '*') || "End " value
  RETURN

applyTemplates: PROCEDURE EXPOSE indent
   DO FOREVER
      event = XmlIn('beer.html');
      type = Substr(event,1,1);
      value = Substr(event,2)
      SELECT
         WHEN type == 'S' THEN DO; CALL (value); RETURN 1; END;
         WHEN type == 'V' THEN DO; CALL (value); RETURN 0; END;  /* <tag/> */
         WHEN type == 'T' THEN DO; CALL #text;   RETURN 1; END;
         WHEN type == 'E' THEN DO;               RETURN 0; END;
         WHEN type == '?' THEN DO; Say "Dangling Parse!"; EXIT; END;
         OTHERWISE  /* keep on looping */
      END
   END
