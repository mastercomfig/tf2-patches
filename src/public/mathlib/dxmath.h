//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#pragma once

#define USE_DXMATH 1

#if USE_DXMATH
#if defined(_WIN32) || defined(POSIX)
#include "../thirdparty/DirectXMath-dec2022/Inc/DirectXMath.h"
#else
#undef USE_DXMATH
#endif
#endif
