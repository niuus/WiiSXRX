/************************************************************************

Copyright mooby 2002

CDRMooby2 Globals.cpp
http://mooby.psxfanatics.com

  This file is protected by the GNU GPL which should be included with
  the source code distribution.

************************************************************************/

#include "defines.h"
#include "Utils.hpp"
#include "CDInterface.hpp"

/** global data for the plugin **/

// the cd data
CDInterface* theCD = NULL;

// a return code
int rc = 0;

// the format of the TD and TN calls
TDTNFormat tdtnformat = msfint;

// the name of the emulator that's using the plugin
std::string programName;

// whether it's using the psemu or fpse interface
EMUMode mode = psemu;
