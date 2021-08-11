//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Random number generator
//
// $Workfile: $
// $NoKeywords: $
//===========================================================================//

#ifndef VSTDLIB_RANDOM_H
#define VSTDLIB_RANDOM_H

#include "vstdlib/vstdlib.h"
#include "tier0/basetypes.h"
#include "tier0/threadtools.h"
#include "tier1/interface.h"

#define NTAB 32

#pragma warning(push)
#pragma warning( disable:4251 )

//-----------------------------------------------------------------------------
// A generator of uniformly distributed random numbers
//-----------------------------------------------------------------------------
class VSTDLIB_CLASS IUniformRandomStream
{
public:
	//virtual ~IUniformRandomStream() { }

	// Sets the seed of the random number generator
	virtual void	SetSeed( int iSeed ) = 0;
	virtual void	SetSeedScoped(int iSeed) = 0;

	// Control scope so that a seeding user can generate multiple consistently
	virtual void RandomStartScope() = 0;
	virtual void RandomEndScope() = 0;

	// Generates random numbers
	virtual float	RandomFloat( float flMinVal = 0.0f, float flMaxVal = 1.0f ) = 0;
	virtual int		RandomInt( int iMinVal, int iMaxVal ) = 0;
	virtual float	RandomFloatExp( float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f ) = 0;

	virtual float	RandomFloatScoped(float flMinVal = 0.0f, float flMaxVal = 1.0f) = 0;
	virtual int		RandomIntScoped(int iMinVal, int iMaxVal) = 0;
};


//-----------------------------------------------------------------------------
// The standard generator of uniformly distributed random numbers
//-----------------------------------------------------------------------------
class VSTDLIB_CLASS CUniformRandomStream : public IUniformRandomStream
{
public:
	CUniformRandomStream();

	// Sets the seed of the random number generator
	virtual void	SetSeed( int iSeed );
	virtual void	SetSeedScoped(int iSeed);

    int GenerateRandomNumberScoped();

	virtual void RandomStartScope();
	virtual void RandomEndScope();

    // Generates random numbers
	virtual float	RandomFloat( float flMinVal = 0.0f, float flMaxVal = 1.0f );
	virtual int		RandomInt( int iMinVal, int iMaxVal );
	virtual float	RandomFloatExp( float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f );

	virtual float	RandomFloatScoped(float flMinVal = 0.0f, float flMaxVal = 1.0f);
	virtual int		RandomIntScoped(int iMinVal, int iMaxVal);

	float GenerateFloat(int flRnd, float flMinVal, float flMaxVal);

private:
	int		GenerateRandomNumber();

	int m_idum;
	int m_iy;
	int m_iprevdum;
	int m_iprevy;
	int m_iv[NTAB];
	int m_iprevv[NTAB];

	CThreadFastMutex m_mutex;
};


//-----------------------------------------------------------------------------
// A generator of gaussian distributed random numbers
//-----------------------------------------------------------------------------
class VSTDLIB_CLASS CGaussianRandomStream
{
public:
	// Passing in NULL will cause the gaussian stream to use the
	// installed global random number generator
	CGaussianRandomStream( IUniformRandomStream *pUniformStream = NULL );

	// Attaches to a random uniform stream
	void	AttachToStream( IUniformRandomStream *pUniformStream = NULL );

	// Generates random numbers
	float	RandomFloat( float flMean = 0.0f, float flStdDev = 1.0f );

private:
	IUniformRandomStream *m_pUniformStream;
	bool	m_bHaveValue;
	float	m_flRandomValue;

	CThreadFastMutex m_mutex;
};

//-----------------------------------------------------------------------------
// A couple of convenience functions to access the library's global uniform stream
//-----------------------------------------------------------------------------
VSTDLIB_INTERFACE void	RandomSeed( int iSeed );
VSTDLIB_INTERFACE void	RandomSeedScoped(int iSeed);
VSTDLIB_INTERFACE void	RandomStartScope();
VSTDLIB_INTERFACE void	RandomEndScope();
VSTDLIB_INTERFACE float	RandomFloat( float flMinVal = 0.0f, float flMaxVal = 1.0f );
VSTDLIB_INTERFACE float	RandomFloatExp( float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f );
VSTDLIB_INTERFACE int	RandomInt( int iMinVal, int iMaxVal );
VSTDLIB_INTERFACE float	RandomFloatScoped(float flMinVal = 0.0f, float flMaxVal = 1.0f);
VSTDLIB_INTERFACE int	RandomIntScoped(int iMinVal, int iMaxVal);
VSTDLIB_INTERFACE float	RandomGaussianFloat( float flMean = 0.0f, float flStdDev = 1.0f );

//-----------------------------------------------------------------------------
// IUniformRandomStream interface for free functions
//-----------------------------------------------------------------------------
class VSTDLIB_CLASS CDefaultUniformRandomStream : public IUniformRandomStream
{
public:
	virtual void	SetSeed( int iSeed ) OVERRIDE												{ RandomSeed( iSeed ); }
	virtual void	SetSeedScoped(int iSeed) OVERRIDE { RandomSeedScoped(iSeed); }
	virtual void	RandomStartScope() OVERRIDE { ::RandomStartScope(); }
	virtual void	RandomEndScope() OVERRIDE { ::RandomEndScope(); }
	virtual float	RandomFloat(float flMinVal, float flMaxVal) OVERRIDE { return ::RandomFloat(flMinVal, flMaxVal); }
	virtual int		RandomInt( int iMinVal, int iMaxVal ) OVERRIDE								{ return ::RandomInt( iMinVal, iMaxVal ); }
	virtual float	RandomFloatExp( float flMinVal, float flMaxVal, float flExponent ) OVERRIDE	{ return ::RandomFloatExp( flMinVal, flMaxVal, flExponent ); }
	virtual float	RandomFloatScoped(float flMinVal, float flMaxVal) OVERRIDE { return ::RandomFloatScoped(flMinVal, flMaxVal); }
	virtual int		RandomIntScoped(int iMinVal, int iMaxVal) OVERRIDE { return ::RandomIntScoped(iMinVal, iMaxVal); }
};

//-----------------------------------------------------------------------------
// Installs a global random number generator, which will affect the Random functions above
//-----------------------------------------------------------------------------
VSTDLIB_INTERFACE void	InstallUniformRandomStream( IUniformRandomStream *pStream );


#pragma warning(pop)

#endif // VSTDLIB_RANDOM_H



