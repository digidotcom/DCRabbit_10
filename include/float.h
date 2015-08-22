/*
	float.h

	Copyright (c) 2010 Digi International Inc., All Rights Reserved

	This software contains proprietary and confidential information of Digi
	International Inc.  By accepting transfer of this copy, Recipient agrees
	to retain this software in confidence, to prevent disclosure to others,
	and to make no use of this software other than that for which it was
	delivered.  This is a published copyrighted work of Digi International
	Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
	prohibited.

	Restricted Rights Legend

	Use, duplication, or disclosure by the Government is subject to
	restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
	Technical Data and Computer Software clause at DFARS 252.227-7031 or
	subparagraphs (c)(1) and (2) of the Commercial Computer Software -
	Restricted Rights at 48 CFR 52.227-19, as applicable.

	Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343

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

