extern int testFileStream();
extern int testMemStream();
extern int testTransient();
extern int testTransition();
extern int testList();
extern int testLinkedList();
extern int testReaderWriter();
extern int testURI(char const * path);

/*-----------------------------------------------------------------------main-+
|                                                                             |
+----------------------------------------------------------------------------*/
int main(int argc, char ** argv)
{
   if (argc > 1) {
      switch (*argv[1]) {
      case 'F': case 'f': testFileStream(); break;
      case 'K': case 'k': testLinkedList(); break;
      case 'L': case 'l': testList(); break;
      case 'M': case 'm': testMemStream(); break;
      case 'N': case 'n': testTransition(); break;
      case 'T': case 't': testTransient(); break;
      case 'U': case 'u': testURI(0); break;
      case 'W': case 'w': testReaderWriter(); break;
      }
   }else {
      testFileStream();
      testMemStream();
      testTransient();
      testTransition();
      testList();
      testLinkedList();
      testReaderWriter();
      testURI("../tests/data/rfc2396.in");
   }
   return 0;
}

/*===========================================================================*/
