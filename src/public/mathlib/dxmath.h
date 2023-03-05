//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#pragma once

#define USE_DXMATH 1

#if USE_DXMATH
#if defined(_WIN32)
#include "../thirdparty/DirectXMath-dec2022/Inc/DirectXMath.h"
#elif defined(POSIX)
#include "../thirdparty/dotnetrt/sal.h"
#include "../thirdparty/DirectXMath-dec2022/Inc/DirectXMath.h"
#else
#undef USE_DXMATH
#endif
#endif
