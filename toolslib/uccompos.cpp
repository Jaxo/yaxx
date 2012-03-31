/*
* $Id: uccompos.cpp,v 1.9 2011-07-29 10:26:38 pgr Exp $
*
* Class to compose UnicodeString's using a Template
*/

/*--------------+
| Include Files |
+--------------*/
#include <assert.h>
#include "ostream"
#include "uccompos.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------+
| Local constants |
+----------------*/

UCS_2 const UnicodeComposer::Template::ucArgOpen = (UCS_2)'&';
UCS_2 const UnicodeComposer::Template::ucArgVi = (UCS_2)'=';
UCS_2 const UnicodeComposer::Template::ucArgTab = (UCS_2)'\t';
UCS_2 const UnicodeComposer::Template::ucArgClose = (UCS_2)';';

UnicodeString const UnicodeComposer::Template::ucsArgClosings("; &");

UnicodeString const UnicodeComposer::Template::ucsRefArguments("errargs");
UnicodeString const UnicodeComposer::Template::ucsRefStringId("errcode");
UnicodeString const UnicodeComposer::Template::ucsRefSeverity("errsev");

UnicodeString const UnicodeComposer::Template::ucsDefaultTemplate(
   "Error &errcode; (&errsev;) &errargs"
);

/*=== UnicodeComposer::Template ==============================================*/


/*-----------------------------------------UnicodeComposer::Template::Template-+
| Constructor for a user-generated template (deprecated).                      |
+-----------------------------------------------------------------------------*/
UnicodeComposer::Template::Template(char const * templateArg)
{
   is = Self;
   stringId = _XXX__UNKNOWN;
   ucsTemplate = UnicodeString(templateArg);
}

/*-----------------------------------------UnicodeComposer::Template::Template-+
| Regular constructor for a template gotten from the resources                 |
+-----------------------------------------------------------------------------*/
UnicodeComposer::Template::Template(MsgTemplateId stringIdArg)
{
   stringId = stringIdArg;
   is = Resource;
   ucsTemplate = getMsgTemplate(stringIdArg);
}

/*---------------------------------UnicodeComposer::Template::convertToDecimal-+
| Simple conversion routine to get the decimal value of a 2 digit number       |
+-----------------------------------------------------------------------------*/
inline int UnicodeComposer::Template::convertToDecimal(
   UnicodeSubstring const & ucss
) {
   int i = -1;
   if (ucss.length() == 1) {
      int const j = ucss[0] - (UCS_2)'0';
      if ((j >= 0) && (j <= 9)) i = j;
   }else if (ucss.length() == 2) {
      int const k = ucss[0] - (UCS_2)'0';
      int const j = ucss[1] - (UCS_2)'0';
      if ((k >= 0) && (k <= 9) && (j >= 0) && (j <= 9)) i = j + (10*k);
   }
   return i;
}

