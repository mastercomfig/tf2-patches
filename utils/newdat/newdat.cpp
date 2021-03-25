//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Makes .DAT files
//
//========================================================================//

#define _FILE_OFFSET_BITS 64
#include <iostream>
#include <cstdio>
#include "basetypes.h"
#include "checksum_md5.h"
#include "tier1/strtools.h"

namespace {
long long fseek64( FILE *f, long long off, int orig) noexcept
{
#ifdef _WIN32
	return _fseeki64( f, off, orig );
#else
	return fseeko( f, off, orig );
#endif
}

long long ftell64( FILE *f ) noexcept
{
#ifdef _WIN32
	return _ftelli64( f );
#else
	return ftello( f );
#endif
}

bool MD5_Hash_File( byte (&digest)[MD5_DIGEST_LENGTH], const char *fileName, std::ostream &err )
{
	FILE* fp{ fopen( fileName, "rb" ) };
	if (!fp)
	{
		const auto rc = errno;
		err << "Can't open file " << fileName << " to md5: " << strerror(rc) << '.\n';
		return false;
	}

	fseek64( fp, 0, SEEK_END );
	long long nSize{ ftell64(fp) };
	if (nSize <= 0)
	{
		const auto rc = errno;
		err << "Can't read file " << fileName << " size to md5: " << strerror(rc) << '.\n';
		fclose( fp );
		return false;
	}

	fseek64( fp, 0, SEEK_SET );

	MD5Context_t ctx;
	memset( &ctx, 0, sizeof(ctx) );
	MD5Init( &ctx );

	byte chunk[1024];
	// Now read in 1K chunks.
	while (nSize > 0)
	{
		const size_t nBytesRead{ fread(chunk, 1, min(std::size(chunk), nSize), fp) };
		// If any data was received, CRC it.
		if (nBytesRead > 0)
		{
			nSize -= nBytesRead;
			MD5Update( &ctx, chunk, nBytesRead );
		}

		// We we are end of file, break loop and return.
		if ( feof( fp ) )
		{
			fclose( fp );
			fp = nullptr;
			break;
		}
		else if ( ferror(fp) )
		{
			err << "Can't read file " << fileName << " to md5: read error occured.\n";
			// If there was a disk error, indicate failure.
			if ( fp )
				fclose( fp );
			return false;
		}
	}

	if ( fp )
		fclose( fp );

	MD5Final( digest, &ctx );

	return true;
}
}

//-----------------------------------------------------------------------------
// Purpose: newdat.exe - makes the .DAT signature for file / virus checking
// Input  : argc - std args
//			*argv[] - 
// Output : int 0 == success. 1 == failure
//-----------------------------------------------------------------------------
int main( int argc, char *argv[] )
{
	if ( argc < 2 )
	{
		std::cerr << "USAGE: newdat <filename>\n";
		return 1;
	}

	char fileName[MAX_PATH];
	// Get the filename without the extension
	Q_StripExtension( argv[1], fileName, std::size( fileName ) );

	char datFileName[MAX_PATH];
	sprintf( datFileName, "%s.dat", fileName );

	byte digest[MD5_DIGEST_LENGTH];
	// Build the MD5 hash for the .EXE file
	if ( !MD5_Hash_File( digest, argv[1], std::cerr ) )
	{
		std::cerr << "Can't md5 file " << argv[1] << ".\n";
		return 2;
	}

	// Write the first 4 bytes of the MD5 hash as the signature ".dat" file
	FILE* fp{ fopen( datFileName, "wb" ) };
	if ( fp )
	{
		if ( fwrite( digest, sizeof(int), 1, fp ) != 1 )
		{
			std::cerr << "Can't write md5 " << sizeof(int) << " bytes to " << datFileName << ".\n";
			fclose( fp );
			return 3;
		}

		if ( fclose( fp ) != EOF )
		{
			std::cout << "Wrote md5 of " << argv[1] << " to " << datFileName << ".\n";
			return 0;
		}

		std::cerr << "Can't close " << datFileName << " to write md5 of " << argv[1] << " to: file may be not written.\n";
		return 4;
	}

	const auto rc = errno;
	std::cerr << "Can't open " << datFileName << " to write md5 of " << argv[1] << " to: " << strerror( rc ) << ".\n";
	return 5;
}
