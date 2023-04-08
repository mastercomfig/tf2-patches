//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef _MATH_PFNS_H_
#define _MATH_PFNS_H_

#if defined( _X360 )
#include <xboxmath.h>
#else
#include "dxmath.h"
#if !USE_DXMATH
#include <xmmintrin.h>
#endif
#define USE_SSE2
#include "../thirdparty/sse_mathfun/sse_mathfun.h"
#endif

#if !defined( _X360 )

FORCEINLINE float RSqrt(float x)
{
	// The compiler will generate ideal instructions for a Newton-Raphson
	// Specifying it directly results in worse assembly.
	return 1.0f / sqrtf(x);
}

FORCEINLINE float RSqrtFast(float x)
{
	// This results in the compiler simplifying down to a plain rsqrtss
	const __m128 vec = _mm_set_ss( x );
	const __m128 r = _mm_rsqrt_ps( vec );
	float temp;
	_mm_store_ss(&temp, r);
	return temp;
}

FORCEINLINE float CosFast(float x)
{
	// Compiler doesn't optimize ::cosf call, use a vectorized cos. This is better than DirectX::XMScalarCos
	const __m128 vec = _mm_set_ss( x );
	const __m128 r = cos_ps(vec);
	float temp;
	_mm_store_ss(&temp, r);
	return temp;
}

// The following are not declared as macros because they are often used in limiting situations,
// and sometimes the compiler simply refuses to inline them for some reason
#define FastSqrt(x)			::sqrtf(x) // sqrt is optimized to an efficient SSE call with modern compilers
#define	FastRSqrt(x)		RSqrt(x)
#define FastRSqrtFast(x)    RSqrtFast(x)
#define FastCos(x)			CosFast(x)

#endif // !_X360

#if defined( _X360 )

FORCEINLINE float _VMX_Sqrt( float x )
{
	return __fsqrts( x );
}

FORCEINLINE float _VMX_RSqrt( float x )
{
	float rroot = __frsqrte( x );

	// Single iteration NewtonRaphson on reciprocal square root estimate
	return (0.5f * rroot) * (3.0f - (x * rroot) * rroot);
}

FORCEINLINE float _VMX_RSqrtFast( float x )
{
	return __frsqrte( x );
}

FORCEINLINE void _VMX_SinCos( float a, float *pS, float *pC )
{
	XMScalarSinCos( pS, pC, a );
}

FORCEINLINE float _VMX_Cos( float a )
{
	return XMScalarCos( a );
}

// the 360 has fixed hw and calls directly
#define FastSqrt(x)			_VMX_Sqrt(x)
#define	FastRSqrt(x)		_VMX_RSqrt(x)
#define FastRSqrtFast(x)	_VMX_RSqrtFast(x)
#define FastSinCos(x,s,c)	_VMX_SinCos(x,s,c)
#define FastCos(x)			_VMX_Cos(x)

#endif // _X360

#endif // _MATH_PFNS_H_
