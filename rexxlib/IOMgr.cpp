/* $Id: IOMgr.cpp,v 1.67 2003-01-21 16:14:23 pgr Exp $ */

#if defined _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "IOMgr.h"
#include "../toolslib/SystemContext.h"
#include "../toolslib/ConsoleSchemeHandler.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

RexxString IOMgr::strCin(ConsoleSchemeHandler::cinUri);

/*--------------------------------------------------IOMgr::UriEntry::UriEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry::UriEntry(RexxString & name, ios__openmode om) :
   m_name(name),
   m_uri(name, SystemContext::getBaseUri()),
   m_line(1),
   m_posSave(0)
{
   if (om & ios::out) {
      m_pStream = m_uri.getStream(ios::out);
      m_mode = WRITE_MODE + WRITE_GRANTED;
   }else {
      m_pStream = m_uri.getStream(ios::in);
      m_mode = READ_MODE;
   }
}

/*--------------------------------------------------IOMgr::UriEntry::UriEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry::UriEntry(int fh) :
   m_line(1),
   m_uri(),
   m_posSave(0)
{
   ios__openmode om;
   switch (fh) {
   case IX_STDIN:
      om = ios::in;
      m_mode = READ_MODE | WRITE_DENIED;
      m_name = ConsoleSchemeHandler::cinUri;
      m_uri = RegisteredURI(m_name);
      m_pStream = &SystemContext::cin();
      break;
   case IX_STDOUT:
      om = ios::out;
      m_mode = WRITE_MODE | READ_DENIED;
      m_name = ConsoleSchemeHandler::coutUri;
      m_uri = RegisteredURI(m_name);
      m_pStream = &SystemContext::cout();
      break;
   case IX_STDERR:
      om = ios::out;
      m_mode = WRITE_MODE | READ_DENIED;
      m_name = ConsoleSchemeHandler::cerrUri;
      m_uri = RegisteredURI(m_name);
      m_pStream = &SystemContext::cerr();
      break;
   default:
      assert (false);
   }

}

/*-------------------------------------------------IOMgr::UriEntry::~UriEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry::~UriEntry() {
   delete m_pStream;
   /*
   | >>>PGR: TODO (it better be done on the caller's side)
   |
   | #if defined _WIN32 (not defined MWERKS???)
   | #define ftruncate chsize
   | #endif
   |
   |
   | bool isEmpty = false;
   | if (file is opened for writing) {
   |    long posWrite = (get Write Position)
   |    if (!posWrite) {
   |       isEmpty = true;
   |    }else {
   |       ftruncate(m_fd, poswrite);
   |    }
   | }
   | >>>PGR: TODO
   | if (isEmpty) remove(path);
   */
}

