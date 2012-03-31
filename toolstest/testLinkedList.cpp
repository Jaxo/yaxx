/* $Id: testLinkedList.cpp,v 1.2 2002-05-25 10:51:06 pgr Exp $ */

// this sample shows a simple linked list
#include <iostream.h>
#include <string.h>
#include "../toolslib/LinkedList.h"

/*-------------------------------------------------------------- class Chose -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Chose : public LinkedList::Item {
   friend ostream & operator<<(ostream & out, Chose const & laChose);
public:
   Chose(char const * n) {
      name = strcpy(new char[1+strlen(n)], n);
   }
   ~Chose() {
      cout << "Removing " << name << endl;
      delete [] name;
   }
private:
   char * name;
};

ostream & operator<<(ostream & out, Chose const & laChose) {
   return out << laChose.name;
}

/*---------------------------------------------------------------------showMe-+
|                                                                             |
+----------------------------------------------------------------------------*/
void showMe(LinkedList & lst)
{
   bool goFifo = true;
   for (;;) {
      LinkedList::Iterator it(lst);
      Chose const * pWhat;
      bool isFirst = true;
      cout << (goFifo? "FIFO: " : "LIFO: ");
      while (pWhat = (Chose *)(goFifo? it.next() : it.previous()), pWhat) {
         if (!isFirst) cout << ", "; else isFirst = false;
         cout << *pWhat;
      }
      if (isFirst) cout << "**EMPTY**";
      cout << endl;
      if (!goFifo) break;
      goFifo = false;
   }
}

/*-------------------------------------------------------------testLinkedList-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testLinkedList()
{
   LinkedList lst;
   cout << "--------------- testLinkedList" << endl;
   showMe(lst);

   cout << "-------------------------" << endl;
   lst.add(new Chose("Pierre"));
   showMe(lst);

   cout << "-------------------------" << endl;
   lst.~LinkedList();
   showMe(lst);

   cout << "-------------------------" << endl;
   lst.add(new Chose("Pierre"));
   lst.add(new Chose("Jacques"));
   lst.add(new Chose("Paul"));
   lst.add(new Chose("Andre"));
   lst.add(new Chose("Nicolas"));
   lst.add(new Chose("Gregory"));
   lst.add(new Chose("Benoit"));
   showMe(lst);

   cout << "-------------------------" << endl;
   lst.addFirst(new Chose("Jean-Paul"));
   lst.addLast(new Chose("Francois"));
   lst.set(1, new Chose("Jean-Louis"));
   cout << "Number 2 is: " << *(Chose *)lst.get(1) << endl;
   lst.add(1, new Chose("Pierre"));
   lst.remove(3);
   showMe(lst);

   cout << "-------------------------" << endl;
   lst.clear();
   showMe(lst);

   cout << "-------------------------" << endl;
   lst.clear();
   showMe(lst);
   return 0;
}

/*===========================================================================*/
