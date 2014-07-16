#ifndef __Common_H__
#define __Common_H__

#include <POL.h>
using namespace POL;

#include "irexx2Rsc.h"

#define CREATORID 'Jaxo'  // NOTE:  This must match the creator id
						// that's specified in the build options'
						// PalmRex Post Linker!!!  I don't think
						// we can get/set it programmatically
						// so that it appears in only one place.
// YES WE CAN!
// use SysCurAppDatabase to get our own database's (PRC's) dbid, 
// and then use DmDatabaseInfo to get the crid, e.g., 
//UInt16 cardNo;
//LocalID dbId ;
//SysCurAppDatabase(&cardNo, &dbId) ;
//DmDatabaseInfo(cardNo, dbId, 0, 0, 0, 0, 0, 0, 0, 0, 0, o, &crid);
					
#endif
