//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs 
{
	/** @addtogroup Image
	 *  @{
	 */

	typedef UINT32 RGBA;
	typedef UINT32 ARGB;
	typedef UINT32 ABGR;
	typedef UINT32 BGRA;

	/** 
	 * Color represented as 4 components, each being a floating point value ranging from 0 to 1. Color components are 
	 * red, green, blue and alpha.
	 */
	class BS_UTILITY_EXPORT Color
	{
	public:
		static const Color ZERO;
		static const Color Black;
		static const Color White;
		static const Color Red;
		static const Color Green;
		static const Color Blue;

		explicit Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f ) 
			:r(red), g(green), b(blue), a(alpha)
		{ }

		bool operator==(const Color& rhs) const;
		bool operator!=(const Color& rhs) const;

		/** Returns the color as a 32-bit value in RGBA order. */
		RGBA getAsRGBA() const;

		/** Returns the color as a 32-bit value in ARGB order. */
		ARGB getAsARGB() const;

		/** Returns the color as a 32-bit value in BGRA order. */
		BGRA getAsBGRA() const;

		/** Returns the color as a 32-bit value in ABGR order. */
		ABGR getAsABGR() const;

		/** Assigns the color from a 32-bit value that encodes a RGBA color. */
		void setAsRGBA(RGBA val);

		/** Assigns the color from a 32-bit value that encodes a ARGB color. */
		void setAsARGB(ARGB val);

		/** Assigns the color from a 32-bit value that encodes a BGRA color. */
		void setAsBGRA(BGRA val);

		/** Assigns the color from a 32-bit value that encodes a ABGR color. */
		void setAsABGR(ABGR val);

		/** Clamps color value to the range [0, 1]. */
		void saturate()
		{
			if (r < 0)
				r = 0;
			else if (r > 1)
				r = 1;

			if (g < 0)
				g = 0;
			else if (g > 1)
				g = 1;

			if (b < 0)
				b = 0;
			else if (b > 1)
				b = 1;

			if (a < 0)
				a = 0;
			else if (a > 1)
				a = 1;
		}

		/** Clamps colour value to the range [0, 1]. Returned saturated color as a copy. */
		Color saturateCopy() const
		{
			Color ret = *this;
			ret.saturate();
			return ret;
		}

		float operator[] (const UINT32 i) const
		{
			assert(i < 4);

			return *(&r+i);
		}

		float& operator[] (const UINT32 i)
		{
			assert(i < 4);

			return *(&r+i);
		}

		/** Pointer accessor for direct copying. */
		float* ptr()
		{
			return &r;
		}

		/** Pointer accessor for direct copying. */
		const float* ptr() const
		{
			return &r;
		}

		Color operator+ (const Color& rhs) const
		{
			Color sum;

			sum.r = r + rhs.r;
			sum.g = g + rhs.g;
			sum.b = b + rhs.b;
			sum.a = a + rhs.a;

			return sum;
		}

		Color operator- (const Color& rhs) const
		{
			Color diff;

			diff.r = r - rhs.r;
			diff.g = g - rhs.g;
			diff.b = b - rhs.b;
			diff.a = a - rhs.a;

			return diff;
		}

		Color operator* (float rhs) const
		{
			Color prod;

			prod.r = rhs*r;
			prod.g = rhs*g;
			prod.b = rhs*b;
			prod.a = rhs*a;

			return prod;
		}

		Color operator* (const Color& rhs) const
		{
			Color prod;

			prod.r = rhs.r * r;
			prod.g = rhs.g * g;
			prod.b = rhs.b * b;
			prod.a = rhs.a * a;

			return prod;
		}

		Color operator/ (const Color& rhs) const
		{
			Color prod;

			prod.r = rhs.r / r;
			prod.g = rhs.g / g;
			prod.b = rhs.b / b;
			prod.a = rhs.a / a;

			return prod;
		}

		Color operator/ (float rhs) const
		{
			assert(rhs != 0.0f);

			Color div;

			float invRhs = 1.0f / rhs;
			div.r = r * invRhs;
			div.g = g * invRhs;
			div.b = b * invRhs;
			div.a = a * invRhs;

			return div;
		}

		friend Color operator* (float lhs, const Color& rhs)
		{
			Color result;

			result.r = lhs * rhs.r;
			result.g = lhs * rhs.g;
			result.b = lhs * rhs.b;
			result.a = lhs * rhs.a;

			return result;
		}

		Color& operator+= (const Color& rhs)
		{
			r += rhs.r;
			g += rhs.g;
			b += rhs.b;
			a += rhs.a;

			return *this;
		}

		Color& operator-= (const Color& rhs)
		{
			r -= rhs.r;
			g -= rhs.g;
			b -= rhs.b;
			a -= rhs.a;

			return *this;
		}

		Color& operator*= (float rhs)
		{
			r *= rhs;
			g *= rhs;
			b *= rhs;
			a *= rhs;

			return *this;
		}

		Color& operator/= (float rhs)
		{
			assert(rhs != 0.0f);

			float invRhs = 1.0f / rhs;

			r *= invRhs;
			g *= invRhs;
			b *= invRhs;
			a *= invRhs;

			return *this;
		}

		/** 
		 * Set a color value from hue, saturation and brightness.
		 *
		 * @param[in] hue			Hue value, scaled to the [0,1] range.
		 * @param[in] saturation	Saturation level, [0,1].
		 * @param[in] brightness	Brightness level, [0,1].
		 */
		void setHSB(float hue, float saturation, float brightness);

		/** 
		 * Convert the current color to hue, saturation and brightness values. 
		 * 
		 * @param[in] hue			Output hue value, scaled to the [0,1] range.
		 * @param[in] saturation	Output saturation level, [0,1].
		 * @param[in] brightness	Output brightness level, [0,1].
		 */
		void getHSB(float* hue, float* saturation, float* brightness) const;

		/**
		 * Linearly interpolates between the two colors using @p t. t should be in [0, 1] range, where t = 0 corresponds
		 * to the left color, while t = 1 corresponds to the right color.
		 */
		static Color lerp(float t, const Color& a, const Color& b);

		float r, g, b, a;
	};

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Color);
	/** @endcond */

	/** @} */
}

/** @cond SPECIALIZATIONS */

namespace std
{
/** Hash value generator for Color. */
template<> 
struct hash<bs::Color>
{
	size_t operator()(const bs::Color& color) const
	{
		size_t hash = 0;
		bs::hash_combine(hash, color.r);
		bs::hash_combine(hash, color.g);
		bs::hash_combine(hash, color.b);
		bs::hash_combine(hash, color.a);

		return hash;
	}
};
}

/** @endcond */
