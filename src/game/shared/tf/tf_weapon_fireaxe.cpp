//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_fireaxe.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon FireAxe tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFFireAxe, DT_TFWeaponFireAxe )

BEGIN_NETWORK_TABLE( CTFFireAxe, DT_TFWeaponFireAxe )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFFireAxe )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_fireaxe, CTFFireAxe );
PRECACHE_WEAPON_REGISTER( tf_weapon_fireaxe );

//=============================================================================
//
// Weapon Breakable Sign
//
IMPLEMENT_NETWORKCLASS_ALIASED(TFBreakableSign, DT_TFWeaponBreakableSign)

BEGIN_NETWORK_TABLE(CTFBreakableSign, DT_TFWeaponBreakableSign)
#if defined( CLIENT_DLL )
    RecvPropBool(RECVINFO(m_bBroken))
#else
    SendPropBool(SENDINFO(m_bBroken))
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CTFBreakableSign)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(tf_weapon_breakable_sign, CTFBreakableSign);
PRECACHE_WEAPON_REGISTER(tf_weapon_breakable_sign);

CTFBreakableSign::CTFBreakableSign()
{
}

void CTFBreakableSign::WeaponReset()
{
	BaseClass::WeaponReset();

	m_bBroken = false;
}

void CTFBreakableSign::SwitchBodyGroups(void)
{
	int iState = m_bBroken ? 1 : 0;

	SetBodygroup(0, iState);

	CTFPlayer* pTFPlayer = ToTFPlayer(GetOwner());

	if (pTFPlayer && pTFPlayer->GetActiveWeapon() == this)
	{
		if (pTFPlayer->GetViewModel())
		{
			pTFPlayer->GetViewModel()->SetBodygroup(0, iState);
		}
	}
}

void CTFBreakableSign::Smack(void)
{
	BaseClass::Smack();

	if (!m_bBroken && ConnectedHit() && IsCurrentAttackACrit())
	{
		m_bBroken = true;
		SwitchBodyGroups();
	}
}
