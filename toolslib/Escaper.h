/*
* $Id: Escaper.h,v 1.4 2002-06-24 03:12:29 jlatone Exp $
*
* Parse escape sequences
*
* Called for each string forming the output stream, Escaper::process() will
* result in calling the virtual Escaper::output () for the textual part
* and/or the appropriate Escaper::Decoder for each particular escape sequence.
*
* Typical example:
*
* class MyConsole : public Escaper {
* public:
*    MyConsole();
*    void display(char const * pchBuf, int len) {
*       process(pchBuf, len);
*    }
*    void clear() { printf("=> [CLEAR]"); }
* private:
*    void output(char const * pchBuf, int len) {
*       printf("=> \"%.*s\"\n", len, pchBuf);
*    };
*    class ClearDecoder : public Escaper::ClearDecoder {
*    public:
*       ClearDecoder(MyConsole & console) : m_console(console) {}
*       void action() { m_console.clear(); }
*       MyConsole & m_console;
*    };
* };
*
* MyConsole::MyConsole() {
*    registerDecoder(new ClearDecoder(*this));
* }
*
* WARNING: after registration, the destruction of the Decoder is
*          under the responsability of this class: never do delete of
*          a registered Decoder!
*/

#ifndef COM_JAXO_TOOLS_ESCAPER_H_INCLUDED
#define COM_JAXO_TOOLS_ESCAPER_H_INCLUDED

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"
#include "LinkedList.h"

/*------------------------------------------------------------ class Escaper -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API Escaper {
public:
   class Decoder;

   Escaper();
   void registerDecoder(Decoder *);
   void process(char const * pchBuf, int len);
   virtual void output(char const * pchBuf, int len) = 0;

   enum { ESCAPE_CHAR = 0x1B };
   class Decoder : public LinkedList::Item {
      friend class Escaper;
   public:
      virtual char const * getCode() = 0;
      virtual void action() = 0;
   protected:
      enum Answer {
         ME_DUNNO,     // definite answer - call me again with more food
         ME_MATCH,     // definite answer - i take action for it
         NOT_ME        // i'm not concerned - iterate next decoder
      };
      Decoder();
   private:
      Answer match(char const * p, int max);
      Decoder(Decoder const & source);
      Decoder & operator=(Decoder const & source);
   };

   class ClearDecoder : public Decoder {
   public:
      char const * getCode() { return "[2J"; }
      virtual void action() = 0;
   };

   class BELDecoder : public Decoder {
   public:
      char const * getCode() { return "!@\x07"; }
      virtual void action() = 0;
   };

private:
   class TOOLS_API DecodersList : public LinkedList {};
   struct TOOLS_API WaitBuffer {
      WaitBuffer();
      int size;
      char const esc;
      char buf[4];
   }m_wait;

   Decoder * m_decoder;
   char const * m_buf;
   DecodersList m_decoders;     // list of registered decoders

   Decoder::Answer findDecoder(char const * p, int max);
};

/* -- INLINES -- */
inline Escaper::WaitBuffer::WaitBuffer() : esc(ESCAPE_CHAR), size(-1) {}
inline Escaper::Decoder::Decoder() {}
inline Escaper::Escaper() {}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
