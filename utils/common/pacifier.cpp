//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>
#include "basetypes.h"
#include "pacifier.h"
#include "tier0/dbg.h"
#include "threadtools.h"


static int g_LastPacifierDrawn = -1;
static bool g_bPacifierSuppressed = false;

#define clamp(a,b,c) ( (a) > (c) ? (c) : ( (a) < (b) ? (b) : (a) ) )

void StartPacifier( char const *pPrefix )
{
	Msg( "%s", pPrefix );
	g_LastPacifierDrawn = -1;
	UpdatePacifier( 0.001f );
}

void UpdatePacifier( float flPercent )
{
	int iCur = (int)(flPercent * 100.0f);
	iCur = clamp( iCur, g_LastPacifierDrawn, 100 );
	
	if (iCur != g_LastPacifierDrawn && !g_bPacifierSuppressed)
	{
		if (showprogress < 1)
		{
			return;
		}
		else if (showprogress == 1)
		{
			for (int i = g_LastPacifierDrawn + 1; i <= iCur; i++)
			{
				if (!(i % 10))
				{
					Msg("%d", i / 10);
				}
				else
				{
					if (i != 100)
					{
						Msg(".");
					}
				}
			}
		}
		else if (showprogress == 2)
		{
			for (int i = g_LastPacifierDrawn + 1; i <= iCur; i++)
			{
				if (!(i % 10))
				{
					Msg("%d", i / 10);
					Msg("%d%%\b\b\b", 0);
				}
			}
		}
		else if (showprogress == 3)
		{
			for (int i = g_LastPacifierDrawn + 1; i <= iCur; i++)
			{
				if (!(i % 10) || i % 10 == 5)
				{
					Msg("%d%d", i / 10, 0);

					if (i % 10 == 5)
					{
						Msg("\b%d", 5);
					}
					else if (!(i % 10))
					{
						Msg("\b%d", 0);
					}
					Msg("%%\b\b\b");
				}
			}

		}
		else if (showprogress == 4)
		{
			for (int i = g_LastPacifierDrawn + 1; i <= iCur; i++)
			{
				Msg("%d%d", i / 10, 0);
				if (i % 10)
				{
					Msg("\b%d", i % 10);
				}
				Msg("%%\b\b\b");
			}
		}
		g_LastPacifierDrawn = iCur;
	}
}

void EndPacifier( bool bCarriageReturn )
{
	UpdatePacifier(1);
	
	if( bCarriageReturn && !g_bPacifierSuppressed )
		Msg("\n");
}

void SuppressPacifier( bool bSuppress )
{
	g_bPacifierSuppressed = bSuppress;
}
