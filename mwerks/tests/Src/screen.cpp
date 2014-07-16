/* $Id: screen.cpp,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include <string.h>
#include "testsRsc.h"
#include "screen.h"

/*---------------------------------------------------------------updateScroll-+
|                                                                             |
+----------------------------------------------------------------------------*/
void updateScroll()
{
   FormPtr frm = FrmGetActiveForm();
   FieldPtr field = (FieldPtr)FrmGetObjectPtr(
      frm,
      FrmGetObjectIndex(frm, MainConsoleField)
   );
   ScrollBarPtr scroll = (ScrollBarPtr)FrmGetObjectPtr(
      frm,
      FrmGetObjectIndex(frm,MainConsoleScrollBar)
   );
   UInt16 currentPosition, textHeight, fieldHeight, maxValue;

   FldGetScrollValues(field, &currentPosition, &textHeight, &fieldHeight);
   if (textHeight > fieldHeight) {
      maxValue = textHeight - fieldHeight;
   }else {
      maxValue = 0;
   }
   SclSetScrollBar(scroll, currentPosition, 0, maxValue, fieldHeight - 1);
}

/*----------------------------------------------------------------linesScroll-+
|                                                                             |
+----------------------------------------------------------------------------*/
void linesScroll(int numLinesToScroll, Boolean redraw)
{
   FormPtr frm = FrmGetActiveForm();
   FieldPtr field = (FieldPtr)FrmGetObjectPtr(
      frm,
      FrmGetObjectIndex(frm, MainConsoleField)
   );

   if (numLinesToScroll < 0) {
      FldScrollField(field, -numLinesToScroll, winUp);
   }else {
      FldScrollField(field, numLinesToScroll, winDown);
   }
   if ((FldGetNumberOfBlankLines(field) && numLinesToScroll < 0) || redraw) {
      updateScroll();
   }
}

/*--------------------------------------------------------------------display-+
|                                                                             |
+----------------------------------------------------------------------------*/
void display(char const * text, int len)
{
   FormPtr frm = FrmGetActiveForm();
   FieldPtr field = (FieldPtr)FrmGetObjectPtr(
      frm,
      FrmGetObjectIndex(frm, MainConsoleField)
   );
   FldInsert(field, text, len);
   updateScroll();
   FldDrawField(field);
}

/*-----------------------------------------------------------------pageScroll-+
|                                                                             |
+----------------------------------------------------------------------------*/
void pageScroll(WinDirectionType direction)
{
   FormPtr frm = FrmGetActiveForm();
   FieldPtr field = (FieldPtr)FrmGetObjectPtr(
      frm,
      FrmGetObjectIndex(frm, MainConsoleField)
   );

   if (FldScrollable(field, direction)) {
      int linesToScroll = FldGetVisibleLines(field) - 1;
      if (direction == winUp) linesToScroll = -linesToScroll;
      linesScroll(linesToScroll, true);
   }
}

/*------------------------------------------------------------------cleanUpCR-+
|                                                                             |
+----------------------------------------------------------------------------*/
int cleanUpCR(char * buf, int len) {
   char * pCur = buf;
   char * pEnd = pCur + len;
   while (
      pCur = (char *)memchr(pCur, '\r', pEnd-pCur),
      pCur != 0
   )  {
      memcpy(pCur, pCur+1, (--pEnd)-pCur);
   }
   return pEnd-buf;
}

/*===========================================================================*/
