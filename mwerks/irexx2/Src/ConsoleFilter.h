/* $Id: ConsoleFilter.h,v 1.21 2002/08/23 16:56:08 pgr Exp $ */

#ifndef __ConsoleFilter_H__
#define __ConsoleFilter_H__

#include "Common.h"
#include "PalmConsole.h"
#include "PalmFileStream.h"
#include "PalmDbmStream.h"
#include "SerialStream.h"
#include "BeamerStream.h"
#include "NetStream.h"
#include "MidiStream.h"
#include "ConsoleSchemeHandler.h"
#include "SystemContext.h"

/*------------------------------------------------------ class ConsoleFilter -+
|                                                                             |
+----------------------------------------------------------------------------*/
class ConsoleFilter :
   public PalmConsole,
   public CEventFilterItem,
   public Escaper
{
public:
   ConsoleFilter();

   void initFilterConsole(
      FormPtr pForm,
      CEventFilter * pFilter,
      UInt16 fieldID,
      UInt16 scrollID,
      UInt16 rcValueID,
      UInt16 doneID
   );

   void showInitialDisplay();
   void showFinalDisplay(int rc);
   void clear();
   void bel();

   bool isInEditableArea();

   static void formatRcField(
      int rcValue, FormPtr pForm, UInt16 fieldId, int maxSize
   );

protected:
   CMemo m_memo;

   void refill(streambuf & sb, bool volatile const & haveEnough);
   int produce(char const * pchBuf, int n);

private:

   class ClearDecoder : public Escaper::ClearDecoder {
   public:
      ClearDecoder(ConsoleFilter & console) : m_console(console) {}
      void action() { m_console.clear(); }
      ConsoleFilter & m_console;
   };

   class BELDecoder : public Escaper::BELDecoder {
   public:
      BELDecoder(ConsoleFilter & console) : m_console(console) {}
      void action() { m_console.bel(); }
      ConsoleFilter & m_console;
   };

   class MySchemeHandler : public ConsoleSchemeHandler {
   public:
      MySchemeHandler(streambuf * pSb);
   private:
      class Rep : public ConsoleSchemeHandler::Rep {
      public:
         Rep(streambuf * pSb);
      private:
         istream & getCin();
         ostream & getCout();
         ostream & getCerr();

         istream cin;
         ostream cout;
         ostream cerr;
      };
   };

   FormPtr m_pForm;
   FieldPtr m_pField;
   UInt16 m_startPos;
   UInt16 m_fieldID;
   UInt16 m_scrollID;
   UInt16 m_doneID;
   UInt16 m_rcValueID;

   SystemContext * m_sysCtxt;

   Boolean PreHandleEvent(EventPtr pEvent, Boolean & bResult);
   void readVirtualKeyboard(EventType * pEvent);
   void output(char const * pchBuf, int len);
};

/* -- INLINES -- */
inline ConsoleFilter::MySchemeHandler::Rep::Rep(streambuf * pSb) :
   cin(pSb), cout(pSb), cerr(pSb) {
}
inline ConsoleFilter::MySchemeHandler::MySchemeHandler(streambuf * pSb) :
   ConsoleSchemeHandler(new Rep(pSb)) {
}
inline istream & ConsoleFilter::MySchemeHandler::Rep::getCin()  {
   return  cin;
}
inline ostream & ConsoleFilter::MySchemeHandler::Rep::getCout() {
   return  cout;
}
inline ostream & ConsoleFilter::MySchemeHandler::Rep::getCerr() {
   return  cerr;
}


#endif
/*===========================================================================*/
