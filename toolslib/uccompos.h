/*
* $Id: uccompos.h,v 1.13 2011-07-29 10:26:38 pgr Exp $
*
* Class to compose UnicodeString's using a Template
*/

#ifndef COM_JAXO_TOOLS_UCCOMPOS_H_INCLUDED
#define COM_JAXO_TOOLS_UCCOMPOS_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"
#include "Writer.h"
#include "arglist.h"
#include "tplist.h"
#include "../reslib/resources.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

enum ErrorSeverity {
   ECE__INFO,
   ECE__WARNING,
   ECE__ERROR,
   ECE__SEVERE,
   ECE__FATAL,
   ECE__DEBUG
};

/*------------------------------------------------------------UnicodeComposer-+
| UnicodeComposer's allow to compose a Message out of a Template              |
| and a variable number of elementary arguments.                              |
|                                                                             |
| It is analoguous to printf(Template, arg1, arg2, ...)                       |
| with the following differences:                                             |
|  - the template can reference the arguments in any order:                   |
|    the argument at the 3rd position is referenced by "&3;"                  |
|    (compare with %s)                                                        |
|  - the arguments are not typed (compare with %s, vs %i)                     |
|    the conversion is made at compile time                                   |
|                                                                             |
| A UnicodeComposer object builds Messages by first collecting its components |
| (template, argument(s)), then calling the virtual "finishMessage()"         |
| when all the components of the message are ready to be assembled.           |
| It then cleans up the place and gets ready to produce another Message.      |
|                                                                             |
| The methods to assemble the Message belong to the Message itself.           |
|                                                                             |
| How to Use?                                                                 |
|                                                                             |
| 1) First create a class, deriving from UnicodeComposer,                     |
|    to define the "finishMessage" method: what to do when the                |
|    string is finished, ready to be assembled.                               |
|                                                                             |
|    Example:                                                                 |
|                                                                             |
|      class MyStringPrinter : public UnicodeComposer {                       |
|         void finishMessage(Message & msg) {                                 |
|            cerr << msg.stringize() << endl;  // assume latin                |
|         }                                                                   |
|      };                                                                     |
|                                                                             |
| 2) Using the insertion operator, you are now able to                        |
|    assemble a string and print it via your "finishMessage" method.          |
|                                                                             |
|    Example:                                                                 |
|                                                                             |
|      MyStringPrinter print;                                                 |
|                                                                             |
|      print << "&1; will be in at &2;:&3"     // template                    |
|            << "Dogbert" << 12 << 30          // arguments                   |
|            << endm;                          // end of message (print it)   |
|                                                                             |
| Note:                                                                       |
|     The template above is "hardwired" and this use should be deprecated.    |
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeComposer {
   friend TOOLS_API UnicodeComposer & endm(UnicodeComposer & cps);

public:
   class Message;
   class Template;
   class Watcher;

   /*---------------+
   | Public Methods |
   +---------------*/
   UnicodeComposer();
   virtual ~UnicodeComposer();

   UnicodeComposer & operator<<(Template const &);
   UnicodeComposer & operator<<(ErrorSeverity);
   UnicodeComposer & operator<<(MsgTemplateId);
   UnicodeComposer & operator<<(UnicodeString const &);
   UnicodeComposer & operator<<(char const *);
   UnicodeComposer & operator<<(NamedArg const &);
   UnicodeComposer & operator<<(UnicodeComposer& (*f)(UnicodeComposer&));

#ifdef OS__IRIX   // Compiler bug - SGI5 is confused with op<<(enums) !!!!
   UnicodeComposer & operator<<(int i);
   UnicodeComposer & operator<<(unsigned int i);
   UnicodeComposer & operator<<(long i);
   UnicodeComposer & operator<<(unsigned long i);
#endif

   void setTemplate(MsgTemplateId stringId);

   void pushWatcher(Watcher * pWatcher);
   void popWatcher();

protected:
   /*------------------+
   | Protected Methods |
   +------------------*/
   virtual void finishMessage(Message & msg);
   void clear();               // get ready to produce a new message
   void endMessage();

