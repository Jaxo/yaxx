/* $Id: screen.h,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

extern void updateScroll();
extern void linesScroll(int numLinesToScroll, Boolean redraw);
extern void display(char const * text, int len);
extern void pageScroll(WinDirectionType direction);
extern int cleanUpCR(char * buf, int len);

/*===========================================================================*/
