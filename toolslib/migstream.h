/*
* $Id: migstream.h,v 1.4 2002-08-29 07:16:28 pgr Exp $
*
* Following macro permits to use new C++ ISO_14882,  or "regular" C++
* for streams.  ISO_14882 must be defined for the former.
*/

#ifndef COM_JAXO_TOOLS_MIGSTREAM_H_INCLUDED
#define COM_JAXO_TOOLS_MIGSTREAM_H_INCLUDED

#if defined __MWERKS__
#define ios__openmode ios::openmode

#elif defined ISO_14882
using std::streambuf;
using std::filebuf;
using std::iostream;
using std::istream;
using std::ostream;
using std::streamoff;
using std::streampos;
using std::streamsize;
using std::ios;
using std::basic_istream;
using std::basic_ostream;
#define ios__openmode ios::openmode

#else
#if defined _WIN32
#define streamsize int
#endif
#define pubseekoff seekoff
#define pubseekpos seekpos
#define pubsetbuf setbuf
#define pubsync sync
#define ios__openmode int
#define seekdir seek_dir
#endif

#endif
/*===========================================================================*/
