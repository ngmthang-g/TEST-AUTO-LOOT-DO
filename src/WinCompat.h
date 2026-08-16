#pragma once

// Force-include Windows headers once, then remove legacy memory-model tokens
// such as `near` that still exist as empty macros in Win32 headers.
#include <windows.h>

#ifdef near
#undef near
#endif
