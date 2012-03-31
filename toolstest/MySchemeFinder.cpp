/* $Id: MySchemeFinder.cpp,v 1.1 2002-06-21 17:40:56 pgr Exp $*/


// compile using: cl -GX MySchemeFinder.cpp ..\toolslib\Debug\toolslib.lib
// run from (YAXX_HOME)/debug

#include "../toolslib/URI.h"


class MySchemeFinder : public URI::SchemeHandler, public URI {
public:
   MySchemeFinder(char const * pszUri);

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
      bool matchesScheme(char const * scheme, int len) const;
   };
};

inline MySchemeFinder::MySchemeFinder(char const * pszUri) :
   SchemeHandler(new Rep),
   URI(pszUri, URI(), *this) {
}
inline char const * MySchemeFinder::Rep::getID() const {
   return "";
}
inline bool MySchemeFinder::Rep::matchesScheme(char const * scheme, int len) const {
   return true;
}
inline iostream * MySchemeFinder::Rep::makeStream(URI const &, ios__openmode) {
   return 0;
}

int main(int argc, char ** argv) {
   if (argc == 1)  {
      cout << "Pass an URI, please" << endl;
   }else {
      try {
         cout << MySchemeFinder(argv[1]).getScheme();
      }catch (URI::MalformedException) {
         cout << "Sorry!  Bad URI" << endl;
      }
   }
   return 0;
}

/*===========================================================================*/
