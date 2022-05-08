//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
// @Note (toml 12-02-02): For now, all of the methods in the types defined here
// are inline to permit simple cross-DLL usage
//=============================================================================//

#ifndef SAVERESTORETYPES_H
#define SAVERESTORETYPES_H

#if defined( _WIN32 )
#pragma once
#endif

#include "tier1/utlhash.h"

#include <string_t.h> // NULL_STRING define
struct edict_t;


#ifdef EHANDLE_H // not available to engine
#define SR_ENTS_VISIBLE 1
#endif


//-----------------------------------------------------------------------------
//
// class CSaveRestoreSegment
//

class CSaveRestoreSegment
{
public:
	CSaveRestoreSegment();
	
	//---------------------------------
	// Buffer operations
	//
	void Init( void *pNewBase, int nBytes );
	void Rebase();
	void Rewind( int nBytes );
	char *GetBuffer();
	int BytesAvailable() const;
	int SizeBuffer() const;
	bool Write( const void *pData, int nBytes );
	bool Read( void *pOutput, int nBytes );
	int GetCurPos();
	char *AccessCurPos();
	bool Seek( int absPosition );
	void MoveCurPos( int nBytes );

	//---------------------------------
	// Symbol table operations
	//
	void InitSymbolTable( char **pNewTokens, int sizeTable);
	char **DetachSymbolTable();
	int SizeSymbolTable();
	bool DefineSymbol( const char *pszToken, int token );
	unsigned short FindCreateSymbol( const char *pszToken );
	const char *StringFromSymbol( int token );

private:
	unsigned int HashString( const char *pszToken );
	
	//---------------------------------
	// Buffer data
	//
	char		*pBaseData;		// Start of all entity save data
	char		*pCurrentData;	// Current buffer pointer for sequential access
	int			size;			// Current data size, aka, pCurrentData - pBaseData
	int			bufferSize;		// Total space for data
	
	//---------------------------------
	// Symbol table
	//
	int			tokenCount;		// Number of elements in the pTokens table
	char		**pTokens;		// Hash table of entity strings (sparse)
};


//-----------------------------------------------------------------------------
//
// class CGameSaveRestoreInfo
//

struct levellist_t
{
	DECLARE_SIMPLE_DATADESC();

	char	mapName[ MAX_MAP_NAME_SAVE ];
	char	landmarkName[ 32 ];
	edict_t	*pentLandmark;
	Vector	vecLandmarkOrigin;
};

#define MAX_LEVEL_CONNECTIONS	16		// These are encoded in the lower 16bits of entitytable_t->flags

//-------------------------------------

struct EHandlePlaceholder_t // Engine does some of the game writing (alas, probably shouldn't), but can't see ehandle.h
{
	unsigned long i;
};

//-------------------------------------

struct entitytable_t
{
	void Clear()
	{
		id = -1;
		edictindex = -1;
		saveentityindex = -1;
		restoreentityindex = -1;
		location = 0;
		size = 0;
		flags = 0;
		classname = NULL_STRING;
		globalname = NULL_STRING;
		landmarkModelSpace.Init();
		modelname = NULL_STRING;
	}

	int			id;				// Ordinal ID of this entity (used for entity <--> pointer conversions)
	int			edictindex;		// saved for if the entity requires a certain edict number when restored (players, world)

	int			saveentityindex; // the entity index the entity had at save time ( for fixing up client side entities )
	int			restoreentityindex; // the entity index given to this entity at restore time

#ifdef SR_ENTS_VISIBLE
	EHANDLE		hEnt;			// Pointer to the in-game entity
#else
	EHandlePlaceholder_t hEnt;
#endif

	int			location;		// Offset from the base data of this entity
	int			size;			// Byte size of this entity's data
	int			flags;			// This could be a short -- bit mask of transitions that this entity is in the PVS of
	string_t	classname;		// entity class name
	string_t	globalname;		// entity global name
	Vector		landmarkModelSpace;	// a fixed position in model space for comparison
									// NOTE: Brush models can be built in different coordiante systems
									//		in different levels, so this fixes up local quantities to match
									//		those differences.
	string_t	modelname;

	DECLARE_SIMPLE_DATADESC();
};

#define FENTTABLE_PLAYER		0x80000000
#define FENTTABLE_REMOVED		0x40000000
#define FENTTABLE_MOVEABLE		0x20000000
#define FENTTABLE_GLOBAL		0x10000000
#define FENTTABLE_PLAYERCHILD	0x08000000		// this entity is connected to a player, so it must be spawned with it
#define FENTTABLE_LEVELMASK		0x0000FFFF		// reserve bits for 16 level connections
//-------------------------------------

struct saverestorelevelinfo_t
{
	int			connectionCount;// Number of elements in the levelList[]
	levellist_t	levelList[ MAX_LEVEL_CONNECTIONS ];		// List of connections from this level

	// smooth transition
	int			fUseLandmark;
	char		szLandmarkName[20];	// landmark we'll spawn near in next level
	Vector		vecLandmarkOffset;	// for landmark transitions
	float		time;
	char		szCurrentMapName[MAX_MAP_NAME_SAVE];	// To check global entities
	int			mapVersion;
};

