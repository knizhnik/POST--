//-< COMPTIME.CXX >--------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:      2-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update:  2-Feb-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Storing timestamp of program building. Always include this file (sources!)
// in the list of files in makefile rule for producing executables. 
//-------------------------------------------------------------------*--------*

#include "stdtp.h"

BEGIN_POST_NAMESPACE

extern char* program_compilation_time;

class set_compilation_time { 
  public:
    set_compilation_time() { 
	program_compilation_time = __DATE__ " " __TIME__;
    }
};

static set_compilation_time timestamp;

END_POST_NAMESPACE
