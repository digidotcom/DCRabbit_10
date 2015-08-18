/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

