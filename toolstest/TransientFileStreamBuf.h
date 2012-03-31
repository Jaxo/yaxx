/*
* $Id: TransientFileStreamBuf.h,v 1.8 2002-06-05 13:12:24 pgr Exp $
*
* Simulation of Transient Streams
*
* TransientFileStreamBuf simulates the behaviour of a transient streams
* by returning a random number of bytes available at a given time.
* This number can be as well 0, indicating the stream is temporarly
* interrupted.
*/

#if defined _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "../toolslib/FileStreamBuf.h"
#include "../toolslib/Reader.h"
#include "../toolslib/Writer.h"

#if defined _WIN32
   #define OPEN_READ_FLAGS  O_RDONLY|O_BINARY
#else
   #define OPEN_READ_FLAGS  O_RDONLY
#endif

/*-----------------------------------------------------TransientFileStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TransientFileStreamBuf : public FileStreamBuf {
public:
   TransientFileStreamBuf() {
      xfd = -1;
      ix = 0;
   }
   FileStreamBuf * open(char const * pszPath, unsigned long opflgs) {
      xfd = ::open((char *)pszPath, opflgs);
      return (xfd >= 0)? this : 0;
   }
   FileStreamBuf * close() {
     if (xfd >= 0) {
        sync();
        ::close(xfd);
        xfd = -1;
        return this;
     }
     return 0;
   }
private:
   int xfd;
   int ix;

   int read(char *pchBuf, int iLen) {
      // each suceeding read reads at maximum the values below (circular)
      static int values[] = { 10, 7, 35, 0 };
      int iAvailable = values[ix];
      ix = (ix+1) % (sizeof values / sizeof values[0]);
      if (!iAvailable) return TIMEOUT;
      if (iLen > iAvailable) iLen = iAvailable;
      return ((xfd >= 0) && iLen)? ::read(xfd, pchBuf, iLen) : 0;
   }
   int write(char const * pchBuf, int iLen) {
      return -1;  // TransientStreamBuf is only used for input
   }

   // undefined:
   TransientFileStreamBuf & operator=(TransientFileStreamBuf const & source);
   TransientFileStreamBuf(TransientFileStreamBuf const & source);
};

/*===========================================================================*/