public:
   /*--------------------+
   | Public Declarations |
   +--------------------*/
   class TOOLS_API Template {
   public:
      enum e_Status { NotSet, Resource, Self };

      Template();
      Template(char const * templateArg);
      Template(MsgTemplateId stringIdArg);

      UnicodeString assemble(
         ArgListWithName const & arglNamed,
         ArgListSimple const & arglPos,
         ErrorSeverity sev
      ) const;
      void clear();

      MsgTemplateId inqStringId() const;
      e_Status inqStatus() const;

   private:
      e_Status is;
      MsgTemplateId stringId;    // irrelevant for a self generated template
      UnicodeString ucsTemplate;

      static int convertToDecimal(UnicodeSubstring const & ucss);

      static UnicodeString const ucsDefaultTemplate;

      static UnicodeString const ucsRefArguments;
      static UnicodeString const ucsRefStringId;
      static UnicodeString const ucsRefSeverity;
      static UnicodeString const ucsArgClosings;

      static UCS_2 const ucArgOpen;
      static UCS_2 const ucArgVi;
      static UCS_2 const ucArgTab;
      static UCS_2 const ucArgClose;
   };

   class TOOLS_API Message {
      friend class UnicodeComposer;
   public:
      UnicodeString stringize() const;
      MsgTemplateId inqStringId() const;
      ErrorSeverity inqSeverity() const;
      UnicodeString const & inqArg(UnicodeString const & ucsName) const;
      UnicodeString const & inqArg(int i) const;
      ArgListWithName & inqNamedArgs();
      ArgListSimple & inqPosArgs();

   private:
      Message();
      void clear();

      Template tpl;
      ArgListWithName arglNamed;
      ArgListSimple arglPos;
      ErrorSeverity sev;
   };

   class TOOLS_API MessageList : private TpList {
   public:
      Message * operator[](int ix);
      Message const * operator[](int ix) const;

      Message * addMessage(Message const & msg);
      bool removeMessage(int ix);
      int countMessages() const;

      void addMessageList(MessageList const & list);
      void clear();
      void print(MultibyteWriter & ubost);

   private:
      class Item : public RefdItemRep, public Message {
      public:
         Item(Message const & msg) : Message(msg) {}
      };
   };

   class TOOLS_API Watcher : public RefdItemRep {
   public:
      virtual bool notifyNewEntry(Message & msg) = 0;
   };

private:
   class TOOLS_API WatcherList : public TpList {
      TP_MakeSequence(Watcher);
   };

   UnicodeComposer & operator=(UnicodeComposer const &); // no!  no way, Jose
   UnicodeComposer(UnicodeComposer const & source);      // no!

   WatcherList watchList;
   Message m_msg;
   bool bMessagePending;
};

/* -- INLINES (UnicodeComposer::Template) -- */

inline UnicodeComposer::Template::Template() {
   is = NotSet;
   stringId = _XXX__UNKNOWN;
}
inline void UnicodeComposer::Template::clear() {
   is = NotSet;
   stringId = _XXX__UNKNOWN;
   ucsTemplate = UnicodeString::Nil;
}
inline MsgTemplateId UnicodeComposer::Template::inqStringId() const {
   return stringId;
}
inline UnicodeComposer::Template::e_Status
UnicodeComposer::Template::inqStatus() const {
   return is;
}

/* -- INLINES (UnicodeComposer::Message) -- */

inline UnicodeComposer::Message::Message() : sev(ECE__INFO) {
}
inline UnicodeString UnicodeComposer::Message::stringize() const {
   return tpl.assemble(arglNamed, arglPos, sev);
}
inline MsgTemplateId UnicodeComposer::Message::inqStringId () const {
   return tpl.inqStringId();
}
inline ErrorSeverity UnicodeComposer::Message::inqSeverity () const {
   return sev;
}
inline UnicodeString const & UnicodeComposer::Message::inqArg(
   UnicodeString const & ucsName
) const {
   return arglNamed[ucsName];
}
inline UnicodeString const & UnicodeComposer::Message::inqArg(
   int i
) const {
   return arglPos[i];
}
inline ArgListWithName & UnicodeComposer::Message::inqNamedArgs() {
   return arglNamed;
}
inline ArgListSimple & UnicodeComposer::Message::inqPosArgs() {
   return arglPos;
}
inline void UnicodeComposer::Message::clear() {
   tpl.clear();
   arglNamed = ArgListWithName();
   arglPos = ArgListSimple::Nil;
   sev = ECE__INFO;
}

