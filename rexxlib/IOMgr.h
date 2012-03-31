/* $Id: IOMgr.h,v 1.27 2003-01-21 16:14:23 pgr Exp $ */

#ifndef COM_JAXO_YAXX_IOMGR_H_INCLUDED
#define COM_JAXO_YAXX_IOMGR_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Constants.h"
#include "../toolslib/LinkedList.h"
#include "../toolslib/RegisteredURI.h"
#include "RexxString.h"
#include "Exception.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-------------------------------------------------------------- class IOMgr -+
|                                                                             |
+----------------------------------------------------------------------------*/
class IOMgr : public Constants {
public:
   IOMgr();
   ~IOMgr();
   RexxString & state(RexxString & name);
   RexxString & describe(RexxString & name);
   RexxString streamCmnd(RexxString & name, RexxString & cmnd, Signaled & sig);
   RexxString charin(RexxString & name, int start, int length, Signaled & sig);
   RexxString linein(RexxString & name, int start, int length, Signaled & sig);
   int charout(RexxString & name, RexxString & strOut, int start, Signaled & sig);
   int lineout(RexxString & name, RexxString & strOut, int start, Signaled & sig);
   int chars(RexxString & name, Signaled & sig);
   int lines(RexxString & name, Signaled & sig);

   static RexxString readLine();  // from the console (aka cin, stdin)
   static RexxString readLine(bool & isEof, streambuf & sb);
   static void writeLine(RexxString const & str); // to the console
   static void writeLine(RexxString const & str, streambuf & cb);

   static RexxString strCin;

   enum {
     IX_STDIN = 0,
     IX_STDOUT = 1,
     IX_STDERR = 2
   };

private:
   class UriEntry : public LinkedList::Item {
   public:
      enum {
         READ_MODE      = 1 << 0,
         WRITE_MODE     = 1 << 1,
         WRITE_GRANTED  = 1 << 2,
         READ_DENIED    = 1 << 3,
         WRITE_DENIED   = 1 << 4
      };
      UriEntry(RexxString & name, ios__openmode om);
      UriEntry(int fh);   // system files (0, 1, 2)
      ~UriEntry();

      streambuf * getBuf();
      UriEntry * ensureWrite();
      UriEntry * ensureRead();

      RexxString m_name;
      RegisteredURI m_uri;
      long m_line;
      long m_posSave;      // save ot the seek position for the other mode
      char m_mode;         // if m_om is ios::in, m_posSave is the write pos.
      iostream * m_pStream;
   };

   LinkedList m_entries;
   UriEntry * m_entryCache;
   int m_iCache;

   UriEntry * makeEntry(RexxString & name, ios__openmode om);
   Signaled ensureOpen(RexxString & name, ios__openmode om);
   Signaled ensureClose(RexxString & name);
   UriEntry * ensureRead(RexxString & name);
   UriEntry * ensureWrite(RexxString & name);
   static int gotoLine(streambuf * pSb, int from, int to, ios__openmode);
   UriEntry * findEntry(RexxString & name);
};

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
