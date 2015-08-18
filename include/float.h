/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	C90 - 5.2.4.2.2 Characteristics of floating types
*/

#define FLT_RADIX                        2
#define FLT_ROUNDS                       0		// toward zero

#define FLT_MANT_DIG                    24
#define FLT_DIG                          6
#define FLT_EPSILON        1.19209290E-07F
#define FLT_MIN_EXP                   -125
#define FLT_MIN            1.17549435E-38F
#define FLT_MIN_10_EXP                 -37
#define FLT_MAX_EXP                   +128
#define FLT_MAX            3.40282347E+38F
#define FLT_MAX_10_EXP                 +38

// Dynamic C's "double" type is the same as "float".
#define DBL_MANT_DIG			FLT_MANT_DIG
#define DBL_DIG				FLT_DIG
#define DBL_EPSILON			FLT_EPSILON
#define DBL_MIN_EXP			FLT_MIN_EXP
#define DBL_MIN				FLT_MIN
#define DBL_MIN_10_EXP		FLT_MIN_10_EXP
#define DBL_MAX_EXP			FLT_MAX_EXP
#define DBL_MAX				FLT_MAX
#define DBL_MAX_10_EXP		FLT_MAX_10_EXP

