#pragma once

// Force-include Windows headers once, then remove legacy Win32 macros that
// collide with modern C++ identifiers/std::min/std::max.
#include <windows.h>

#ifdef near
#undef near
#endif
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
