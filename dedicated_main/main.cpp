//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//-----------------------------------------------------------------------------
// This is just a little redirection tool so I can get all the dlls in bin
//-----------------------------------------------------------------------------

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <direct.h>
#include <system_error>
#elif POSIX
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#define MAX_PATH PATH_MAX
#endif
#include "basetypes.h"

#ifdef _WIN32
using DedicatedMain_t = int (*)( HINSTANCE hInstance, HINSTANCE hPrevInstance,
							  LPSTR lpCmdLine, int nCmdShow );
#elif POSIX
using DedicatedMain_t = int (*)( int argc, char *argv[] );
#endif

namespace {

//-----------------------------------------------------------------------------
// Purpose: Return the directory where this .exe is running from.
// Output : wchar_t
//-----------------------------------------------------------------------------
template<size_t bufferSize>
[[nodiscard]] const wchar_t* GetBaseDir( const wchar_t(&szBuffer)[bufferSize] )
{
	static wchar_t basedir[MAX_PATH];
	wcscpy_s( basedir, szBuffer );

	wchar_t* pBuffer{ wcsrchr( basedir, L'\\' ) };
	if ( pBuffer )
	{
		*(pBuffer + 1) = L'\0';
	}

	const size_t j = wcslen( basedir );
	if ( j > 0 )
	{
		wchar_t& lastChar = basedir[j-1];
		if ( lastChar == L'\\' || lastChar == L'/' )
		{
			lastChar = L'\0';
		}
	}

	return basedir;
}

//-----------------------------------------------------------------------------
// Purpose: Error codes.
//-----------------------------------------------------------------------------
enum class ErrorCode : int
{
	None = 0,
	CantGetModuleFileName,
	CantUpdatePathEnvVariable,
	CantLoadDedicatedDll,
	CantFindDedicatedMainInDedicatedDll
};

//-----------------------------------------------------------------------------
// Purpose: Shows error box and returns error code.
// Output : int
//-----------------------------------------------------------------------------
[[nodiscard]] int ShowErrorBoxAndExitWithCode( const wchar_t* szError, ErrorCode errorCode )
{
	MessageBoxW( 0, szError, L"Launcher Error", MB_OK | MB_ICONERROR );
	return static_cast<int>(errorCode);
}

}  // namespace

#ifdef WIN32

int APIENTRY WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{
	// Use the .EXE name to determine the root directory.
	wchar_t moduleName[MAX_PATH];
	if ( !GetModuleFileNameW( hInstance, moduleName, MAX_PATH ) )
	{
		return ShowErrorBoxAndExitWithCode( L"Failed calling GetModuleFileName.", ErrorCode::CantGetModuleFileName );
	}

	// Get the root directory the .exe is in.
	const wchar_t* rootDirPath{ GetBaseDir( moduleName ) };
	// x64: Use subdir as CS:GO does. Allows to have both x86/x86-64 binaries and choose game bitness.
	constexpr wchar_t binDirPath[] =
#ifdef _WIN64
		L"\\x64"
#else
		L""
#endif
		;
	wchar_t buffer[4096];
	// Must add 'bin' to the path...
	const wchar_t* oldPathEnv{ _wgetenv( L"PATH" ) };
	if ( oldPathEnv )
	{
		swprintf_s( buffer, L"PATH=%s\\bin%s\\;%s", rootDirPath, binDirPath, oldPathEnv );
		if ( _wputenv( buffer ) == -1 )
		{
			return ShowErrorBoxAndExitWithCode( L"Failed to update PATH env variable.", ErrorCode::CantUpdatePathEnvVariable );
		}
	}

	// Assemble the full path to our "dedicated.dll".
	swprintf_s( buffer, L"%s\\bin%s\\dedicated.dll", rootDirPath, binDirPath );

	// STEAM OK ... filesystem not mounted yet.
	const HINSTANCE dedicated{ LoadLibraryExW( buffer, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH ) };
	if (dedicated)
	{
		const auto main = reinterpret_cast<DedicatedMain_t>( GetProcAddress(dedicated, "DedicatedMain") );

		return main
			? main( hInstance, hPrevInstance, lpCmdLine, nCmdShow )
			: ShowErrorBoxAndExitWithCode( L"Failed to get \"DedicatedMain\" entry point in the dedicated DLL.",
					ErrorCode::CantFindDedicatedMainInDedicatedDll );
	}

	const auto lastErrorText = std::system_category().message( GetLastError() );

	wchar_t userErrorText[1024];
	swprintf_s( userErrorText, L"Failed to load the dedicated DLL:\n\n%S", lastErrorText.c_str() );

	return ShowErrorBoxAndExitWithCode( userErrorText, ErrorCode::CantLoadDedicatedDll );
}