/*----------------------------------------------------IOMgr::UriEntry::getBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
streambuf * IOMgr::UriEntry::getBuf() {
   return m_pStream->rdbuf();
}

/*-----------------------------------------------IOMgr::UriEntry::ensureWrite-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry * IOMgr::UriEntry::ensureWrite()
{
   if (m_mode & WRITE_MODE) {
      return this;
   }else if (m_mode & WRITE_DENIED) {
      return 0;
   }else {
      // we are in READ mode.  And want to go to WRITE mode.  Not always easy
      long posSave = m_pStream->tellg();
      if (!(m_mode & WRITE_GRANTED)) {          // a rdonly stream (the worst)
         delete m_pStream;
         m_pStream = m_uri.getStream(ios::out); // reopen it read write
         if (!m_pStream->good()) {              // no good!
            delete m_pStream;
            m_pStream = m_uri.getStream(ios::in); // reopen it read only
            m_pStream->seekg(posSave);
            m_mode |= WRITE_DENIED;             // now, we know the bad news
            return 0;                           // sigh.
         }else {                                // good!
            m_mode |= WRITE_GRANTED;            // now, we know the good news
         }
         m_pStream->seekp(0, ios::end);         // seek at the end
      }else {
         m_pStream->seekp(m_posSave); // REXX has different read and write pos
      }
      m_posSave = posSave;
      m_mode &= ~READ_MODE;
      m_mode |= WRITE_MODE;
      return this;
   }
}

/*------------------------------------------------IOMgr::UriEntry::ensureRead-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry * IOMgr::UriEntry::ensureRead()
{
   if (m_mode & READ_MODE) {
      return this;
   }else if (m_mode & READ_DENIED) {
      return 0;
   }else {
      // we are in WRITE mode.  And want to go to READ mode.  Easy.
      long posSave = m_pStream->tellp();
      m_pStream->seekg(m_posSave); // REXX has different read and write pos
      m_posSave = posSave;
      m_mode &= ~WRITE_MODE;
      m_mode |= READ_MODE;
      return this;
   }
}

/*---------------------------------------------------------------IOMgr::IOMgr-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
IOMgr::IOMgr() : m_iCache(0) {
   #if defined _WIN32
   _setmode(IX_STDIN, _O_BINARY);
   _setmode(IX_STDOUT, _O_BINARY);
   _setmode(IX_STDERR, _O_BINARY);
   #endif

   m_entries.add(IX_STDIN, m_entryCache = new UriEntry(IX_STDIN));
   m_entries.add(IX_STDOUT,new UriEntry(IX_STDOUT));
   m_entries.add(IX_STDERR, new UriEntry(IX_STDERR));
}

/*--------------------------------------------------------------IOMgr::~IOMgr-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
IOMgr::~IOMgr()
{
   LinkedList::Iterator it(m_entries);
   for (int i=0; i < 3; ++i) {
      ((UriEntry *)it())->m_pStream = 0;  // avoid to close the system files!
   }
}

/*-----------------------------------------------------------IOMgr::findEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry * IOMgr::findEntry(RexxString & name)
{
   assert (name.exists());
   if ((m_iCache >= 0) && (name == m_entryCache->m_name)) {
      return m_entryCache;
   }else {
      int max = m_entries.size();
      LinkedList::Iterator it(m_entries);
      for (int ix=0; ix < max; ++ix) {
         UriEntry * entry = (UriEntry *)it();
         if (name == entry->m_name) {
            m_entryCache = entry;
            m_iCache = ix;
            return entry;
         }
      }
   }
   return 0;
}

/*-----------------------------------------------------------IOMgr::makeEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry * IOMgr::makeEntry(RexxString & name, ios__openmode om)
{
   UriEntry * entry;
   try {
      entry = new UriEntry(name, om);
   }catch (URI::MalformedException) {
      return 0;
   }
   if (!entry->m_pStream->good()) {
      delete entry;
      return 0;
   }else {
      m_entries.add(entry);
      m_entryCache = entry;
      m_iCache = m_entries.size() - 1;
      if (om & ios::out) {
         entry->m_pStream->seekp(0, ios::end);  // seek at the end
      }
      return entry;
   }
}

/*----------------------------------------------------------IOMgr::ensureRead-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry * IOMgr::ensureRead(RexxString & name)
{
   if (name.length()) {
      UriEntry * entry = findEntry(name);
      if (!entry) {
         return makeEntry(name, ios::in);
      }else {
         return entry->ensureRead();
      }
   }else {
      return (UriEntry *)m_entries.get(IX_STDIN);
   }
}

/*---------------------------------------------------------IOMgr::ensureWrite-+
|                                                                             |
+----------------------------------------------------------------------------*/
IOMgr::UriEntry * IOMgr::ensureWrite(RexxString & name)
{
   if (name.length()) {
      UriEntry * entry = findEntry(name);
      if (!entry) {
         return  makeEntry(name, ios::out);
      }else {
         return entry->ensureWrite();
      }
   }else {
      return (UriEntry *)m_entries.get(IX_STDOUT);
   }
}

/*----------------------------------------------------------IOMgr::ensureOpen-+
|                                                                             |
+----------------------------------------------------------------------------*/
Signaled IOMgr::ensureOpen(RexxString & name, ios__openmode om)
{
   UriEntry * entry = findEntry(name);
   if (entry) {
      if (entry->m_pStream && entry->m_pStream->good()) {
         return SIG_NONE;
      }else {
         return SIG_NOTREADY;
      }
   }else {
      if (!makeEntry(name, om)) {
         return SIG_NOTREADY;
      }else {
         return SIG_NONE;
      }
   }
}

/*---------------------------------------------------------IOMgr::ensureClose-+
|                                                                             |
+----------------------------------------------------------------------------*/
Signaled IOMgr::ensureClose(RexxString & name)
{
   UriEntry * entry = findEntry(name);
   if (entry) {
      m_entries.remove(m_iCache);
      m_iCache = -1;
      return SIG_NONE;
   }else {
//    return SIG_NOTREADY;     // it looks OK to close an unexistent file
      return SIG_NONE;
   }
}

