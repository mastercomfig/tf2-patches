//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef COLOR_H
#define COLOR_H

#ifdef _WIN32
#pragma once
#endif

#include <cstring>  // memset, memcpy, memcmp

//-----------------------------------------------------------------------------
// Purpose: Basic handler for an rgb set of colors
//			This class is fully inline
//-----------------------------------------------------------------------------
class Color
{
public:
	// constructors
	Color() noexcept
	{
		memset(this, 0, sizeof(*this));
	}
	Color(int _r,int _g,int _b) noexcept
	{
		SetColor(_r, _g, _b, 0);
	}
	Color(int _r,int _g,int _b,int _a) noexcept
	{
		SetColor(_r, _g, _b, _a);
	}
	
	// set the color
	// r - red component (0-255)
	// g - green component (0-255)
	// b - blue component (0-255)
	// a - alpha component, controls transparency (0 - transparent, 255 - opaque);
	void SetColor(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 0) noexcept
	{
		_color[0] = _r;
		_color[1] = _g;
		_color[2] = _b;
		_color[3] = _a;
	}

	void GetColor(int &_r, int &_g, int &_b, int &_a) const noexcept
	{
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	void SetRawColor( int color32 ) noexcept
	{
		static_assert(sizeof(int) == sizeof(*this));
		memcpy(this, &color32, sizeof(*this));
	}

	int GetRawColor() const noexcept
	{
		int raw;
		static_assert(sizeof(int) == sizeof(*this));
		memcpy(&raw, this, sizeof(*this));
		return raw;
	}

	inline int r() const noexcept { return _color[0]; }
	inline int g() const noexcept { return _color[1]; }
	inline int b() const noexcept { return _color[2]; }
	inline int a() const noexcept { return _color[3]; }
	
	unsigned char &operator[](int index) noexcept
	{
		return _color[index];
	}

	const unsigned char &operator[](int index) const noexcept
	{
		return _color[index];
	}

	bool operator == (const Color &rhs) const noexcept
	{
		return memcmp(this, &rhs, sizeof(*this)) == 0;
	}

	bool operator != (const Color &rhs) const noexcept
	{
		return !(operator==(rhs));
	}

	Color( const Color& rhs ) noexcept
	{
		SetRawColor( rhs.GetRawColor() );
	}

	Color( Color&& rhs ) noexcept
	{
		SetRawColor( rhs.GetRawColor() );
	}

	Color &operator=( const Color &rhs ) noexcept
	{
		SetRawColor( rhs.GetRawColor() );
		return *this;
	}

	Color& operator=( Color&& rhs ) noexcept
	{
		SetRawColor( rhs.GetRawColor() );
		return *this;
	}

private:
	unsigned char _color[4];
};


#endif // COLOR_H
