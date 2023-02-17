//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: schemainitutils: Helpful macros when initing stuff that you'll
//			want to record multiple errors from
//
//=============================================================================

#ifndef SCHEMAINITUTILS_H
#define SCHEMAINITUTILS_H
#ifdef _WIN32
#pragma once
#endif

// used for initialization functions. Adds an error message if we're recording
// them or returns false if we're not
#if defined(DEBUG) || defined(VALVE_PURE)
#define SCHEMA_INIT_CHECK( expr, ... )							\
	if ( false == ( expr ) )											\
	{																	\
		CUtlString msg;													\
		msg.Format( __VA_ARGS__ );										\
		if ( NULL == ( pVecErrors ) )									\
		{																\
			AssertMsg( expr, "%s", msg.String() );						\
		}																\
		else															\
		{																\
#ifndef VALVE_PURE														\
			Warning( "%s\n", msg.String() );							\
			/*
				todo(maximsmol):
				we do not support upstream item schema
			*/															\
			return false;												\
#endif																	\
			pVecErrors->AddToTail( msg );								\
		}																\
		return false;													\
	}																	
#else
#define SCHEMA_INIT_CHECK( expr, ... )							\
	if ( false == ( expr ) )											\
	{																	\
		CUtlString msg;													\
		msg.Format( __VA_ARGS__ );										\
		Warning( "%s\n", msg.String() );						    \
		return false;													\
	}
#endif

#define SCHEMA_INIT_SUCCESS( )											\
	( NULL == pVecErrors ) || ( 0 == pVecErrors->Count() )

#define SCHEMA_INIT_SUBSTEP( expr )										\
	if ( !( expr ) )	\
		return false;

#endif // SCHEMAINITUTILS_H