/*------------------------------------------------------------IOMgr::readLine-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString IOMgr::readLine() {
   bool isEof;
   return readLine(isEof, *SystemContext::cin().rdbuf());
}

RexxString IOMgr::readLine(bool & isEof, streambuf & sb)
{
   StringBuffer buf(100);
   buf.append(sb, '\n');
   isEof = (sb.sbumpc() == -1); // EOF also, this skips the ending delimiter
   if (buf.length() && ('\r' == buf.charAt(buf.length()-1))) {
      buf.setLength(buf.length() - 1);
   }
   return RexxString(buf);
}

/*-----------------------------------------------------------IOMgr::writeLine-+
|                                                                             |
+----------------------------------------------------------------------------*/
void IOMgr::writeLine(RexxString const & str) {
   streambuf * pSb = SystemContext::cout().rdbuf();
   pSb->sputn(str, str.length());
   pSb->sputc('\n');
}

void IOMgr::writeLine(RexxString const & str, streambuf & sb)
{
   sb.sputn(str, str.length());
   sb.sputc('\n');
}

/*----------------------------------------------------------IOMgr::streamCmnd-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString IOMgr::streamCmnd(
   RexxString & name,
   RexxString & cmdArg,
   Signaled & sig
) {
   RexxString cmd;
   cmd.strip(
      RexxString(cmdArg, CharConverter::Upper),
      RexxString::KillBOTH, ' '
   );
   if (cmd == Constants::STR_STREAM_OPEN) {
      sig = ensureOpen(name, ios::in);
   }else if (cmd == Constants::STR_STREAM_CLOSE) {
      sig = ensureClose(name);
   }else if (cmd == Constants::STR_STREAM_FLUSH) {
      UriEntry * entry = findEntry(name);
      if (!entry) {
         sig = SIG_NOTREADY;
      }else if (-1 == entry->getBuf()->pubsync()) { // EOF
         sig = SIG_NOTREADY;
      }else {
         sig = SIG_NONE;
      }
   }else if (cmd == Constants::STR_STREAM_RESET) {
      UriEntry * entry = findEntry(name);
      if (!entry) {
         sig = SIG_NOTREADY;
      }else if (entry->getBuf()->pubseekpos(0, ios::in)) {
         sig = SIG_NOTREADY;
      }else {
         sig = SIG_NONE;
      }
   }else {
      sig = SIG_ERROR;
   }
   return RexxString("");
}

/*---------------------------------------------------------------IOMgr::state-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString & IOMgr::state(RexxString & name) {
   UriEntry * entry = ensureRead(name);
   if (!entry) {
      return Constants::STR_UNKNOWN;
   }else if (entry->m_pStream->good()) {
      return Constants::STR_READY;
   }else if (!entry->m_pStream->fail() && entry->m_pStream->eof()) {
      return Constants::STR_NOTREADY;
   }else {
      return Constants::STR_ERROR;
   }
}

/*------------------------------------------------------------IOMgr::describe-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString & IOMgr::describe(RexxString & name)
{
// UriEntry * entry = findEntry(name);
// if (ferror(entry->m_f)) {
//   return Constants::STR_ERROR;
// }
   return state(name);
}

/*---------------------------------------------------------------IOMgr::chars-+
|                                                                             |
+----------------------------------------------------------------------------*/
int IOMgr::chars(RexxString & name, Signaled & sig)
{
   UriEntry * entry = ensureRead(name);
   if (!entry) {
      sig = SIG_NOTREADY;
      return 0;
   }else {
      streambuf * pSb = entry->getBuf();
      long curPos = pSb->pubseekoff(0, ios::cur, ios::in);
      if (curPos != -1) { // EOF
         long endPos = pSb->pubseekoff(0, ios::end, ios::in);
         if (
            (-1 != pSb->pubseekoff(curPos, ios::beg, ios::in)) && // EOF
            (-1 != endPos) // EOF
         ) {
            sig = SIG_NONE;
            return endPos - curPos;
         }
      }
      sig = SIG_ERROR;
      return 1;
   }
}

