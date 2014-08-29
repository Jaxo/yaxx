/* $Id: Clauses.cpp,v 1.64 2002-11-13 21:06:24 jlatone Exp $ */

#include <string.h>

#include "../toolslib/uccompos.h"
#include "../toolslib/SystemContext.h"
#include "Constants.h"
#include "Tracer.h"
#include "Clauses.h"
#include "IOMgr.h"

#ifdef __MWERKS_NEEDS_REALLOC_FIX__
#define realloc reallocFix4Mwerks
extern void *reallocFix4Mwerks(void *ptr, size_t size);
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

#if defined MWERKS_NEEDS_ADDR_FIX  // getenv
   char const * const Clauses::m_rxlib = 0;
#else
   char const * const Clauses::m_rxlib = "RXLIB";
#endif

/*----------------------------------------------------Clauses::RxFile::RxFile-+
|                                                                             |
+----------------------------------------------------------------------------*/
Clauses::RxFile::RxFile(
   RexxString const & strPath,
   istream & input,
   istream * pInput
) :
   m_input(input),
   m_pInput(pInput),
   m_strPath(strPath),
   m_totalLines(0)
{
}

/*---------------------------------------------------Clauses::RxFile::~RxFile-+
|                                                                             |
+----------------------------------------------------------------------------*/
Clauses::RxFile::~RxFile()
{
   delete m_pInput;
}

/*-----------------------------------------------------------Clauses::Clauses-+
|                                                                             |
+----------------------------------------------------------------------------*/
Clauses::Clauses(char const * pgmName) : m_pgmName(pgmName)
{
   m_isJustStarted = true;
   m_sizeClauses = 0;
   m_pCur = 0;
   #ifdef __MWERKS__
   m_sb.open("rxclause");
   m_sb.pubseekpos(0);
   m_sb.pubsetbuf(0, 64);
   #endif
}

/*----------------------------------------------------------Clauses::~Clauses-+
|                                                                             |
+----------------------------------------------------------------------------*/
Clauses::~Clauses()
{
   if (m_pCur) {
      delete m_pCur;
   }
   #ifdef __MWERKS__
   m_sb.pubseekpos(0);
   m_sb.close();
   #endif
}

/*-------------------------------------------------------Clauses::writeClause-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Clauses::writeClause(Clause & clause)
{
   m_sb.sputn((char const *)&clause, sizeof clause);
   ++m_sizeClauses;
}

/*--------------------------------------------------------Clauses::readClause-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Clauses::readClause(int no, Clause & clause) const
{
   ((Clauses *)this)->m_sb.pubseekpos(no * sizeof(Clause), ios::in);
   ((Clauses *)this)->m_sb.sgetn((char *)&clause, sizeof clause);
}

/*------------------------------------------------------Clauses::makeNewInput-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream * Clauses::makeNewInput(RexxString const & strFilePath)
{
   RexxString strRealFilePath(strFilePath);
   istream * pInput = makeInputStream(strRealFilePath);
   int max = m_rxfList.size();
   LinkedList::Iterator it((LinkedList &)m_rxfList); // violates constness
   for (int ix=0; ix < max; ++ix) {
      RxFile const * pRxf = (RxFile *)it();
      if (pRxf->m_strPath == strRealFilePath) {
         if (pInput != &SystemContext::cin()) delete pInput;
         return 0;
      }
   }
   m_rxfList.add(new RxFile(strRealFilePath, *pInput, pInput));
   return pInput;
}

/*-------------------------------------------------------------Clauses::start-+
| The input file can either have been created by makeNewInput (in which case  |
| it is registered in the rxfList), or it can be a file externally owned.     |
+----------------------------------------------------------------------------*/
void Clauses::start(
   istream & input,
   RexxString const & strFilePath,
   UnicodeComposer & erh
) {
   m_isJustStarted = true;
   int max = m_rxfList.size();
   int ix = 0;
   RxFile * pRxf;
   LinkedList::Iterator it((LinkedList &)m_rxfList); // violates constness

   for (;;) {
      if (ix == max) {
         pRxf = new RxFile(strFilePath, input, 0);
         m_rxfList.add(pRxf);
         break;
      }else if (pRxf = (RxFile *)it(), pRxf->m_pInput == &input) {
         break;
      }else {
         ++ix;
      }
   }
   m_pCur = new Current();
   m_pCur->m_rxfNo = ix;
   if (!input.good()) {
      erh << ECE__ERROR << _REX__3_0 << endm;
   }
}

