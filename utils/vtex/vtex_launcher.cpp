//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <cstdio>
#include <system_error>
#include "tier1/interface.h"
#include "ilaunchabledll.h"

extern "C" __declspec(dllimport) unsigned long __stdcall GetLastError(void);

int main( int argc, char **argv )
{
	constexpr char pModuleName[]{ "vtex_dll.dll" };
	
	CSysModule* pModule{ Sys_LoadModule(pModuleName) };
	if ( !pModule )
	{
		const auto error = std::system_category().message(static_cast<int>(::GetLastError()));
		fprintf( stderr, "Can't load %s\n%s", pModuleName, error.c_str() );
		return 1;
	}

	CreateInterfaceFn fn{ Sys_GetFactory(pModule) };
	if ( !fn )
	{
		fprintf( stderr, "Can't get factory from %s.", pModuleName );
		Sys_UnloadModule( pModule );
		return 1;
	}

	auto *pDll = static_cast<ILaunchableDLL*>( fn( LAUNCHABLE_DLL_INTERFACE_VERSION, nullptr ) );
	if ( !pDll )
	{
		fprintf( stderr, "Can't get '%s' interface from %s.", LAUNCHABLE_DLL_INTERFACE_VERSION, pModuleName );
		Sys_UnloadModule( pModule );
		return 1;
	}

	const int rc{ pDll->main(argc, argv) };
	Sys_UnloadModule( pModule );
	return rc;
}
