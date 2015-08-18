/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/**
	@addtogroup zcl_64
	@{
	@file zigbee/zcl64.h
	Macros for working with 64-bit integers via the zcl64_t datatype.  Makes
	use of the JSInt64 type and support functions included in the platform
	support code (jslong.h, jslong.c).

	On some platforms (Win32), zcl64_t is simply a uint64_t.  Most embedded
	platforms represent a zcl64_t value as a structure.  Because of that
	difference, you can't just write "c = a + b" or
	"a.lo = ~a.lo, a.hi = ~a.hi" -- neither statement is portable to the
	other platform type.

	Therefore, it is necessary to use the macro functions in this file to
	manipulate zcl64_t variables.

	@todo add a ZCL64_SPLIT macro to split zcl64_t into high and low halves
*/

#ifndef ZIGBEE_ZCL64_H
#define ZIGBEE_ZCL64_H

#include "xbee/platform.h"

/// 64-bit integer in host-byte-order
/// Use for 56-bit values as well -- ZCL layer will make sure top byte is
/// sign-extended.
/// @todo Actually code up support for 56-bit values like we did for 24 bit.
typedef JSUint64 zcl64_t;

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_INIT                              <zcl64.h>

MACRO SYNTAX:
     ZCL64_INIT( hi,  lo)

DESCRIPTION:
     Initialize a zcl64_t variable with two literal 32-bit values.

     Note that this macro is only valid as an initializer in a variable
     declaration.  Use ZCL64_LOAD in general program statements.


PARAMETER1:  hi - upper 32 bits
PARAMETER2:  lo - lower 32 bits


RETURNS:  an initializer for a zcl64_t variable



**************************************************************************/
#define ZCL64_INIT(hi, lo)				JSLL_INIT(hi, lo)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_LOAD                              <zcl64.h>

MACRO SYNTAX:
     ZCL64_LOAD( r,  hi32,  lo32)

DESCRIPTION:
     Load a zcl64_t variable with two 32-bit values (high and low).


PARAMETER1:  r - zcl64_t variable to assign (hi << 32 + lo) to
PARAMETER2:  hi32 - upper 32 bits to load into \param r
PARAMETER3:  lo32 - lower 32 bits to load into \param r



**************************************************************************/
#ifdef XBEE_NATIVE_64BIT
	#define ZCL64_LOAD(r, hi32, lo32)	r = (((uint64_t)(hi32) << 32) + lo32)
#else
	#define ZCL64_LOAD(r, hi32, lo32)	((r).hi = (hi32), (r).lo = (lo32))
#endif

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_IS_ZERO                           <zcl64.h>

MACRO SYNTAX:
     ZCL64_IS_ZERO( a)

DESCRIPTION:
     Compare a zcl64_t variable to zero.


PARAMETER1:  a - zcl64_t variable


RETURNS:  (a == 0)



**************************************************************************/
#define ZCL64_IS_ZERO(a)				JSLL_IS_ZERO(a)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_EQ                                <zcl64.h>

MACRO SYNTAX:
     ZCL64_EQ( a,  b)

DESCRIPTION:
     Compare two zcl64_t variables for equality.


PARAMETER1:  a - zcl64_t variable
PARAMETER2:  b - zcl64_t variable


RETURNS:  (a == b)



**************************************************************************/
#define ZCL64_EQ(a, b)					JSLL_EQ(a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_NE                                <zcl64.h>

MACRO SYNTAX:
     ZCL64_NE( a,  b)

DESCRIPTION:
     Compare two zcl64_t variables for inequality.


PARAMETER1:  a - zcl64_t variable
PARAMETER2:  b - zcl64_t variable


RETURNS:  (a != b)



**************************************************************************/
#define ZCL64_NE(a, b)					JSLL_NE(a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_GE_ZERO                           <zcl64.h>

MACRO SYNTAX:
     ZCL64_GE_ZERO( a)

