/*  Project Starshatter Wars
	Fractal Dev Games
	Copyright (C) 2024. All Rights Reserved.

	SUBSYSTEM:    Foundation
	FILE:         Fix.h
	AUTHOR:       Carlos Bott

	OVERVIEW
	========
	Fixed point number class with 16 bits of fractional precision
*/

#pragma once

#include "CoreMinimal.h"
#include "Types.h"

/**
 * 
 */

// +--------------------------------------------------------------------+

const double fix_sixty_five = 65536.0;

inline int fast_f2i(double d)
{
	return FMath::FloorToInt(d);
}

// +--------------------------------------------------------------------+

class STARSHATTERWARS_API fix
{
public:
	static const char* TYPENAME() { return "fix"; }

	enum FixDef { Precision = 16, IntMask = 0xffff0000, FractMask = 0x0000ffff };
	static const fix     one;
	static const fix     two;
	static const fix     three;
	static const fix     five;
	static const fix     ten;

	fix() { }
	fix(int n) : val(n << Precision) { }
	fix(double d) : val(d) {}
	fix(const fix& f) : val(f.val) { }

	// conversion operators:
	operator int() const { return (val >> Precision); }
	operator float() const { return ((float)val) / ((float)fix_sixty_five); }
	operator double() const { return ((double)val) / fix_sixty_five; }

	// assignment operators:
	fix& operator=(const fix& f) { val = f.val; return *this; }
	fix& operator=(int n) { val = (n << Precision); return *this; }
	fix& operator=(double d) { val = d; return *this; }

	// comparison operators:
	int operator==(const fix& f) const { return val == f.val; }
	int operator!=(const fix& f) const { return val != f.val; }
	int operator<=(const fix& f) const { return val <= f.val; }
	int operator>=(const fix& f) const { return val >= f.val; }
	int operator< (const fix& f) const { return val < f.val; }
	int operator> (const fix& f) const { return val > f.val; }

	// arithmetic operators:
	fix  operator+(const fix& f) const { fix r; r.val = val + f.val; return r; }
	fix  operator-(const fix& f) const { fix r; r.val = val - f.val; return r; }
	fix  operator*(const fix& f) const { fix r; r.val = val * f.val; return r; }
	fix  operator/(const fix& f) const { fix r; r.val = val / f.val; return r; }

	fix& operator+=(const fix& f) { val += f.val; return *this; }
	fix& operator-=(const fix& f) { val -= f.val; return *this; }
	fix& operator*=(const fix& f) { val *= f.val; return *this; }
	fix& operator/=(const fix& f) { val /= f.val; return *this; }

	fix  operator+(int n) const { fix r; r.val = val + (n << Precision); return r; }
	fix  operator-(int n) const { fix r; r.val = val - (n << Precision); return r; }
	fix  operator*(int n) const { fix r; r.val = val * n; return r; }
	fix  operator/(int n) const { fix r; r.val = val / n; return r; }
	fix& operator+=(int n) { val += (n << Precision); return *this; }
	fix& operator-=(int n) { val -= (n << Precision); return *this; }
	fix& operator*=(int n) { val *= n; return *this; }
	fix& operator/=(int n) { val /= n; return *this; }

	fix  operator+(double d) const { fix f(d); return (*this) + f; }
	fix  operator-(double d) const { fix f(d); return (*this) - f; }
	fix  operator*(double d) const { fix f(d); return (*this) * f; }
	fix  operator/(double d) const { fix f(d); return (*this) / f; }
	fix& operator+=(double d) { fix f(d); val += f.val; return *this; }
	fix& operator-=(double d) { fix f(d); val -= f.val; return *this; }
	fix& operator*=(double d) { fix f(d); val *= f.val; return *this; }
	fix& operator/=(double d) { fix f(d); val /= f.val; return *this; }

	// misc. functions:
	fix  truncate()    const { fix r; r.val = val & IntMask; return r; }
	fix  fraction()    const { fix r; r.val = val - truncate().val; return r; }
	fix  floor()       const { fix r; r.val = val & IntMask; return r; }
	fix  ceil()        const { fix r; r.val = (val + FractMask) & IntMask; return r; }
	fix  adjust_up()   const { fix r; r.val = val + FractMask; return r; }
	fix  adjust_down() const { fix r; r.val = val - FractMask; return r; }

	fix muldiv(const fix& num, const fix& den) const
	{
		long a = val, b = num.val, c = den.val;
		a = a * b / c; 
		fix r; r.val = a; return r;
	}

	// data:
	long val;
};