//-------------------------------------

class CGameSaveRestoreInfo
{
public:
	CGameSaveRestoreInfo()
		: tableCount( 0 ), pTable( 0 ), m_pCurrentEntity( 0 ), m_EntityToIndex( 1024 )
	{
		memset( &levelInfo, 0, sizeof( levelInfo ) );
		modelSpaceOffset.Init( 0, 0, 0 );
	}

	void InitEntityTable( entitytable_t *pNewTable = NULL, int size = 0 )
	{
		pTable = pNewTable;
		tableCount = size;

		for ( int i = 0; i < NumEntities(); i++ )
		{
			GetEntityInfo( i )->Clear();
		}
	}

	entitytable_t *DetachEntityTable()
	{
		entitytable_t *pReturn = pTable;
		pTable = NULL;
		tableCount = 0;
		return pReturn;
	}

	CBaseEntity *GetCurrentEntityContext()	{ return m_pCurrentEntity; }
	void		SetCurrentEntityContext(CBaseEntity *pEntity) { m_pCurrentEntity = pEntity; }

	int NumEntities()						{ return tableCount; }
	entitytable_t *GetEntityInfo( int i )	{ return (pTable + i); }
	float GetBaseTime() const				{ return levelInfo.time; }
	Vector GetLandmark() const				{ return ( levelInfo.fUseLandmark ) ? levelInfo.vecLandmarkOffset : vec3_origin; }

	void BuildEntityHash()
	{
#ifdef GAME_DLL
		int i;
		entitytable_t *pTable;
		int nEntities = NumEntities();

		for ( i = 0; i < nEntities; i++ )
		{
			pTable = GetEntityInfo( i );
			m_EntityToIndex.Insert(  CHashElement( pTable->hEnt.Get(), i ) );
		}
#endif
	}

	void PurgeEntityHash()
	{
		m_EntityToIndex.Purge();
	}

	int	GetEntityIndex( const CBaseEntity *pEntity )
	{
#ifdef SR_ENTS_VISIBLE
		if ( pEntity )
		{
			if ( m_EntityToIndex.Count() )
			{
				UtlHashHandle_t hElement = m_EntityToIndex.Find( CHashElement( pEntity ) );
				if ( hElement != m_EntityToIndex.InvalidHandle() )
				{
					return m_EntityToIndex.Element( hElement ).index;
				}
			}
			else
			{
				int i;
				entitytable_t *pEntTable;

				int nEntities = NumEntities();
				for ( i = 0; i < nEntities; i++ )
				{
					pEntTable = GetEntityInfo( i );
					if ( pEntTable->hEnt == pEntity )
						return pEntTable->id;
				}
			}
		}
#endif
		return -1;
	}

	saverestorelevelinfo_t levelInfo;
	Vector		modelSpaceOffset;			// used only for globaly entity brushes modelled in different coordinate systems.
	
private:
	int			tableCount;		// Number of elements in the entity table
	entitytable_t	*pTable;		// Array of entitytable_t elements (1 for each entity)
	CBaseEntity		*m_pCurrentEntity; // only valid during the save functions of this entity, NULL otherwise


	struct CHashElement
	{
		const CBaseEntity *pEntity; 
		int index;

		CHashElement( const CBaseEntity *pEntity, int index) : pEntity(pEntity), index(index) {}
		CHashElement( const CBaseEntity *pEntity ) : pEntity(pEntity) {}
		CHashElement() {}
	};

	class CHashFuncs
	{
	public:
		CHashFuncs( int ) {}

		// COMPARE
		bool operator()( const CHashElement &lhs, const CHashElement &rhs ) const
		{
			return lhs.pEntity == rhs.pEntity;
		}

		// HASH
		unsigned int operator()( const CHashElement &item ) const
		{
			return HashItem( item.pEntity );
		}
	};

	typedef CUtlHash<CHashElement, CHashFuncs, CHashFuncs> CEntityToIndexHash;

	CEntityToIndexHash m_EntityToIndex;
};

//-----------------------------------------------------------------------------


class CSaveRestoreData : public CSaveRestoreSegment,
						 public CGameSaveRestoreInfo
{
public:
	CSaveRestoreData() : bAsync( false ) {}


	bool bAsync;
};

inline CSaveRestoreData *MakeSaveRestoreData( void *pMemory )
{
	return new (pMemory) CSaveRestoreData;
}

//-----------------------------------------------------------------------------
//
// class CSaveRestoreSegment, inline functions
//

inline CSaveRestoreSegment::CSaveRestoreSegment()
{
	memset( this, 0, sizeof(*this) );
}

inline void CSaveRestoreSegment::Init( void *pNewBase, int nBytes )
{
	pCurrentData = pBaseData = (char *)pNewBase;
	size = 0;
	bufferSize = nBytes;
}

inline void CSaveRestoreSegment::MoveCurPos( int nBytes )
{
	pCurrentData += nBytes;
	size += nBytes;
}

