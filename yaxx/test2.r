/* $Id: test2.r,v 1.6 2002-03-27 08:57:42 pgr Exp $ */
/* This is the file which is called by YAXX in RXF mode.                   */
/* YAXX -r beer.html test2.r                                               */

indent = 0
/* CALL ON NOVALUE #DefaultProc  - not yet implemented */
PARSE ARG w1 w2
SAY "ARG is " w1 ", " w2
CALL applyTemplates
RETURN

p:
q:
r:
   SAY Right("", indent, '-') || "Start " event " " type
   indent = indent + 1
   IF (type \= "V") THEN DO WHILE applyTemplates(); END
   indent = indent - 1;
   SAY Right("", indent, '-') || "End " value
   RETURN

#text:
   SAY Right("", indent, '-') || 'Text "' || event || '"'
   RETURN

#error:
   SAY Right("", indent, '-') || 'Error "' || event || '"'
   RETURN

#cdata:
   SAY Right("", indent, '-') || 'CDATA "' || event || '"'
   RETURN

#PI:
   SAY Right("", indent, '-') || 'PI "' || event || '"'
   RETURN

#Comment:
   SAY Right("", indent, '-') || 'Comment "' || event || '"'
   RETURN

#DefaultProc:
   SAY Right("", indent, '*') || "Start " event " " type
   indent = indent + 1
   IF (type \= "V") THEN DO WHILE applyTemplates(); END
   indent = indent - 1;
   SAY Right("", indent, '*') || "End " value
   RETURN


applyTemplates: PROCEDURE EXPOSE indent event
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
   |      '!' if the event is NIL (generally: EOF)
   |      '?' if the event is invalid (bad markup)       ** NYI **
   |      Upper case describes markup explicitely entered in the document.
   |      Lower case describes markup inferred by the SGML Parser.
   */
   DO FOREVER
      event = XmlIn();
      type = XmlType(event)
      SELECT
         WHEN type == 'S' | type == 'V' | type == 's' | type == 'v' THEN DO
            /* following gets the tagname */
/*          value = SUBSTR(STRIP(WORD(event, 1), 'T', '>'), 2) */
            value = XmlTagName(event);
            CALL (value)

            RETURN 1
            END
         WHEN type == 'T' THEN DO
            CALL #text
            RETURN 1
            END
         WHEN type == 'D' THEN DO
            CALL #cdata
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
         WHEN type == '?' THEN DO
            CALL #error
            RETURN 1
            END
         WHEN type == '!' THEN DO
            Say "Dangling Parse!"
            EXIT
            END
         OTHERWISE  /* keep on looping */
      END
   END
