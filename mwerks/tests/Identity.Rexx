/* a xmltest */

xml="dm:///MemoDB/XML/1"

type = XmlType(XmlIn(xml));
IF type == '{' THEN DO UNTIL type == '}'
   CALL applyTemplates
END
RETURN

#text:
   SAY event
   RETURN

#PI:
   SAY "<?" || event || "?>"
   RETURN

#Comment:
   SAY "<!--" || event || "-->"
   RETURN

#DefaultProc:
   IF type == "S" | type == "V" THEN DO /* iff that was explicitely entered! */
      SAY event
   END
   IF type \= "V" THEN DO WHILE applyTemplates(); END
   IF type == "E" THEN DO  /* iff that was explicitely entered! */
      SAY "</" || value || ">"
   END
   RETURN
   
applyTemplates: PROCEDURE EXPOSE event type xml
   DO FOREVER
      event = XmlIn(xml);
      type = XmlType(event)
      SELECT
         WHEN type == 'S' | type == 'V' | type == 's' | type == 'v' THEN DO
            /* following gets the tagname */
            /* value = SUBSTR(STRIP(WORD(event, 1), 'T', '>'), 2) */
            value = XmlTagName(event)
            CALL (value)
            RETURN 1
            END
         WHEN type == 'T' | type == 'D' | type == '?' THEN DO
            CALL #text
            RETURN 1
            END
         WHEN type == 'E' | type == 'e' THEN
            RETURN 0
         WHEN type == 'P' THEN DO
            CALL #PI
            RETURN 1
            END
         WHEN type == 'C' THEN DO
            CALL #Comment
            RETURN 1
            END
         WHEN type == '}' THEN DO
            RETURN 0
            END
         OTHERWISE
            SAY "*** Unknown event:" type
            EXIT
      END
   END

