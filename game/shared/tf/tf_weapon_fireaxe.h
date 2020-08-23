//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_FIREAXE_H
#define TF_WEAPON_FIREAXE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFFireAxe C_TFFireAxe
#define CTFBreakableSign C_TFBreakableSign
#endif

//=============================================================================
//
// BrandingIron class.
//
class CTFFireAxe : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFFireAxe, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFFireAxe() {}
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_FIREAXE; }

private:

	CTFFireAxe( const CTFFireAxe & ) {}
};

class CTFBreakableSign : public CTFFireAxe
{
public:

	DECLARE_CLASS(CTFBreakableSign, CTFFireAxe);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFBreakableSign();
	virtual int			GetWeaponID(void) const { return TF_WEAPON_FIREAXE; }
	virtual void		WeaponReset(void);
	virtual void		Smack(void);
	virtual void		SwitchBodyGroups(void);

private:

	CTFBreakableSign(const CTFBreakableSign&) {}

protected:

	CNetworkVar( bool, m_bBroken );
};

#endif // TF_WEAPON_FIREAXE_H
