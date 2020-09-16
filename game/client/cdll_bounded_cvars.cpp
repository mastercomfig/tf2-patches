//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "cdll_bounded_cvars.h"
#include "convar_serverbounded.h"
#include "tier0/icommandline.h"


bool g_bForceCLPredictOff = false;

// ------------------------------------------------------------------------------------------ //
// cl_predict.
// ------------------------------------------------------------------------------------------ //

class CBoundedCvar_Predict : public ConVar_ServerBounded
{
public:
	CBoundedCvar_Predict() :
	  ConVar_ServerBounded( "cl_predict", 
		  "1.0", 
#if defined(DOD_DLL) || defined(CSTRIKE_DLL)
		  FCVAR_USERINFO | FCVAR_CHEAT, 
#else
		  FCVAR_USERINFO | FCVAR_NOT_CONNECTED, 
#endif
		  "Perform client side prediction." )
	  {
	  }

	  virtual float GetFloat() const
	  {
		  // Used temporarily for CS kill cam.
		  if ( g_bForceCLPredictOff )
			  return 0;

		  static const ConVar *pClientPredict = g_pCVar->FindVar( "sv_client_predict" );
		  if ( pClientPredict && pClientPredict->GetInt() != -1 )
		  {
			  // Ok, the server wants to control this value.
			  return pClientPredict->GetFloat();
		  }
		  else
		  {
			  return GetBaseFloatValue();
		  }
	  }
};

static CBoundedCvar_Predict cl_predict_var;
ConVar_ServerBounded *cl_predict = &cl_predict_var;



// ------------------------------------------------------------------------------------------ //
// cl_interp_ratio.
// ------------------------------------------------------------------------------------------ //

class CBoundedCvar_InterpRatio : public ConVar_ServerBounded
{
public:
	CBoundedCvar_InterpRatio() :
	  ConVar_ServerBounded( "cl_interp_ratio", 
		  "2.0", 
		  FCVAR_USERINFO | FCVAR_NOT_CONNECTED | FCVAR_ARCHIVE, 
		  "Sets the interpolation amount (final amount is cl_interp_ratio * cl_updateinterval)." )
	  {
	  }

	  virtual float GetFloat() const
	  {
		  static const ConVar *pMin = g_pCVar->FindVar( "sv_client_min_interp_ratio" );
		  static const ConVar *pMax = g_pCVar->FindVar( "sv_client_max_interp_ratio" );
		  if ( pMin && pMax && pMin->GetFloat() != -1 )
		  {
			  return clamp( GetBaseFloatValue(), pMin->GetFloat(), pMax->GetFloat() );
		  }
		  else
		  {
			  return GetBaseFloatValue();
		  }
	  }
};

static CBoundedCvar_InterpRatio cl_interp_ratio_var;
ConVar_ServerBounded *cl_interp_ratio = &cl_interp_ratio_var;

float GetClientInterpAmount()
{
	static const ConVar_ServerBounded* pUpdateInterval = static_cast<const ConVar_ServerBounded*>(g_pCVar->FindVar("cl_updateinterval"));
	if (!pUpdateInterval)
	{
		return 0.03f;
	}
	return cl_interp_ratio->GetFloat() * pUpdateInterval->GetFloat();;
}

