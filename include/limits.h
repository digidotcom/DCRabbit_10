/*
	limits.h

	Copyright (c)2000-2010 Digi International Inc., All Rights Reserved

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
	Sizes of integral types.  Conforms to C90 standard.
*/

#ifndef __LIMITS_H
#define __LIMITS_H

#define CHAR_BIT                8
#define UCHAR_MAX             255
#define SCHAR_MAX             127
#define SCHAR_MIN   (-SCHAR_MAX-1)
#define CHAR_MAX        UCHAR_MAX
#define CHAR_MIN                0
#define MB_LEN_MAX              1
#define SHRT_MAX            32767
#define SHRT_MIN     (-SHRT_MAX-1)
#define USHRT_MAX           65535U
#define INT_MAX             32767
#define INT_MIN       (-INT_MAX-1)
#define UINT_MAX            65535U
#define LONG_MAX       2147483647L
#define LONG_MIN     (-LONG_MAX-1)
#define ULONG_MAX      4294967295LU

#endif