inline void CSaveRestoreSegment::Rebase()
{
	pBaseData = pCurrentData;
	bufferSize -= size;
	size = 0;
}

inline void CSaveRestoreSegment::Rewind( int nBytes )
{
	if ( size < nBytes )
		nBytes = size;

	MoveCurPos( -nBytes );
}

inline char *CSaveRestoreSegment::GetBuffer()
{
	return pBaseData;
}

inline int CSaveRestoreSegment::BytesAvailable() const
{
	return (bufferSize - size);
}

inline int CSaveRestoreSegment::SizeBuffer() const
{
	return bufferSize;
}

inline bool CSaveRestoreSegment::Write( const void *pData, int nBytes )
{
	if ( nBytes > BytesAvailable() )
	{
		size = bufferSize;
		return false;
	}

	memcpy( pCurrentData, pData, nBytes );
	MoveCurPos( nBytes );

	return true;
}

inline bool CSaveRestoreSegment::Read( void *pOutput, int nBytes )
{
	if ( !BytesAvailable() )
		return false;

	if ( nBytes > BytesAvailable() )
	{
		size = bufferSize;
		return false;
	}

	if ( pOutput )
		memcpy( pOutput, pCurrentData, nBytes );
	MoveCurPos( nBytes );
	return true;
}

inline int CSaveRestoreSegment::GetCurPos()
{
	return size;
}

inline char *CSaveRestoreSegment::AccessCurPos()
{
	return pCurrentData;
}

inline bool CSaveRestoreSegment::Seek( int absPosition )
{
	if ( absPosition < 0 || absPosition >= bufferSize )
		return false;
	
	size = absPosition;
	pCurrentData = pBaseData + size;
	return true;
}

inline void CSaveRestoreSegment::InitSymbolTable( char **pNewTokens, int sizeTable)
{
	Assert( !pTokens );
	tokenCount = sizeTable;
	pTokens = pNewTokens;
	memset( pTokens, 0, sizeTable * sizeof( pTokens[0]) );
}

inline char **CSaveRestoreSegment::DetachSymbolTable()
{
	char **pResult = pTokens;
	tokenCount = 0;
	pTokens = NULL;
	return pResult;
}

inline int CSaveRestoreSegment::SizeSymbolTable()
{
	return tokenCount;
}

inline bool CSaveRestoreSegment::DefineSymbol( const char *pszToken, int token )
{
	if ( pTokens[token] == NULL )
	{
		pTokens[token] = (char *)pszToken;
		return true;
	}
	Assert( 0 );
	return false;
}

inline unsigned short CSaveRestoreSegment::FindCreateSymbol( const char *pszToken )
{
	unsigned short	hash = (unsigned short)(HashString( pszToken ) % (unsigned)tokenCount );
	
#if _DEBUG
	static int tokensparsed = 0;
	tokensparsed++;
	if ( !tokenCount || !pTokens )
	{
		AssertMsg( 0, ("No token table array in TokenHash()!") );
	}
#endif

	for ( int i=0; i<tokenCount; i++ )
	{
#if _DEBUG
		static bool beentheredonethat = false;
		if ( i > 50 && !beentheredonethat )
		{
			beentheredonethat = true;
			AssertMsg( 0, ("CSaveRestoreBuffer::TokenHash() is getting too full!" ) );
		}
#endif

		int	index = hash + i;
		if ( index >= tokenCount )
			index -= tokenCount;

		if ( !pTokens[index] || strcmp( pszToken, pTokens[index] ) == 0 )
		{
			pTokens[index] = (char *)pszToken;
			return index;
		}
	}
		
	// Token hash table full!!! 
	// [Consider doing overflow table(s) after the main table & limiting linear hash table search]
	Warning( "CSaveRestoreBuffer::TokenHash() is COMPLETELY FULL!" );
	Assert( 0 );
	return 0;
}

inline const char *CSaveRestoreSegment::StringFromSymbol( int token )
{
	if ( token >= 0 && token < tokenCount )
		return pTokens[token];
	Assert( 0 );
	return "<<illegal>>";
}

/// XXX(JohnS): I'm not sure using an intrinsic has any value here, just doing the shift should be recognized by most
///             compilers. Either way, there's no portable intrinsic.

// Newer GCC versions provide this in this header, older did by default.
#ifndef _rotr
#ifdef COMPILER_GCC
#include <x86intrin.h>
#elif COMPILER_CLANG
static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_rotr(unsigned int _Value, int _Shift) {
	_Shift &= 0x1f;
	return _Shift ? (_Value >> _Shift) | (_Value << (32 - _Shift)) : _Value;
}
#endif
#endif


inline unsigned int CSaveRestoreSegment::HashString( const char *pszToken )
{
	COMPILE_TIME_ASSERT( sizeof( unsigned int ) == 4 );
	unsigned int	hash = 0;

	while ( *pszToken )
		hash = _rotr( hash, 4 ) ^ *pszToken++;

	return hash;
}

//=============================================================================

#endif // SAVERESTORETYPES_H