#elif POSIX

#if defined( LINUX )

#include <fcntl.h>

static bool IsDebuggerPresent( int time )
{
	// Need to get around the __wrap_open() stuff. Just find the open symbol
	// directly and use it...
	typedef int (open_func_t)( const char *pathname, int flags, mode_t mode );
	open_func_t *open_func = (open_func_t *)dlsym( RTLD_NEXT, "open" );

	if ( open_func )
	{
		for ( int i = 0; i < time; i++ )
		{
			int tracerpid = -1;

			int fd = (*open_func)( "/proc/self/status", O_RDONLY, S_IRUSR );
			if (fd >= 0)
			{
				char buf[ 4096 ];
				static const char tracerpid_str[] = "TracerPid:";

				const int len = read( fd, buf, sizeof(buf) - 1 );
				if ( len > 0 )
				{
					buf[ len ] = 0;

					const char *str = strstr( buf, tracerpid_str );
					tracerpid = str ? atoi( str + sizeof( tracerpid_str ) ) : -1;
				}

				close( fd );
			}

			if ( tracerpid > 0 )
				return true;

			sleep( 1 );
		}
	}

	return false;
}

static void WaitForDebuggerConnect( int argc, char *argv[], int time )
{
	for ( int i = 1; i < argc; i++ )
	{
		if ( strstr( argv[i], "-wait_for_debugger" ) )
		{
			printf( "\nArg -wait_for_debugger found.\nWaiting %dsec for debugger...\n", time );
			printf( "  pid = %d\n", getpid() );

			if ( IsDebuggerPresent( time ) )
				printf("Debugger connected...\n\n");

			break;
		}
	}
}

#else

static void WaitForDebuggerConnect( int argc, char *argv[], int time )
{
}

#endif // !LINUX

int main( int argc, char *argv[] )
{
	// Must add 'bin' to the path....
	char* pPath = getenv("LD_LIBRARY_PATH");
	char szBuffer[4096];
	char cwd[ MAX_PATH ];
	if ( !getcwd( cwd, sizeof(cwd)) )
	{
		printf( "getcwd failed (%s)", strerror(errno));
	}

	snprintf( szBuffer, sizeof( szBuffer ) - 1, "LD_LIBRARY_PATH=%s/bin:%s", cwd, pPath );
	int ret = putenv( szBuffer );
	if ( ret )	
	{
		printf( "%s\n", strerror(errno) );
	}
	void *tier0 = dlopen( "libtier0" DLL_EXT_STRING, RTLD_NOW );
	void *vstdlib = dlopen( "libvstdlib" DLL_EXT_STRING, RTLD_NOW );

	const char *pBinaryName = "dedicated" DLL_EXT_STRING;

	void *dedicated = dlopen( pBinaryName, RTLD_NOW );
	if ( !dedicated )
	{
		printf( "Failed to open %s (%s)\n", pBinaryName, dlerror());
		return -1;
	}
	DedicatedMain_t dedicated_main = (DedicatedMain_t)dlsym( dedicated, "DedicatedMain" );
	if ( !dedicated_main )
	{
		printf( "Failed to find dedicated server entry point (%s)\n", dlerror() );
		return -1;
	}

	WaitForDebuggerConnect( argc, argv, 30 );

	ret = dedicated_main( argc,argv );
	dlclose( dedicated );
	dlclose( vstdlib );
	dlclose( tier0 );
}
#endif
