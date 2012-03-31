/* $Id: identity.r,v 1.16 2002-03-27 08:57:42 pgr Exp $
*  The IRexx identity transform.
*  to test it: YAXX -r foo.html identity.r
*/

/* CALL ON NOVALUE #DefaultProc  - not yet implemented */

type = XmlType(XmlIn());
IF type == '{' THEN DO UNTIL type == '}'
   CALL applyTemplates
END
RETURN

#text:
   CALL XmlOut , event
   RETURN

#PI:
   CALL XmlOut , "<?" || event || "?>"
   RETURN

#Comment:
   CALL XmlOut , "<!--" || event || "-->"
   RETURN

#DefaultProc:
   IF type == "S" | type == "V" THEN DO /* iff that was explicitely entered! */
      CALL XmlOut , event
   END
   IF type \= "V" THEN DO WHILE applyTemplates(); END
   IF type == "E" THEN DO  /* iff that was explicitely entered! */
      CALL XmlOut , "</" || value || ">"
   END
   RETURN

applyTemplates: PROCEDURE EXPOSE event type
   DO FOREVER
      event = XmlIn();
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
