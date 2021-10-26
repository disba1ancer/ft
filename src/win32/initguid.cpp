#include <initguid.h>
#include "win32.h"

#ifndef __MINGW32__
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const CLSID IID_IAudioClient = __uuidof(IAudioClient);
#endif
