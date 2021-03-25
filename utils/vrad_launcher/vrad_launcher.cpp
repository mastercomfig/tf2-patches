//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// vrad_launcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <direct.h>
#include <system_error>
#include "tier1/strtools.h"
#include "tier0/icommandline.h"

namespace
{
void MakeFullPath( const char *pIn, char *pOut, int outLen )
{
	if ( pIn[0] == '/' || pIn[0] == '\\' || pIn[1] == ':' )
	{
		// It's already a full path.
		Q_strncpy( pOut, pIn, outLen );
	}
	else
	{
		_getcwd( pOut, outLen );
		Q_strncat( pOut, "\\", outLen, COPY_ALL_CHARACTERS );
		Q_strncat( pOut, pIn, outLen, COPY_ALL_CHARACTERS );
	}
}
}  // namespace

extern "C" __declspec(dllimport) unsigned long __stdcall GetLastError(void);

int main(int argc, char* argv[])
{
	CommandLine()->CreateCmdLine( argc, const_cast<const char **>(argv) );

	// check whether they used the -both switch. If this is specified, vrad will be run
	// twice, once with -hdr and once without
	ptrdiff_t both_arg{ 0 };
	for (ptrdiff_t arg{ 1 }; arg < argc; arg++)
	{
		if (Q_stricmp(argv[arg], "-both") == 0)
		{
			both_arg = arg;
		}
	}

	char fullPath[MAX_PATH], redirectFilename[MAX_PATH];
	MakeFullPath( argv[0], fullPath, sizeof( fullPath ) );
	Q_StripFilename( fullPath );
	Q_snprintf( redirectFilename, sizeof( redirectFilename ), "%s\\%s", fullPath, "vrad.redirect" );

	// First, look for vrad.redirect and load the dll specified in there if possible.
	char dllName[MAX_PATH];
	CSysModule* pModule{ nullptr };
	FILE* fp{ fopen(redirectFilename, "rt") };
	if ( fp )
	{
		if ( fgets( dllName, sizeof( dllName ), fp ) )
		{
			char* pEnd{ strstr(dllName, "\n") };
			if ( pEnd )
				*pEnd = '\0';

			pModule = Sys_LoadModule( dllName );
			if ( pModule )
				printf( "Loaded alternate VRAD DLL (%s) specified in vrad.redirect.\n", dllName );
			else
				fprintf( stderr, "Can't find '%s' specified in vrad.redirect.\n", dllName );
		}
		
		fclose( fp );
	}

	int rc{ 0 };
	
	for (int mode{ 0 }; mode < 2; mode++)
	{
		if (mode && (! both_arg))
			continue;

		// If it didn't load the module above, then use the 
		if ( !pModule )
		{
			strcpy( dllName, "vrad_dll.dll" );
			pModule = Sys_LoadModule( dllName );
		}
		
		if( !pModule )
		{
			const auto error = std::system_category().message( static_cast<int>(::GetLastError()) );
			fprintf( stderr, "vrad_launcher error: can't load %s\n%s", dllName, error.c_str() );
			return 1;
		}
		
		CreateInterfaceFn fn{ Sys_GetFactory( pModule ) };
		if( !fn )
		{
			const auto error = std::system_category().message( static_cast<int>(::GetLastError()) );
			fprintf( stderr, "vrad_launcher error: can't get factory from vrad_dll.dll\n%s", error.c_str() );
			Sys_UnloadModule( pModule );
			return 2;
		}

		auto *pDLL = static_cast<IVRadDLL*>( fn( VRAD_INTERFACE_VERSION, nullptr ) );
		if( !pDLL )
		{
			fprintf( stderr, "vrad_launcher error: can't get IVRadDLL interface from vrad_dll.dll\n" );
			Sys_UnloadModule( pModule );
			return 3;
		}
		
		if (both_arg)
			strcpy(argv[both_arg], mode ? "-hdr" : "-ldr");

		rc = pDLL->main( argc, argv );

		Sys_UnloadModule( pModule );
		pModule = nullptr;
	}

	return rc;
}