DESCRIPTION:
     Compare a signed zcl64_t variable to 0.


PARAMETER1:  a - signed zcl64_t variable


RETURNS:  (a >= 0)



**************************************************************************/
#define ZCL64_GE_ZERO(a)				JSLL_GE_ZERO(a)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_LT                                <zcl64.h>

MACRO SYNTAX:
     ZCL64_LT( a,  b)

DESCRIPTION:
     Compare two zcl64_t variables (signed less-than comparison)

PARAMETER1:  a - signed zcl64_t variable
PARAMETER2:  b - signed zcl64_t variable


RETURNS:  (a < b)



**************************************************************************/
#define ZCL64_LT(a, b)					JSLL_REAL_CMP(a, <, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_LTU                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_LTU( a,  b)

DESCRIPTION:
     Compare two zcl64_t variables (unsigned less-than comparison)

PARAMETER1:  a - unsigned zcl64_t variable
PARAMETER2:  b - unsigned zcl64_t variable


RETURNS:  (a < b)



**************************************************************************/
#define ZCL64_LTU(a, b)					JSLL_REAL_UCMP(a, <, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_AND                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_AND( r,  a,  b)

DESCRIPTION:
     Perform a bitwise AND of two zcl64_t variables.

PARAMETER1:  r - zcl64_t variable to assign (a & b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_AND(r, a, b)				JSLL_AND(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_OR                                <zcl64.h>

MACRO SYNTAX:
     ZCL64_OR( r,  a,  b)

DESCRIPTION:
     Perform a bitwise OR of two zcl64_t variables.

PARAMETER1:  r - zcl64_t variable to assign (a | b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_OR(r, a, b)				JSLL_OR(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_XOR                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_XOR( r,  a,  b)

DESCRIPTION:
     Perform a bitwise XOR of two zcl64_t variables.

PARAMETER1:  r - zcl64_t variable to assign (a ^ b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_XOR(r, a, b)				JSLL_XOR(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_NOT                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_NOT( r,  a)

DESCRIPTION:
     Perform a bitwise NOT of a zcl64_t variable.

PARAMETER1:  r - zcl64_t variable to assign (~a) to
PARAMETER2:  a - zcl64_t variable



**************************************************************************/
#define ZCL64_NOT(r, a)					JSLL_NOT(r, a)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_NEG                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_NEG( r,  a)

DESCRIPTION:
     Negate a zcl64_t variable.

PARAMETER1:  r - zcl64_t variable to assign (-a) to
PARAMETER2:  a - zcl64_t variable



**************************************************************************/
#define ZCL64_NEG(r, a)					JSLL_NEG(r, a)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_ADD                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_ADD( r,  a,  b)

DESCRIPTION:
     Add two zcl64_t variables.

PARAMETER1:  r - zcl64_t variable to assign (a + b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_ADD(r, a, b)				JSLL_ADD(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_SUB                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_SUB( r,  a,  b)

DESCRIPTION:
     Subtract two zcl64_t variables.

PARAMETER1:  r - zcl64_t variable to assign (a - b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_SUB(r, a, b)				JSLL_SUB(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_MUL                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_MUL( r,  a,  b)

DESCRIPTION:
     Multiply two zcl64_t variables.

PARAMETER1:  r - zcl64_t variable to assign (a * b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_MUL(r, a, b)				JSLL_MUL(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_MUL32                             <zcl64.h>

MACRO SYNTAX:
     ZCL64_MUL32( r,  a,  b)

DESCRIPTION:
     Multiply two 32-bit variables (int32_t or uint32_t) and store the result
     in a zcl64_t variable.

PARAMETER1:  r - zcl64_t variable to assign (a * b) to
PARAMETER2:  a - uint32_t variable
PARAMETER3:  b - uint32_t variable



**************************************************************************/
#define ZCL64_MUL32(r, a, b)			JSLL_MUL32(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_UDIVMOD                           <zcl64.h>

MACRO SYNTAX:
     ZCL64_UDIVMOD( qp,  rp,  a,  b)

DESCRIPTION:
     Divide an unsigned zcl64_t variable by another unsigned zcl64_t variable
     and store the 64-bit quotient and remainder.

PARAMETER1:  qp - NULL to ignore the quotient, or address of a zcl64_t
              variable to assign (a / b) to
PARAMETER2:  rp - NULL to ignore the remainder, or address of a zcl64_t
              variable to assign (a % b) to
PARAMETER3:  a - zcl64_t variable
PARAMETER4:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_UDIVMOD(qp, rp, a, b)	JSLL_UDIVMOD(qp, rp, a, b)

// Is there a need for a signed divmod, based on JSLL_DIV and JSLL_MOD?
// Would need to store two negate flags, one for quotient and one for
// remainder.  Remainder only negative if a is negative, quotient is negative
// if sign of a and b differ

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_DIV                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_DIV( r,  a,  b)

DESCRIPTION:
     Perform signed division of two zcl64_t variables and store the quotient.

PARAMETER1:  r - signed zcl64_t variable to assign (a / b) to
PARAMETER2:  a - signed zcl64_t variable
PARAMETER3:  b - signed zcl64_t variable



**************************************************************************/
#define ZCL64_DIV(r, a, b)				JSLL_DIV(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_MOD                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_MOD( r,  a,  b)

DESCRIPTION:
     Perform signed division of two zcl64_t variables and store the remainder.

PARAMETER1:  r - zcl64_t variable to assign (a + b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - zcl64_t variable



**************************************************************************/
#define ZCL64_MOD(r, a, b)				JSLL_MOD(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_ASL                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_ASL( r,  a,  b)

DESCRIPTION:
     Arithmetic Shift Left of a zcl64_t variable.  Shifts bits of a left
     by \b positions, inserting zeros on the right.  Equivalent to ZCL64_LSL.

PARAMETER1:  r - zcl64_t variable to assign (a << b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - integral value from 0 to 63



**************************************************************************/
#define ZCL64_ASL(r, a, b)					JSLL_SHL(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_LSL                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_LSL( r,  a,  b)

DESCRIPTION:
     Logical Shift Left of a zcl64_t variable.  Shifts bits of a left
     by \b positions, inserting zeros on the right.  Equivalent to ZCL64_ASL.

PARAMETER1:  r - zcl64_t variable to assign (a << b) to
PARAMETER2:  a - zcl64_t variable
PARAMETER3:  b - integral value from 0 to 63



**************************************************************************/
#define ZCL64_LSL(r, a, b)					ZCL64_SLA(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_ASR                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_ASR( r,  a,  b)

DESCRIPTION:
     Arithmetic Shift Right of a signed zcl64_t variable.  Shifts bits of
     a right by \b positions, extending the sign bit on the left.

PARAMETER1:  r - signed zcl64_t variable to assign (a >> b) to
PARAMETER2:  a - signed zcl64_t variable
PARAMETER3:  b - integral value from 0 to 63



**************************************************************************/
#define ZCL64_ASR(r, a, b)					JSLL_SHR(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_LSR                               <zcl64.h>

MACRO SYNTAX:
     ZCL64_LSR( r,  a,  b)

DESCRIPTION:
     Logical Shift Right of an unsigned zcl64_t variable.  Shifts bits of
     a right by \b positions, inserting zeros on the left.

PARAMETER1:  r - unsigned zcl64_t variable to assign (a >> b) to
PARAMETER2:  a - unsigned zcl64_t variable
PARAMETER3:  b - integral value from 0 to 63



**************************************************************************/
#define ZCL64_LSR(r, a, b)					JSLL_USHR(r, a, b)

/* a is an JSInt32, b is JSInt32, r is JSInt64 */
//#define JSLL_ISHL(r, a, b)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_TO_INT32                          <zcl64.h>

MACRO SYNTAX:
     ZCL64_TO_INT32( i32,  i64)

DESCRIPTION:
     Cast a signed zcl64_t variable down to a signed 32-bit integer.

PARAMETER1:  i32 - int32_t variable to cast \c i64 into
PARAMETER2:  i64 - signed zcl64_t variable


              ZCL64_FROM_UINT32, ZCL64_FROM_FLOAT, ZCL64_FROM_DOUBLE

**************************************************************************/
#define ZCL64_TO_INT32(i32, i64)			JSLL_L2I(i32, i64)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_TO_UINT32                         <zcl64.h>

MACRO SYNTAX:
     ZCL64_TO_UINT32( u32,  u64)

DESCRIPTION:
     Cast an unsigned zcl64_t variable down to an unsigned 32-bit integer.

PARAMETER1:  u32 - uint32_t variable to cast \c u64 into
PARAMETER2:  u64 - unsigned zcl64_t variable


              ZCL64_FROM_UINT32, ZCL64_FROM_FLOAT, ZCL64_FROM_DOUBLE

**************************************************************************/
#define ZCL64_TO_UINT32(u32, u64)		JSLL_L2UI(u32, u64)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_LOW32                             <zcl64.h>

MACRO SYNTAX:
     ZCL64_LOW32( u64)

DESCRIPTION:
     The lower-32 bits of a ZCL64 value.


PARAMETER1:  u64 - unsigned zcl64_t variable

**************************************************************************/
#ifdef XBEE_NATIVE_64BIT
	#define ZCL64_LOW32(u64)				((uint32_t)(u64))
#else
	#define ZCL64_LOW32(u64)				((u64).lo)
#endif

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_HIGH32                            <zcl64.h>

MACRO SYNTAX:
     ZCL64_HIGH32( u64)

DESCRIPTION:
     The upper-32 bits of a ZCL64 value.


PARAMETER1:  u64 - unsigned zcl64_t variable

**************************************************************************/
#ifdef XBEE_NATIVE_64BIT
	#define ZCL64_HIGH32(u64)				((uint32_t)(u64 >> 32))
#else
	#define ZCL64_HIGH32(u64)				((u64).hi)
#endif

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_TO_FLOAT                          <zcl64.h>

MACRO SYNTAX:
     ZCL64_TO_FLOAT( f,  i64)

DESCRIPTION:
     Cast a signed zcl64_t variable to a float.

PARAMETER1:  f - float variable to cast \c i64 into
PARAMETER2:  i64 - signed zcl64_t variable


              ZCL64_FROM_UINT32, ZCL64_FROM_FLOAT, ZCL64_FROM_DOUBLE,

**************************************************************************/
#define ZCL64_TO_FLOAT(f, i64)			JSLL_L2F(f, i64)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_TO_DOUBLE                         <zcl64.h>

MACRO SYNTAX:
     ZCL64_TO_DOUBLE( d,  i64)

DESCRIPTION:
     Cast a signed zcl64_t variable to a double.

PARAMETER1:  d - double variable to cast \c i64 into
PARAMETER2:  i64 - signed zcl64_t variable


              ZCL64_FROM_UINT32, ZCL64_FROM_FLOAT, ZCL64_FROM_DOUBLE

**************************************************************************/
#define ZCL64_TO_DOUBLE(d, i64)			JSLL_L2D(d, i64)


/* START FUNCTION DESCRIPTION ********************************************
ZCL64_FROM_INT32                        <zcl64.h>

MACRO SYNTAX:
     ZCL64_FROM_INT32( i64,  i32)

DESCRIPTION:
     Cast a signed 32-bit integer up to a zcl64_t variable.

PARAMETER1:  i64 - signed zcl64_t variable to cast \c i32 into
PARAMETER2:  i32 - int32_t variable


              ZCL64_FROM_UINT32, ZCL64_FROM_FLOAT, ZCL64_FROM_DOUBLE

**************************************************************************/
#define ZCL64_FROM_INT32(i64, i32)		JSLL_I2L(i64, i32)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_FROM_UINT32                       <zcl64.h>

MACRO SYNTAX:
     ZCL64_FROM_UINT32( u64,  u32)

DESCRIPTION:
     Cast an unsigned 32-bit integer up to a zcl64_t variable.

PARAMETER1:  u64 - unsigned zcl64_t variable to cast \c u32 into
PARAMETER2:  u32 - uint32_t variable


              ZCL64_FROM_INT32, ZCL64_FROM_FLOAT, ZCL64_FROM_DOUBLE

**************************************************************************/
#define ZCL64_FROM_UINT32(u64, u32)		JSLL_UI2L(u64, u32)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_FROM_FLOAT                        <zcl64.h>

MACRO SYNTAX:
     ZCL64_FROM_FLOAT( i64,  f)

DESCRIPTION:
     Cast a double into a signed zcl64_t variable.

PARAMETER1:  i64 - signed zcl64_t variable to cast \c f into
PARAMETER2:  f - float variable/value


              ZCL64_FROM_INT32, ZCL64_FROM_UINT32, ZCL64_FROM_DOUBLE

**************************************************************************/
#define ZCL64_FROM_FLOAT(i64, f)			JSLL_F2L(i64, f)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_FROM_DOUBLE                       <zcl64.h>

MACRO SYNTAX:
     ZCL64_FROM_DOUBLE( i64,  d)

DESCRIPTION:
     Cast a float into a signed zcl64_t variable.

PARAMETER1:  i64 - signed zcl64_t variable to cast \c d into
PARAMETER2:  d - double variable/value


              ZCL64_FROM_INT32, ZCL64_FROM_UINT32, ZCL64_FROM_FLOAT

**************************************************************************/
#define ZCL64_FROM_DOUBLE(i64, d)		JSLL_D2L(i64, d)


/* START FUNCTION DESCRIPTION ********************************************
ZCL64_TO_HEXSTR                         <zcl64.h>

MACRO SYNTAX:
     ZCL64_TO_HEXSTR( buffer,  var)

DESCRIPTION:
     Convert a zcl64_t variable to a 16-character printable hexadecimal
     string.

PARAMETER1:  buffer - 17-character buffer to hold hexadecimal string
PARAMETER2:  var - zcl64_t variable to stringify


RETURNS:  16       - this function always returns 16, the number of
                     characters written to buffer (in addition to
                     the null terminator)



**************************************************************************/
#define ZCL64_TO_HEXSTR(buffer, var)	JSLL_HEXSTR(buffer, var)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_TO_DECSTR                         <zcl64.h>

MACRO SYNTAX:
     ZCL64_TO_DECSTR( buffer,  var)

DESCRIPTION:
     Convert a signed zcl64_t variable to a 20-character printable decimal
     string.

PARAMETER1:  buffer - 21-character buffer to hold hexadecimal string
PARAMETER2:  var - signed zcl64_t variable to stringify


RETURNS:  number of characters written to \c buffer (1 to 20), in addition
                     to the null terminator



**************************************************************************/
#define ZCL64_TO_DECSTR(buffer, var)	JSLL_DECSTR(buffer, var)

/* START FUNCTION DESCRIPTION ********************************************
ZCL64_TO_UDECSTR                        <zcl64.h>

MACRO SYNTAX:
     ZCL64_TO_UDECSTR( buffer,  var)

DESCRIPTION:
     Convert an unsigned zcl64_t variable to a 20-character printable decimal
     string.

PARAMETER1:  buffer - 21-character buffer to hold hexadecimal string
PARAMETER2:  var - unsigned zcl64_t variable to stringify


RETURNS:  number of characters written to \c buffer (1 to 20), in addition
                     to the null terminator



**************************************************************************/
#define ZCL64_TO_UDECSTR(buffer, var)	JSLL_UDECSTR(buffer, var)

#endif