/*--------------------------------------------------------------IOMgr::charin-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString IOMgr::charin(
   RexxString & name, int start, int length, Signaled & sig
) {
   UriEntry * entry = ensureRead(name);
   if (!entry) {
      sig = SIG_NOTREADY;
   }else {
      streambuf * pSb = entry->getBuf();
      if (
         (start > 0) &&
         (-1 == pSb->pubseekoff(start-1, ios::beg, ios::in)) // EOF
      ) {
         sig = SIG_ERROR;
      }else {
         sig = SIG_NONE;
         switch (length) {
         case 0:
            break;
         case 1:
            {
               int c = pSb->sbumpc();
               if (c != -1) return RexxString((char)c); // EOF
            }
         default:
            {
               StringBuffer buf(length);
               buf.append(*pSb, length);
               return RexxString(buf);
            }
         }
      }
   }
   return RexxString::Nil;
}

/*-------------------------------------------------------------IOMgr::charout-+
|                                                                             |
+----------------------------------------------------------------------------*/
int IOMgr::charout(
   RexxString & name, RexxString & strOut, int start, Signaled & sig
) {
   UriEntry * entry = ensureWrite(name);
   int len = strOut.length();
   if (!entry) {
      sig = SIG_NOTREADY;
   }else {
      streambuf * pSb = entry->getBuf();
      if (
         (start > 0) &&
         (-1 == pSb->pubseekoff(start-1, ios::beg, ios::out)) // EOF
      ) {
         sig = SIG_ERROR;
      }else {
         if (len) {
            pSb->sputn(strOut, len);
            pSb->pubsync();
         }
         sig = SIG_NONE;
         return 0;  // number of chars remaining to write
      }
   }
   return len;
}

/*------------------------------------------------------------IOMgr::gotoLine-+
|                                                                             |
+----------------------------------------------------------------------------*/
int IOMgr::gotoLine(streambuf * pSb, int from, int to, ios__openmode om)
{
   bool isCrBefore = true;
   int c;
   if (from > to) {
      from = 1;
      if (-1 == pSb->pubseekoff(0, ios::beg, om)) { // EOF
         return -1;
      }
   }
   while (from < to) {
      if (c = pSb->sbumpc(), c == -1) { // EOF
         if (!isCrBefore) ++from;
         break;
      }
      isCrBefore = (c == '\n');
      if (isCrBefore) ++from;
   }
   return from;
}

/*---------------------------------------------------------------IOMgr::lines-+
|                                                                             |
+----------------------------------------------------------------------------*/
int IOMgr::lines(RexxString & name, Signaled & sig)
{
   UriEntry * entry = ensureRead(name);
   if (!entry) {
      sig = SIG_NOTREADY;
      return 0;
   }else {
      streambuf * pSb = entry->getBuf();
      long curPos = pSb->pubseekoff(0, ios::cur, ios::in);
      if (curPos != -1) { // EOF
         int res = gotoLine(pSb, 0, INT_MAX, ios::in);
         if (-1 != pSb->pubseekoff(curPos, ios::beg, ios::in)) { // EOF
            sig = SIG_NONE;
            return res;
         }
      }
      sig = SIG_ERROR;
      return 1;
   }
}

/*--------------------------------------------------------------IOMgr::linein-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString IOMgr::linein(
   RexxString & name, int startLine, int count, Signaled & sig
) {
   UriEntry * entry = ensureRead(name);
   if (!entry) {
      sig = SIG_NOTREADY;
      return RexxString::Nil;
   }else {
      streambuf * pSb = entry->getBuf();
      sig = SIG_NONE;
      if (startLine > 0) {
         entry->m_line = gotoLine(pSb, entry->m_line, startLine, ios::in);
         if (-1 == pSb->sgetc()) { // EOF
            return RexxString::Nil;
         }
      }
      if (count == 0) {
         return RexxString::Nil;
      }else {
         bool isEof;
         RexxString res = readLine(isEof, *pSb);
         if (!isEof) ++entry->m_line;
         return res;
      }
   }
}

/*-------------------------------------------------------------IOMgr::lineout-+
|                                                                             |
+----------------------------------------------------------------------------*/
int IOMgr::lineout(
   RexxString & name, RexxString & strOut, int startLine, Signaled & sig
) {
   UriEntry * entry = ensureWrite(name);
   if (!entry) {
      sig = SIG_NOTREADY;
      return 1;
   }else {
      sig = SIG_NONE;
      streambuf * pSb = entry->getBuf();

      if (startLine > 0) {
         entry->m_line = gotoLine(pSb, entry->m_line, startLine, ios::out);
      }
      writeLine(strOut, *pSb);
      ++entry->m_line;
      return 0;      // number of lines remaining to write
   }
}


#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
