//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: particle system code
//
//===========================================================================//

#include <algorithm>
#include "tier0/platform.h"
#include "tier0/vprof.h"
#include "particles/particles.h"
#include "psheet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static ALIGN16 ParticleRenderData_t s_SortedIndexList[MAX_PARTICLES_IN_A_SYSTEM] ALIGN16_POST;


enum EParticleSortKeyType
{
	SORT_KEY_NONE,
	SORT_KEY_DISTANCE,
	SORT_KEY_CREATION_TIME,
};


template<EParticleSortKeyType eSortKeyMode, bool bCull> void s_GenerateData( void *pOutData, Vector CameraPos, Vector *pCameraFwd, 
																			 CParticleVisibilityData *pVisibilityData, CParticleCollection *pParticles )
{
	fltx4 *pOutUnSorted = reinterpret_cast<fltx4 *>( pOutData );

	C4VAttributeIterator pXYZ( PARTICLE_ATTRIBUTE_XYZ, pParticles );
	CM128AttributeIterator pCreationTimeStamp( PARTICLE_ATTRIBUTE_CREATION_TIME, pParticles );
	CM128AttributeIterator pAlpha( PARTICLE_ATTRIBUTE_ALPHA, pParticles );
	CM128AttributeIterator pAlpha2( PARTICLE_ATTRIBUTE_ALPHA2, pParticles );
 	CM128AttributeIterator pRadius( PARTICLE_ATTRIBUTE_RADIUS, pParticles );

	int nParticles = pParticles->m_nActiveParticles;

	FourVectors EyePos;
	EyePos.DuplicateVector( CameraPos );
	FourVectors v4Fwd;
	if ( bCull )
		v4Fwd.DuplicateVector( *pCameraFwd );


	fltx4 fl4AlphaVis = ReplicateX4( pVisibilityData->m_flAlphaVisibility );
	fltx4 fl4RadVis = ReplicateX4( pVisibilityData->m_flRadiusVisibility );

	// indexing. We will generate the index as float and use magicf2i to convert to integer
	fltx4 fl4OutIdx = g_SIMD_0123; // 0 1 2 3

	fl4OutIdx = AddSIMD( fl4OutIdx, Four_2ToThe23s);							// fix as int

	bool bUseVis = pVisibilityData->m_bUseVisibility;

	fltx4 fl4AlphaScale = ReplicateX4( 255.0 );
	fltx4 fl4SortKey = Four_Zeros;

	do
	{
		fltx4 fl4FinalAlpha = MulSIMD( *pAlpha, *pAlpha2 );
		fltx4 fl4FinalRadius = *pRadius;

		if ( bUseVis )
		{
			fl4FinalAlpha = MaxSIMD ( Four_Zeros, MinSIMD( Four_Ones, MulSIMD( fl4FinalAlpha, fl4AlphaVis) ) );
			fl4FinalRadius = MulSIMD( fl4FinalRadius, fl4RadVis );
		}
		// convert float 0..1 to int 0..255
		fl4FinalAlpha = AddSIMD( MulSIMD( fl4FinalAlpha, fl4AlphaScale ), Four_2ToThe23s );

		if ( eSortKeyMode == SORT_KEY_CREATION_TIME )
		{
			fl4SortKey = *pCreationTimeStamp;
		}
		if ( bCull || ( eSortKeyMode == SORT_KEY_DISTANCE ) )
		{
			fltx4 fl4X = pXYZ->x;
			fltx4 fl4Y = pXYZ->y;
			fltx4 fl4Z = pXYZ->z;
			fltx4 Xdiff = SubSIMD( fl4X, EyePos.x );
			fltx4 Ydiff = SubSIMD( fl4Y, EyePos.y );
			fltx4 Zdiff = SubSIMD( fl4Z, EyePos.z );
			if ( bCull )
			{
				fltx4 dot = AddSIMD( MulSIMD( Xdiff, v4Fwd.x ),
									 AddSIMD(
										 MulSIMD( Ydiff, v4Fwd.y ),
										 MulSIMD( Zdiff, v4Fwd.z ) ) );
				fl4FinalAlpha = AndSIMD( fl4FinalAlpha, CmpGeSIMD( dot, Four_Zeros ) );
			}
			if ( eSortKeyMode == SORT_KEY_DISTANCE )
			{
				fl4SortKey = AddSIMD( MulSIMD( Xdiff, Xdiff ),
									  AddSIMD( MulSIMD( Ydiff, Ydiff ),
											   MulSIMD( Zdiff, Zdiff ) ) );
			}
		}
		// now, we will use simd transpose to write the output
		fltx4 i4Indices = AndSIMD( fl4OutIdx, 	LoadAlignedSIMD( (float *) g_SIMD_Low16BitsMask ) );
		TransposeSIMD( fl4SortKey, i4Indices, fl4FinalRadius, fl4FinalAlpha );
		pOutUnSorted[0] = fl4SortKey;
		pOutUnSorted[1] = i4Indices;
		pOutUnSorted[2] = fl4FinalRadius;
		pOutUnSorted[3] = fl4FinalAlpha;
		
		pOutUnSorted += 4;

		fl4OutIdx = AddSIMD( fl4OutIdx, Four_Fours );

		nParticles -= 4;

		++pXYZ;
		++pAlpha;
		++pAlpha2;
		++pRadius;
	} while( nParticles > 0 );								// we're not called with 0
}