/*-----------------------------------------UnicodeComposer::Template::assemble-+
| Assemble a message, given a named and a positional argument list.            |
+-----------------------------------------------------------------------------*/
UnicodeString UnicodeComposer::Template::assemble(
   ArgListWithName const & arglNamed,
   ArgListSimple const & arglPos,
   ErrorSeverity sev
) const {
   UnicodeMemWriter umostTemp;

   UnicodeSubstring ucssCurr;

   if (is == NotSet) {
      ucssCurr = ucsDefaultTemplate;
   }else {
      ucssCurr = ucsTemplate;
   }
   if (ucssCurr.empty()) {
      return UnicodeString::Nil;
   }

   while (ucssCurr.good()) {
      UnicodeSubstring ucssArgName(    // find '&'
         ucssCurr,
         ucArgOpen
      );
      umostTemp.setWidth(ucssCurr.length() - ucssArgName.length());
      umostTemp << ucssCurr;
      if (ucssArgName.empty()) {        // end of string found:
         break;                         // exit
      }

      ucssArgName = ucssArgName(1, UnicodeSubstring::LMAX);
      ucssCurr = UnicodeSubstring(      // skip to some closing delimiter
         ucssArgName,
         UnicodeSubstring::ONE_OF, ucsArgClosings
      );
      ucssArgName = ucssArgName(0, ucssArgName.length() - ucssCurr.length());

      // skip over the true closing delimiter, if any
      bool isArgClosed = false;
      if (ucssCurr[0] == ucArgClose) {
         isArgClosed = true;
         ucssCurr = ucssCurr(1, UnicodeSubstring::LMAX);
      }
      if (ucssArgName.good()) {
         int iValue = convertToDecimal(ucssArgName);
         UnicodeString ucsArgValue(arglPos[iValue-1]);
         if (
            ucsArgValue.good() ||
            (ucsArgValue = arglNamed[ucssArgName], ucsArgValue.good())
         ) {
            umostTemp << ucsArgValue;
            continue;
         }else {
            /*
            | PGR: 05/01/98 - This is a temporary fix.
            | The algorithm was used to keep unknown references as if
            | it has been the text itself.
            | Unfortunately, some messages do not have their &1; / &2; ...
            | resolved.  These messages need to be fixed.
            | In the meanwhile , they are produced as:
            | > Error in "&1;" near "&2;"
            | which is not really user friendly...
            | Hence, while waiting for all messages to be fixed,
            | let's just do what ERR was doing: produce an empty reference.
            */
            if (iValue > 0) {  // it's &[1..99];
               continue;
            }
         }
         if (ucssArgName == ucsRefStringId) {
            umostTemp << (int)stringId;
            continue;
         }
         if (ucssArgName == ucsRefSeverity) {
            switch (sev) {
            case ECE__INFO:     umostTemp << 'I';  break;
            case ECE__WARNING:  umostTemp << 'W';  break;
            case ECE__ERROR:    umostTemp << 'E';  break;
            case ECE__SEVERE:   umostTemp << 'S';  break;
            case ECE__FATAL:    umostTemp << 'F';  break;
            case ECE__DEBUG:    umostTemp << 'D';  break;
            default:            umostTemp << '?';  break;
            }
            continue;
         }
         if (ucssArgName == ucsRefArguments) {
            bool isSomethingWritten = umostTemp.inAvail()? true : false;
            for (int iTimes = 0; iTimes < 2; ++ iTimes) {
               int iCount;
               switch (iTimes) {
               case 0:  iCount = arglPos.count(); break;
               default: iCount = arglNamed.count(); break;
               }
               if (iCount) {
                  int i = 0;
                  do {
                     if (!isSomethingWritten) {    // starting a new message..
                        umostTemp << "--" << ucArgTab;
                        isSomethingWritten = true;
                     }else {                       // else, end previous output
                        umostTemp << endl << ucArgTab;
                     }
                     if (!iTimes){
                        umostTemp << arglPos[i];
                     }else {
                        umostTemp << arglNamed[i].inqName()
                           << ucArgVi << arglNamed[i].inqValue();
                     }
                  }while (++i < iCount);
               }
            }
            if (isSomethingWritten && ucssCurr.good()) {  // more to write...
               umostTemp << endl;                  // separator with the rest
            }
            continue;
         }
      }
      // if nothing was properly interpreted, output what was found
      umostTemp << ucArgOpen << ucssArgName;
      if (isArgClosed) {
         umostTemp << ucArgClose;
      }
   }
   return umostTemp;
}


/*=== UnicodeComposer ========================================================*/


/*--------------------------------------------UnicodeComposer::UnicodeComposer-+
| Constructor                                                                  |
+-----------------------------------------------------------------------------*/
UnicodeComposer::UnicodeComposer() : bMessagePending(false) {
}

/*VIRTUAL--------------------------------------UnicodeComposer::finishMessage-+
|                                                                             |
+----------------------------------------------------------------------------*/
void UnicodeComposer::finishMessage(Message & msgArg)
{
   Message msg = msgArg;        // save it!
   clear();                     // get ready for new input
   int iLast = watchList.count();
   while (
      iLast-- &&
      !watchList[iLast]->notifyNewEntry(msg)
   ) {}
}

/*-------------------------------------------------------UnicodeComposer::endm-+
|                                                                              |
+-----------------------------------------------------------------------------*/
UnicodeComposer & endm(UnicodeComposer & cps) {
   cps.finishMessage(cps.m_msg);
   cps.clear();
   return cps;
}

/*VIRTUAL------------------------------------UnicodeComposer::~UnicodeComposer-+
|                                                                              |
+-----------------------------------------------------------------------------*/
UnicodeComposer::~UnicodeComposer()
{
   endMessage();
}

/*-----------------------------------------UnicodeComposer::MessageList::clear-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void UnicodeComposer::MessageList::clear() {
   while (count()) removeFirst();
}

/*-----------------------------------------UnicodeComposer::MessageList::print-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void UnicodeComposer::MessageList::print(MultibyteWriter & ubost)
{
   int const iCount = count();
   for (int ix=0; ix < iCount; ++ix) {
      Item * pItem = (Item *)findDataAt(ix);
      ubost << pItem->stringize() << endl;
   }
   ubost << flush;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