/* -- INLINES (UnicodeComposer) -- */

inline void UnicodeComposer::setTemplate(MsgTemplateId stringId)
{
   if (m_msg.tpl.inqStatus() != Template::NotSet) endm(*this);
   m_msg.tpl = Template(stringId);
   bMessagePending = true;
}
inline UnicodeComposer & UnicodeComposer::operator<<(
   Template const & tplArg
) {
   if (m_msg.tpl.inqStatus() != Template::NotSet) endm(*this);
   m_msg.tpl = tplArg;
   bMessagePending = true;
   return *this;
}
inline UnicodeComposer & UnicodeComposer::operator<<(
   MsgTemplateId id
) {
   // help this poor MS Visual that never get a clue about constructors
   return operator<<(Template(id));
}
inline UnicodeComposer & UnicodeComposer::operator<<(
   ErrorSeverity sevArg
) {
   m_msg.sev = sevArg;
   bMessagePending = true;
   return *this;
}
inline UnicodeComposer & UnicodeComposer::operator<<(
   NamedArg const & var
) {
   m_msg.arglNamed += var;
   bMessagePending = true;
   return *this;
}
inline UnicodeComposer & UnicodeComposer::operator<<(
   UnicodeString const & ucs
) {
   m_msg.arglPos += ucs;
   bMessagePending = true;
   return *this;
}

#ifdef OS__IRIX   // Compiler bug - SGI5 is confused with op<<(enums) !!!!
inline UnicodeComposer & UnicodeComposer::operator<<(int i) {
   return operator<<(UnicodeString(i));
}
inline UnicodeComposer & UnicodeComposer::operator<<(unsigned int i) {
   return operator<<(UnicodeString(i));
}
inline UnicodeComposer & UnicodeComposer::operator<<(long i) {
   return operator<<(UnicodeString(i));
}
inline UnicodeComposer & UnicodeComposer::operator<<(unsigned long i) {
   return operator<<(UnicodeString(i));
}
#endif

inline UnicodeComposer & UnicodeComposer::operator<<(
   char const * psz
) {
   if (m_msg.tpl.inqStatus() == Template::NotSet) {
      return operator<<(Template(psz));
   }else {
      return operator<<(UnicodeString(psz));
   }
}
inline UnicodeComposer& UnicodeComposer::operator<<(
   UnicodeComposer & (*f)(UnicodeComposer &)
) {
   return f(*this);
}
inline void UnicodeComposer::clear() {
   bMessagePending = false;
   m_msg.clear();
}
inline void UnicodeComposer::endMessage() {
   if (bMessagePending) {
      finishMessage(m_msg);
      clear();
   }
}

inline void UnicodeComposer::pushWatcher(Watcher * pWatcher) {
   watchList += pWatcher;
}

inline void UnicodeComposer::popWatcher() {
   --watchList;
}

/* -- INLINES (UnicodeComposer::MessageList) -- */

inline UnicodeComposer::Message * UnicodeComposer::MessageList::operator[](
   int ix
) {
   Item * pItm = (Item *)findDataAt(ix);
   return pItm;
}
inline UnicodeComposer::Message const *
UnicodeComposer::MessageList::operator[](
   int ix
) const {
   Item const * pItm = (Item const *)findDataAt(ix);
   return pItm;
}
inline UnicodeComposer::Message * UnicodeComposer::MessageList::addMessage(
   Message const & msg
) {
   Item * pItm = new Item(msg);
   if (insertLast(pItm)) return pItm; else return 0;
}
inline bool UnicodeComposer::MessageList::removeMessage(int ix) {
   return removeAt(ix);
}
inline int UnicodeComposer::MessageList::countMessages() const {
   return count();
}
inline void UnicodeComposer::MessageList::addMessageList(
   MessageList const & list
) {
   insertLast(list);
}
TOOLS_API UnicodeComposer & endm(UnicodeComposer & cps);

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
