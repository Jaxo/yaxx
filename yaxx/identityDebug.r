/* $Id: identityDebug.r,v 1.2 2002-02-25 22:02:14 jlatone Exp $                     */
/* The YAXX identity transform.                                            */
/* to test it: YAXX -r beer.html identity.r                                */

indent = 0
/* CALL ON NOVALUE #DefaultProc  - not yet implemented */
call applyTemplates
return

#text:
   CALL XmlOut , event
   RETURN

#PI:
   CALL XmlOut , '0a'x || '<?'
   CALL XmlOut , event
   CALL XmlOut , '?>'
   RETURN

#Comment:
   CALL XmlOut , '0a'x || '<!--'
   CALL XmlOut , event
   CALL XmlOut , '-->'
   RETURN

#DefaultProc:
   if type == "S" | type == "V" THEN DO /* iff that was explicitely entered! */
      CALL XmlOut , Left('0a'x, indent, ' ')
      CALL XmlOut , event
   END
   indent = indent + 1
   IF type \= "V" THEN DO WHILE applyTemplates(); END
   indent = indent - 1;
   if type == "E" THEN DO  /* iff that was explicitely entered! */
      CALL XmlOut , Left('0a'x, indent, ' ') || "</" || value || ">"
   END
   RETURN

applyTemplates: PROCEDURE EXPOSE indent event type
   /*
   | Notes:
   |
   | 1) expose variables as shown above, so they  won't be duplicated
   |    among the nest of procedure calls.  It matters for the global
   |    footprint.
   |
   | 2) The "xmltype()" built-in
   |    - has two prototypes:
   |      "xmltype(event)"        that returns "type"
   |      "xmltype(event, type)"  that returns "1" or "0" if the event
   |                              matches the type
   |
   |    - where event is a string containing some XML markup
   |
   |    - and type is one of:
   |      'S' or 's' if the event is a start-tag
   |      'V' or 'v' if the event is an empty-tag (e.g. Void)
   |      'E' or 'e' if the event is an end-tag
   |      'T' if the event is regular text
   |      'D' if the event is data text (CDATA or SDATA)
   |      'P' if the event is a Processing Instruction
   |      'C' if the event is a Comment
   |      '!' if the event is NIL (generally: EOF)
   |      '?' if the event is invalid (bad markup)
   |      Upper case describes markup explicitely entered in the document.
   |      Lower case describes markup inferred by the SGML Parser.
   */
   DO FOREVER
      event = XmlIn();
      type = XmlType(event)
      SELECT
         WHEN type == 'S' | type == 'V' | type == 's' | type == 'v' THEN DO
            /* following gets the tagname */
            value = SUBSTR(STRIP(WORD(event, 1), 'T', '>'), 2)
            CALL (value)
            RETURN 1
            END
         WHEN type == 'T' | type == 'D' | type == '?' THEN DO
            CALL #text
            RETURN 1
            END
         WHEN type == 'P' THEN DO
            CALL #PI
            RETURN 1
            END
         WHEN type == 'C' THEN DO
            CALL #Comment
            RETURN 1
            END
         WHEN type == 'E' | type == 'e' THEN
            RETURN 0
         WHEN type == '!' THEN DO
            Say "Dangling Parse!"
            EXIT
            END
         OTHERWISE  /* keep on looping */
      END
   END
