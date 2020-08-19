//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:  baseclientstate.cpp: implementation of the CBaseClientState class.
//
//=============================================================================//

#include "client.h"
#include "convar.h"
#include "convar_serverbounded.h"
#include "sys.h"
#include "net.h"


// These are the server cvars that control our cvars.
extern int ClampClientRate( int nRate );

extern ConVar  sv_mincmdinterval;
extern ConVar  sv_maxcmdinterval;
extern ConVar  sv_minupdateinterval;
extern ConVar  sv_maxupdateinterval;
extern ConVar  sv_client_cmdinterval_difference;

extern ConVar  sv_client_interp;
extern ConVar  sv_client_predict;


// ------------------------------------------------------------------------------------------ //
// rate
// ------------------------------------------------------------------------------------------ //

class CBoundedCvar_Rate : public ConVar_ServerBounded
{
public:
	CBoundedCvar_Rate() :
	  ConVar_ServerBounded( 
		  "rate",
#if defined( _X360 )
		  "6000",
#else
		  V_STRINGIFY(DEFAULT_RATE),
#endif
		  FCVAR_ARCHIVE | FCVAR_USERINFO, 
		  "Max bytes/sec the host can receive data" )
	  {
	  }

	virtual float GetFloat() const
	{
		if ( cl.m_nSignonState >= SIGNONSTATE_FULL )
		{
			int nRate = (int)GetBaseFloatValue();
			return (float)ClampClientRate( nRate );
		}
		else
		{
			return GetBaseFloatValue();
		}
	}
};

static CBoundedCvar_Rate cl_rate_var;
ConVar_ServerBounded *cl_rate = &cl_rate_var;


// ------------------------------------------------------------------------------------------ //
// cl_cmdinterval
// ------------------------------------------------------------------------------------------ //

class CBoundedCvar_CmdInterval : public ConVar_ServerBounded
{
public:
	CBoundedCvar_CmdInterval() :
	  ConVar_ServerBounded( 
		  "cl_cmdinterval", 
		  "0.015", 
		  FCVAR_ARCHIVE | FCVAR_USERINFO, 
		  "Max number of command packets sent to server per second", true, MIN_CMD_RATE, true, MAX_CMD_RATE )
	{
	}

	virtual float GetFloat() const
	{
		float flCmdInterval = GetBaseFloatValue();

		if ( sv_mincmdinterval.GetInt() != 0 && cl.m_nSignonState >= SIGNONSTATE_FULL )
		{
			// First, we make it stay within range of cl_updateinterval.
			float diff = flCmdInterval - cl_updateinterval->GetFloat();
			if ( fabs( diff ) > sv_client_cmdinterval_difference.GetFloat() )
			{
				if ( diff > 0 )
					flCmdInterval = cl_updateinterval->GetFloat() + sv_client_cmdinterval_difference.GetFloat();
				else
					flCmdInterval = cl_updateinterval->GetFloat() - sv_client_cmdinterval_difference.GetFloat();
			}

			// Then we clamp to the min/max values the server has set.
			return clamp( flCmdInterval, sv_mincmdinterval.GetFloat(), sv_maxcmdinterval.GetFloat() );
		}
		else
		{
			return flCmdInterval;
		}
	}
};

static CBoundedCvar_CmdInterval cl_cmdinterval_var;
ConVar_ServerBounded *cl_cmdinterval = &cl_cmdinterval_var;


// ------------------------------------------------------------------------------------------ //
// cl_updateinterval
// ------------------------------------------------------------------------------------------ //
class CBoundedCvar_UpdateInterval : public ConVar_ServerBounded
{
public:
	CBoundedCvar_UpdateInterval() :
		ConVar_ServerBounded(
			"cl_updateinterval",
			"0.015",
			FCVAR_ARCHIVE | FCVAR_USERINFO | FCVAR_NOT_CONNECTED,
			"Time between packets for updates you are requesting from the server")
	{
	}

	virtual float GetFloat() const
	{
		// Clamp to the min/max values the server has set.
		//
		// This cvar only takes effect on the server anyway, and this is done there too,
		// but we have this here so they'll get the **note thing telling them the value 
		// isn't functioning the way they set it.		
		return clamp(GetBaseFloatValue(), sv_minupdateinterval.GetFloat(), sv_maxupdateinterval.GetFloat());
	}
};

static CBoundedCvar_UpdateInterval cl_updateinterval_var;
ConVar_ServerBounded* cl_updateinterval = &cl_updateinterval_var;
