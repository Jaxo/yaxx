/*
* $Id: main.cpp,v 1.5 2002-04-01 10:21:21 pgr Exp $
*
*  Parse a SGML/HTML/XML document and trace the events returned.
*/

/*---------+
| Includes |
+---------*/
#include "main.h"
#include "solver.h"

/*---------------------------------------------------------Sample01::Sample01-+
|                                                                             |
+----------------------------------------------------------------------------*/
Sample01::Sample01(
   UnicodeComposer &erhArg,
   int argc,
   char const * const argv[]
) :
   erh(erhArg),
   options(erhArg, argc, argv),
   ysp(
      erhArg,
      options.isHtml()? SgmlDecl::HTML_WITH_NO_ENTREF : SgmlDecl::XML
   ),
   bOk(true)
{
   if (!options.isOk()) {
      bOk = false;
      return;
   }
   if (!ysp) {
      erh << ECE__ERROR << _SMP__NOPARSER << endm;
      bOk = false;
      return;
   }
   // Set up empty tag list
   if (options.inqEmpty(0)) {
      erh << ECE__INFO << _SMP__SETEMPTIES << endm;
      // don't reduce following for loop: win32 wouldn't understand.
      for (int i = 0; ; ++i) {
         UnicodeString ucsEmpty(options.inqEmpty(i));
         if (!ucsEmpty) break;
         erh << ECE__INFO << "   &1;" << ucsEmpty << endm;
         ysp.addElementEmpty(ucsEmpty);
      }
   }
   if (options.isValidate()) {
      ysp << YSP__validate;
   }
   if (options.isCompressBlanks()) {
      erh << ECE__INFO << _SMP__BLANKCOMPRESS << endm;
      ysp << YSP__killRERS << YSP__killTAB << YSP__killLEAD << YSP__killTRAIL;
   }
   if (options.isIncludeIgnoredMS()) {
      ysp << YSP__includeIgnoredMS;
   }
   if (options.isReportEntities()) {
      ysp << YSP__reportEntityEvents;
   }
   if (options.isDontExpandPIorSDATA()) {
      ysp >> ENTK_PI >> ENTK_SDATA;
   }
   if (options.isDontExpandExternal()) {
      ysp >> ENTK_SGML;
   }

   /*
   | the following is for our own internal tests.
   */
   if (options.inqListWildOptions().count()) {
      TpIxList const & list = options.inqListWildOptions();
      int const iLast = list.count();
      int i = 0;
      do {
         ysp << (YaspOptName)(list[i]);
      }while (++i < iLast);
   }

   return;
}

/*--------------------------------------------------------------Sample01::run-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Sample01::run()
{
   assert (bOk);
   if (!bOk) return;
   Solver solver(options.inqPrimaryPath(), options.inqDtd(), ysp);
   MultibyteWriter out(cout.rdbuf());
   Reporter reporter(
      ysp,
      options,
      (options.isHtml() || options.isXml())? false : true,
      out
   );

   if (!reporter) {                        // can't get an event reporter
      erh << ECE__ERROR << _SMP__NOPARSER << endm;
      bOk = false;
      return;
   }
   if (!ysp.openDocument(solver)) {
      erh << ECE__ERROR << _SMP__CANTPARSE << options.inqPrimaryPath() << endm;
      bOk = false;
      return;
   }
   iDepthElt = 0;                          // Element nesting depth
   eidCurrent = 0;                         // Current EID number

   erh << _SMP__BIGSEPA <<  endm;
   while (ysp.isMoreToParse() && (bOk)) {
      YaspEvent const & ev = ysp.next();
      ev.accept(*this);            // do anything specific
      if (!options.isPrintOnHold(ysp, eidCurrent)) {
         reporter.printRegular(ev, iDepthElt, eidCurrent);
      }
   }
   erh << _SMP__BIGSEPA <<  endm;
   erh << _SMP__ENDREVIEW << options.inqPrimaryPath() << eidCurrent << endm;
   return;
}

/*----------------------------------------------------------Sample01::bumpEid-+
| Also: report progress through the file.                                     |
+----------------------------------------------------------------------------*/
inline void Sample01::bumpEid() {
   ++eidCurrent;
   if ((options.inqInterval()) && (!(eidCurrent % options.inqInterval()))) {
      erh << _SMP__CHKPT
          << eidCurrent << ysp.inqLine() << ysp.inqEntity().inqName()
          << endm;
   }
}

/*-------------------------------------------------------Sample01::inspectors-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Sample01::inspectTagEmpty(Tag const & tag) {
   bumpEid();
}

void Sample01::inspectTagStart(Tag const & tag) {
   bumpEid();
   if (++iDepthElt > MAX_TAGDEPTH) {
      erh << ECE__ERROR << _SMP__BIGLVL << iDepthElt << endm;
      bOk = false;
   }
}
void Sample01::inspectTagEnd(Tag const &) {
   --iDepthElt;
}
void Sample01::inspectSdataText(TextData const & text) {
   bumpEid();
}
void Sample01::inspectCdataText(TextData const & text) {
   bumpEid();
}
void Sample01::inspectText(TextData const & text) {
   bumpEid();
}
void Sample01::inspectEntityRef(Entity const & ent) {
   bumpEid();
}
void Sample01::inspectEntityStart(Entity const & ent) {
}
void Sample01::inspectEntityCantOpen(Entity const & ent) {
   erh << _SMP__CANTOPENENT << ent.inqName() << endm;
}


/*------------------------------------------------------------ class Watcher -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Watcher : public UnicodeComposer::Watcher {
private:
   bool notifyNewEntry(UnicodeComposer::Message & msg) {
      cerr << msg.stringize() << endl;
      return true;
   }
};

/*-----------------------------------------------------------------------main-+
|                                                                             |
+----------------------------------------------------------------------------*/
int main(int argc, char const * const * argv)
{
   UnicodeComposer erh;
   erh.pushWatcher(new Watcher);

   if ((argc == 1) || ((argv[1][0] == '?') && (argc == 2))) {
      Sample01::Options::usage(erh);
   }else {
      Sample01 review(erh, argc, argv);
      if (review) {
         review.run();
      }
   }
   return 0;
}

/*===========================================================================*/
