//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// vvis_launcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <system_error>
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
#include "ilaunchabledll.h"

extern "C" __declspec(dllimport) unsigned long __stdcall GetLastError(void);

int main(int argc, char* argv[])
{
	CommandLine()->CreateCmdLine( argc, const_cast<const char**>(argv) );
	constexpr char pDLLName[]{ "vvis_dll.dll" };
	
	CSysModule* pModule{ Sys_LoadModule(pDLLName) };
	if ( !pModule )
	{
		const auto error = std::system_category().message(static_cast<int>(::GetLastError()));
		fprintf( stderr, "vvis launcher error: can't load %s\n%s", pDLLName, error.c_str() );
		return 1;
	}

	CreateInterfaceFn fn{ Sys_GetFactory(pModule) };
	if( !fn )
	{
		fprintf( stderr, "vvis launcher error: can't get factory from %s\n", pDLLName );
		Sys_UnloadModule( pModule );
		return 2;
	}

	auto *pDLL = static_cast<ILaunchableDLL*>( fn( LAUNCHABLE_DLL_INTERFACE_VERSION, nullptr ) );
	if( !pDLL )
	{
		fprintf( stderr, "vvis launcher error: can't get IVVisDLL interface from %s\n", pDLLName );
		Sys_UnloadModule( pModule );
		return 3;
	}

	const int rc{ pDLL->main(argc, argv) };
	Sys_UnloadModule( pModule );

	return rc;
}