#define TREATASINT(x) ( *(  ( (int32 const *)( &(x) ) ) ) )

static bool SortLessFunc( const ParticleRenderData_t &left, const ParticleRenderData_t &right )
{
	return TREATASINT( left.m_flSortKey ) < TREATASINT( right.m_flSortKey );
	
}


int CParticleCollection::GenerateSortedIndexList( ParticleRenderData_t *pOut, Vector vecCamera, CParticleVisibilityData *pVisibilityData, bool bSorted )
{
	VPROF_BUDGET( "CParticleCollection::GenerateSortedIndexList", VPROF_BUDGETGROUP_PARTICLE_RENDERING );

	int nParticles = m_nActiveParticles;
	if ( bSorted )
	{
		s_GenerateData<SORT_KEY_DISTANCE, false>( pOut, vecCamera, NULL, pVisibilityData, this );
	}
	else
		s_GenerateData<SORT_KEY_NONE, false>( pOut, vecCamera, NULL, pVisibilityData, this );

#ifndef SWDS
	if ( bSorted )
	{
		// sort the output in place
		std::make_heap( pOut, pOut + nParticles, SortLessFunc );
		std::sort_heap( pOut, pOut + nParticles, SortLessFunc );
	}
#endif
	return nParticles;
}

int CParticleCollection::GenerateCulledSortedIndexList( 
	ParticleRenderData_t *pOut, Vector vecCamera, Vector vecFwd, CParticleVisibilityData *pVisibilityData, bool bSorted )
{
	VPROF_BUDGET( "CParticleCollection::GenerateSortedIndexList", VPROF_BUDGETGROUP_PARTICLE_RENDERING );

	int nParticles = m_nActiveParticles;
	if ( bSorted )
	{
		s_GenerateData<SORT_KEY_DISTANCE, true>( pOut, vecCamera, &vecFwd, pVisibilityData, this );
	}
	else
		s_GenerateData<SORT_KEY_NONE, true>( pOut, vecCamera, &vecFwd, pVisibilityData, this );

#ifndef SWDS
	if ( bSorted )
	{
		// sort the output in place
		std::make_heap( pOut, pOut + nParticles, SortLessFunc );
		std::sort_heap( pOut, pOut + nParticles, SortLessFunc );
	}
#endif
	return nParticles;
}


const ParticleRenderData_t *CParticleCollection::GetRenderList( IMatRenderContext *pRenderContext, bool bSorted, int *pNparticles, CParticleVisibilityData *pVisibilityData)
{
	if ( bSorted )
		bSorted = m_pDef->m_bShouldSort;

	Vector vecCamera;
	pRenderContext->GetWorldSpaceCameraPosition( &vecCamera );
	ParticleRenderData_t *pOut = s_SortedIndexList;
	// check if the camera is inside the bounding box to see whether culling is worth it
	int nParticles;

	if ( vecCamera.WithinAABox( m_MinBounds, m_MaxBounds ) )
	{
		Vector vecFwd, vecRight, vecUp;
		pRenderContext->GetWorldSpaceCameraVectors( &vecFwd, &vecRight, &vecUp );
		
		nParticles = GenerateCulledSortedIndexList( pOut, vecCamera, vecFwd, pVisibilityData, bSorted );
	}
	else
	{
		// outside the bounds. don't bother agressive culling
		nParticles = GenerateSortedIndexList( pOut, vecCamera, pVisibilityData, bSorted );
	}
	*pNparticles = nParticles;
	return pOut + nParticles;
}




