//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//


#include "mathlib/mathlib.h"
#include "convar.h" // ConVar define
#include "view.h"
#include "gl_cvars.h" // mat_overbright
#include "cmd.h" // Cmd_*
#include "console.h"  // ConMsg

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void InitMathlib( void )
{
	MathLib_Init( 2.2f, // v_gamma.GetFloat()
		2.2f, // v_texgamma.GetFloat()
		0.0f /*v_brightness.GetFloat() */, 
		2.0f /*mat_overbright.GetInt() */ );
}