/*---------------------------------------------------------Clauses::addClause-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Clauses::addClause(
   int lineno,
   int nesting,
   streampos startPos
) {
   assert (m_pCur);                // clauses must be started
   Clause &  clause = m_pCur->m_clause;
   if (m_isJustStarted) {
      m_isJustStarted = false;
   }else {
      // be carefull: do not assume (clauses[i-1].pEnd == clauses[i].pStart)
      // source can be sparse in memory.  (bug fix)
      clause.m_len = (unsigned short)(startPos - clause.m_startPos);
      writeClause(clause);
   }
   clause.m_startPos = startPos;
   clause.m_lineno = lineno;
   clause.m_nesting = nesting;
   clause.m_rxfNo  = m_pCur->m_rxfNo;
   return m_sizeClauses;
}

/*---------------------------------------------------------------Clauses::end-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Clauses::end(streampos endPos, int lineno)
{
   assert (m_pCur);                // clauses must be started
   Clause & clause = m_pCur->m_clause;
   ((RxFile *)m_rxfList.get(m_pCur->m_rxfNo))->m_totalLines = lineno-1;
   if (!m_isJustStarted) {
      clause.m_len = (unsigned short)(endPos - clause.m_startPos);
   }
   writeClause(clause);
   delete m_pCur;
   m_pCur = 0;
}

/*------------------------------------------------------------Clauses::locate-+
| This locate occurs during compilation                                       |
+----------------------------------------------------------------------------*/
Location const & Clauses::locate(
   streampos startPos,
   Location & loc
) const {
   assert (m_pCur);                // clauses must be started
   RxFile const * pRxf = (RxFile *)((Clauses *)this)->m_rxfList.get(
      m_pCur->m_rxfNo
   );
   istream & input = pRxf->m_input;
   streampos pos;

   loc.m_path = pRxf->m_strPath;
   if (m_isJustStarted) {
      loc.m_lineno = 0;
      // violates constness
      pos = 0;
      loc.m_nesting = 0;
   }else {
      Clause & clause = m_pCur->m_clause;
      loc.m_lineno = clause.m_lineno - 1;
      pos  = clause.m_startPos;
      startPos -= pos;
      loc.m_nesting = clause.m_nesting;
   }
   if (input.seekg(pos), !input.good()) {
      loc.m_contents == RexxString::Nil;
   }else {
      // Show the line which is closest to that startPosition
      streambuf & sb = *(input.rdbuf());
      StringBuffer buf;
      do {
         ++loc.m_lineno;
         buf.empty();
         buf.append(sb, '\n');
      }while ((sb.sbumpc() != EOF) && ((startPos -= (buf.length()+1)) > 2));
      if (buf.length() && buf.charAt(buf.length()-1) == '\r') {
         buf.deleteCharAt(buf.length()-1);
      }
      loc.m_contents = RexxString(buf);
   }
   return loc;
}

/*------------------------------------------------------------Clauses::locate-+
| This locate occurs when after the Rexx script was compiled                  |
+----------------------------------------------------------------------------*/
Location const & Clauses::locate(
   int clauseNo,
   int nesting,
   Location & loc
) const {
   assert (m_pCur == 0);           // clauses must be ended

   StringBuffer sb;
   Clause clause;
   readClause(clauseNo, clause);
   RxFile const * pRxf = (RxFile *)((Clauses *)this)->m_rxfList.get(
      clause.m_rxfNo
   );
   istream & input = pRxf->m_input;

   loc.m_nesting = nesting + clause.m_nesting;
   loc.m_path = pRxf->m_strPath;
   loc.m_lineno = clause.m_lineno;
   if (input.seekg(clause.m_startPos), input.good()) {
      for (int len = clause.m_len; len; --len) {
         int c = input.get();
         if ((c == EOF) || (c == '\r') || (c == '\n')) break;
         sb.append((char)c);
      }
   }
   loc.m_contents = RexxString(sb);
   return loc;
}

/*---------------------------------------------------------Clauses::getLineAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Clauses::getLineAt(int lineno) const
{
   int i;
   int startingLineno = 1;
   streampos pos = 0;
   StringBuffer sb;
   RxFile const * pRxf = getMainRxFile();
   istream & input = pRxf->m_input;

   for (i=0;  i < m_sizeClauses; ++i) {
      // walk the main file (rxfNo == 0), stop at the clause that follows
      Clause clause;
      readClause(i, clause);
      if ((clause.m_lineno >= lineno) || clause.m_rxfNo) break;
      startingLineno = clause.m_lineno;
      pos = clause.m_startPos;
   }
   input.seekg(pos);
   while (startingLineno < lineno) {
      while (i = input.get(), i != '\n') {
         if (i == EOF) return RexxString::Nil; // Sigh... too big!
      }
      ++startingLineno;
   }
   while (i = input.get(), (i != EOF) && (i != '\r') && (i != '\n')) {
      sb.append((char)i);
   }
   return sb;
}

/*STATIC---------------------------------------------Clauses::makeInputStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream * Clauses::makeInputStream(RexxString & strPath)
{
   if (!strPath.exists() || !strPath.length()) {
      strPath = IOMgr::strCin;
      return &SystemContext::cin();
   }else {
      istream * pInput = SystemContext::makeStream(strPath, ios::in);
      if (!pInput->good()) {
         int len = m_rxlib? 0 : strlen(m_rxlib);
         if (len) {
            delete pInput;
            int lenPath = strPath.length();
            char * temp = new char[len + 1 + lenPath + 1];
            memcpy(temp, m_rxlib, len);
            if (temp[len-1] != Constants::FILE_SEPARATOR) {
               temp[len++] = Constants::FILE_SEPARATOR;
            }
            memcpy(temp+len, strPath, lenPath);
            strPath.assign(temp, len+lenPath);
            delete [] temp;
            pInput = SystemContext::makeStream(strPath, ios::in);
         }
      }else {
         // look for a shebang
         if (pInput->peek() == '#') {
            // a bit of paranoia in what's below...
            pInput->get();
            if (pInput->peek() == '!') {
               while (pInput->get() != '\n');
            }else {
               pInput->putback('#');
            }
         }
      }
      return pInput;
   }
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
