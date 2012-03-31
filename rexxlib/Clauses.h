/* $Id: Clauses.h,v 1.30 2002-09-25 14:27:35 jlatone Exp $ */

#ifndef COM_JAXO_YAXX_CLAUSES_H_INCLUDED
#define COM_JAXO_YAXX_CLAUSES_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <stdio.h>
#include "RexxString.h"
#include "../toolslib/LinkedList.h"
class UnicodeComposer;

#ifdef __MWERKS__
#include "PalmFileStream.h"
#else
#include "../toolslib/MemStream.h"
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class Location;

/*------------------------------------------------------------ class Clauses -+
|                                                                             |
+----------------------------------------------------------------------------*/
class REXX_API Clauses {
public:
   Clauses(char const * pgmName);
   ~Clauses();

   void start(
      istream & input,
      RexxString const & strFilePath,
      UnicodeComposer & erh
   );
   int addClause(int lineno, int nesting, streampos startPos);
   Location const & locate(streampos startPos, Location & loc) const;
   void end(streampos endPos, int lineno);

   Location const & locate(int clauseNo, int nesting, Location & loc) const;
   RexxString const & getMainFileName() const;
   istream * makeNewInput(RexxString const & strFilePath);
   RexxString & getProgramName();
   int getSourceLine(int clauseNo) const;
   int getTotalLines() const;
   RexxString getLineAt(int lineno) const;

private:
   class RxFile : public LinkedList::Item {  // Rexx Script
   public:
      RxFile(RexxString const & strPath, istream & input, istream * pInput);
      ~RxFile();
      istream & m_input;            // associated stream
      istream * m_pInput;           // 0 if we don't own it
      RexxString m_strPath;         // file path
      int m_totalLines;             // total number of lines
   };
   struct Clause {
      streampos m_startPos;         // where source starts
      unsigned short m_len;         // max length of the clause
      unsigned short m_lineno;      // line number in source file
      unsigned char m_nesting;      // nesting level
      unsigned char m_rxfNo;        // RxFile number
   };
   struct Current {                 // when a Rexx script is parsed
      int m_rxfNo;                  // current file no
      Clause m_clause;              // current clause
   };
   #ifdef __MWERKS__
      PalmFileStreamBuf m_sb;
   #else
      MemStreamBuf m_sb;
   #endif
   bool m_isJustStarted;
   RexxString m_pgmName;
   LinkedList m_rxfList;
   Current * m_pCur;
   int m_sizeClauses;
   static char const * const m_rxlib;

   RxFile const * getMainRxFile() const;
   void writeClause(Clause & clause);
   void readClause(int no, Clause & clause) const;
   static istream * makeInputStream(RexxString & strPath);
};

inline Clauses::RxFile const * Clauses::getMainRxFile() const {
   // violates constness
   return (RxFile *)((Clauses *)this)->m_rxfList.getFirst();
}

inline int Clauses::getSourceLine(int clauseNo) const {
   Clause clause;
   readClause(clauseNo, clause);
   return clause.m_lineno;
}

inline RexxString const & Clauses::getMainFileName() const {
   return getMainRxFile()->m_strPath;
}

inline int Clauses::getTotalLines() const {
   return getMainRxFile()->m_totalLines;
}

inline RexxString & Clauses::getProgramName() {
   return m_pgmName;
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
