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
#endif

#if !defined( _X360 )

FORCEINLINE float RSqrt(float x)
{
#if USE_DXMATH
	const __m128 vec = _mm_set_ss(x);
	const __m128 r = DirectX::XMVectorReciprocalSqrt(vec);
#else
	const __m128 one = _mm_set_ss(1.0f);
	const __m128 y = _mm_set_ss(x);
	const __m128 x0 = _mm_sqrt_ss(y);
	const __m128 r = _mm_div_ss(one, x0);
#endif
	float temp;
	_mm_store_ss(&temp, r);
	return temp;
}

FORCEINLINE float RSqrtFast(float x)
{
#if USE_DXMATH
	const __m128 vec = _mm_set_ss(x);
	const __m128 r = DirectX::XMVectorReciprocalSqrtEst(vec);
#else
	const __m128 vec = _mm_set_ss(x);
	const __m128 r = _mm_rsqrt_ps(vec);
#endif
	float temp;
	_mm_store_ss(&temp, r);
	return temp;
}

// The following are not declared as macros because they are often used in limiting situations,
// and sometimes the compiler simply refuses to inline them for some reason
// On x86, the inline FPU or SSE sqrt instruction is faster than
// the overhead of setting up a function call and saving/restoring
// the FPU or SSE register state and can be scheduled better, too.
#define FastSqrt(x)			::sqrtf(x)
#define	FastRSqrt(x)		RSqrt(x)
#define FastRSqrtFast(x)    RSqrtFast(x)
#define FastCos(x)			::cosf(x)

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
