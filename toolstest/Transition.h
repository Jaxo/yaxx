/*
* $Id: Transition.h,v 1.1 2002-04-05 07:33:30 pgr Exp $
*
* Example of simple RefdItems.
*
* A Transition, and its derived class NamedTransition,
*  - IS-A RefdItem controlling the construction, assignement and destruction
*  - HAS-A target Transition, e.g it is a single-linked list
*/

#include <string.h>
#include <iostream>
#define ISO_14882   // please, don't ask me why (string?)
#include "../toolslib/migstream.h"
#include "../toolslib/RefdItem.h"

/*--------------------------------------------------------------- Transition -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TransitionRep;

class Transition : public RefdItem {
   friend class TransitionList;
public:
   Transition();
   /*
   | carefull: do not write next line such as:
   | Transition(Transition const & target);
   | This is *not* a copy constructor!
   */
   Transition(Transition const * target);
   TransitionRep * operator->();
protected:
   Transition(RefdItemRep * pRep);
};

class TransitionRep : public RefdItemRep {
public:
   TransitionRep() {
   }
   TransitionRep(Transition const & target) {
      m_target = target;
   }
   virtual void sayHi() {
      cout << "Hello - I'm a native Transition" << endl;
   }
   Transition const & getConnected() const {
      return (Transition const &)m_target;
   }
private:
   RefdItem m_target;
};

inline Transition::Transition() :
   RefdItem(new TransitionRep)
{}

inline Transition::Transition(Transition const * target) :
   RefdItem(new TransitionRep(*target))
{}

inline Transition::Transition(RefdItemRep * pRep) :
   RefdItem(pRep)
{}

inline TransitionRep * Transition::operator->() {
   return (TransitionRep *)inqData();
};

/*---------------------------------------------------------- NamedTransition -+
|                                                                             |
+----------------------------------------------------------------------------*/
class NamedTransitionRep;

class NamedTransition : public Transition {
public:
   NamedTransition(Transition const * target, char const * name);
   NamedTransitionRep * operator->();
};

class NamedTransitionRep : public TransitionRep {
public:
   NamedTransitionRep(Transition const & target, char const * name) :
      TransitionRep(target)
   {
      strcpy(m_name=new char[1+strlen(name)], name);
   }
   ~NamedTransitionRep() {
      delete[] m_name;
   }
   virtual void sayHi() {
      cout << "Hello - I'm a NamedTransition: " << m_name << endl;
   }
private:
   char * m_name;
};

inline NamedTransition::NamedTransition(Transition const * target, char const * name) :
   Transition(new NamedTransitionRep(*target, name))
{}

inline NamedTransitionRep * NamedTransition::operator->() {
   return (NamedTransitionRep *)inqData();
};

/*===========================================================================*/
