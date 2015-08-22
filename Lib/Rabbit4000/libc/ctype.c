/*
	ctype.c

	Copyright (c) 2006-09 Digi International Inc., All Rights Reserved

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
	Functions defined in ctype.h.
*/

/*** BeginHeader */
#include <ctype.h>
/*** EndHeader */

/*** Beginheader _ctype */
#define _LOWER		0x01
#define _UPPER		0x02
#define _DIGIT		0x04
#define _XDIGIT	0x08
#define _PUNCT		0x10
#define _CNTRL		0x20
#define _PRINT		0x40
#define _SPACE		0x80

__root void _ctype();
/*** Endheader */

// Ctype Table
const char _ctype_table[] = {
	_CNTRL,									// control-@ (null)
	_CNTRL,									// control-A
	_CNTRL,									// control-B
	_CNTRL,									// control-C
	_CNTRL,									// control-D
	_CNTRL,									// control-E
	_CNTRL,									// control-F
	_CNTRL,									// control-G
	_CNTRL,									// control-H
	_CNTRL | _SPACE,						// control-I (\t, horizontal tab)
	_CNTRL | _SPACE,						// control-J (\n, newline)
	_CNTRL | _SPACE,						// control-K (\v, vertical tab)
	_CNTRL | _SPACE,						// control-L (\f, form feed)
	_CNTRL | _SPACE,						// control-M (\r, return)
	_CNTRL,									// control-N
	_CNTRL,									// control-O

	_CNTRL,									// control-P
	_CNTRL,									// control-Q
	_CNTRL,									// control-R
	_CNTRL,									// control-S
	_CNTRL,									// control-T
	_CNTRL,									// control-U
	_CNTRL,									// control-V
	_CNTRL,									// control-W
	_CNTRL,									// control-X
	_CNTRL,									// control-Y
	_CNTRL,									// control-Z
	_CNTRL,									// control-[ (escape)
	_CNTRL,									// control-\ (control backslash)
	_CNTRL,									// control-]
	_CNTRL,									// control-^
	_CNTRL,									// control-_

	_PRINT | _SPACE,						// space
	_PRINT | _PUNCT,						// !
	_PRINT | _PUNCT,						// "
	_PRINT | _PUNCT,						// #
	_PRINT | _PUNCT,						// $
	_PRINT | _PUNCT,						// %
	_PRINT | _PUNCT,						// &
	_PRINT | _PUNCT,						// '
	_PRINT | _PUNCT,						// (
	_PRINT | _PUNCT,						// )
	_PRINT | _PUNCT,						// *
	_PRINT | _PUNCT,						// +
	_PRINT | _PUNCT,						// ,
	_PRINT | _PUNCT,						// -
	_PRINT | _PUNCT,						// .
	_PRINT | _PUNCT,						// /

	_PRINT | _DIGIT | _XDIGIT,			// 0
	_PRINT | _DIGIT | _XDIGIT,			// 1
	_PRINT | _DIGIT | _XDIGIT,			// 2
	_PRINT | _DIGIT | _XDIGIT,			// 3
	_PRINT | _DIGIT | _XDIGIT,			// 4
	_PRINT | _DIGIT | _XDIGIT,			// 5
	_PRINT | _DIGIT | _XDIGIT,			// 6
	_PRINT | _DIGIT | _XDIGIT,			// 7
	_PRINT | _DIGIT | _XDIGIT,			// 8
	_PRINT | _DIGIT | _XDIGIT,			// 9
	_PRINT | _PUNCT,						// :
	_PRINT | _PUNCT,						// ;
	_PRINT | _PUNCT,						// <
	_PRINT | _PUNCT,						// =
	_PRINT | _PUNCT,						// >
	_PRINT | _PUNCT,						// ?

	_PRINT | _PUNCT,						// @
	_PRINT | _UPPER | _XDIGIT,			// A
	_PRINT | _UPPER | _XDIGIT,			// B
	_PRINT | _UPPER | _XDIGIT,			// C
	_PRINT | _UPPER | _XDIGIT,			// D
	_PRINT | _UPPER | _XDIGIT,			// E
	_PRINT | _UPPER | _XDIGIT,			// F
	_PRINT | _UPPER,						// G
	_PRINT | _UPPER,						// H
	_PRINT | _UPPER,						// I
	_PRINT | _UPPER,						// J
	_PRINT | _UPPER,						// K
	_PRINT | _UPPER,						// L
	_PRINT | _UPPER,						// M
	_PRINT | _UPPER,						// N
	_PRINT | _UPPER,						// O

	_PRINT | _UPPER,						// P
	_PRINT | _UPPER,						// Q
	_PRINT | _UPPER,						// R
	_PRINT | _UPPER,						// S
	_PRINT | _UPPER,						// T
	_PRINT | _UPPER,						// U
	_PRINT | _UPPER,						// V
	_PRINT | _UPPER,						// W
	_PRINT | _UPPER,						// X
	_PRINT | _UPPER,						// Y
	_PRINT | _UPPER,						// Z
	_PRINT | _PUNCT,						// [
	_PRINT | _PUNCT,						// \ (backslash)
	_PRINT | _PUNCT,						// ]
	_PRINT | _PUNCT,						// ^
	_PRINT | _PUNCT,						// _

	_PRINT | _PUNCT,						// `
	_PRINT | _LOWER | _XDIGIT,			// a
	_PRINT | _LOWER | _XDIGIT,			// b
	_PRINT | _LOWER | _XDIGIT,			// c
	_PRINT | _LOWER | _XDIGIT,			// d
	_PRINT | _LOWER | _XDIGIT,			// e
	_PRINT | _LOWER | _XDIGIT,			// f
	_PRINT | _LOWER,						// g
	_PRINT | _LOWER,						// h
	_PRINT | _LOWER,						// i
	_PRINT | _LOWER,						// j
	_PRINT | _LOWER,						// k
	_PRINT | _LOWER,						// l
	_PRINT | _LOWER,						// m
	_PRINT | _LOWER,						// n
	_PRINT | _LOWER,						// o

	_PRINT | _LOWER,						// p
	_PRINT | _LOWER,						// q
	_PRINT | _LOWER,						// r
	_PRINT | _LOWER,						// s
	_PRINT | _LOWER,						// t
	_PRINT | _LOWER,						// u
	_PRINT | _LOWER,						// v
	_PRINT | _LOWER,						// w
	_PRINT | _LOWER,						// x
	_PRINT | _LOWER,						// y
	_PRINT | _LOWER,						// z
	_PRINT | _PUNCT,						// {
	_PRINT | _PUNCT,						// |
	_PRINT | _PUNCT,						// }
	_PRINT | _PUNCT,						// ~
	_CNTRL,									// DEL

	0											// char 128 and above are non-ASCII
};

#asm __root __nodebug

; _ctype : Machine Callable Version of ctype Table Lookup
;
; INPUT  :
;          HL = Character Value
;          DE = Byte Mask
; OUTPUT :
;          HL = Non-Zero if Mask is True
;          Z flag set Accordingly ( Required for other Library Routines )

_ctype::
	; do unsigned compare of HL to 127 -- only 0 to 127 are valid ASCII
   cp   hl,127
   jr   gtu,.invalid    ; HL > 127 and zero flag is set, so return 0 for failure

   ld   bc,_ctype_table ; Index Character into Table
   add  hl,bc
   ld   a,(hl)          ; Read Mask Entry from Table
   and  e               ; Mask Against Request
   clr  hl              ; Return 0 for Failure
   ret  z
   inc  hl              ; Return 1 for Success
   ret
.invalid:
	clr  hl              ; return 0 and set z flag for failure
   xor  a               ; set z flag
	ret
#endasm

/*** Beginheader toupper */
/*** Endheader */

/* START FUNCTION DESCRIPTION ********************************************
toupper                                                          <ctype.h>

SYNTAX: int toupper(int c);

KEYWORDS: convert

DESCRIPTION:	Convert alphabetic character "c" to it's upper case equivalent.

RETURN VALUE:	Upper case alphabetic character.
END DESCRIPTION **********************************************************/

#asm __root __nodebug

toupper::
   push  hl				; protect character
   call  islower		; Convert only if islower(c)
   pop	hl				; restore character
   ret	z				; return unconverted if not lower (z flag set)
   res	5, l			; convert to upper case (clear bit 5)
   ret

#endasm

/*** Beginheader tolower */
/*** Endheader */

/* START FUNCTION DESCRIPTION ********************************************
tolower                                                          <ctype.h>

SYNTAX: int tolower(int c);

KEYWORDS: convert

DESCRIPTION:	Convert alphabetic character "c" to it's lower case equivalent.

RETURN VALUE:	Lower case alphabetic character.
END DESCRIPTION **********************************************************/

#asm __root __nodebug

tolower::
   push  hl				; protect character
   call  isupper		; Convert only if isupper(c)
   pop	hl				; restore character
   ret	z				; return unconverted if not upper (z flag set)
   set	5, l			; convert to lower case (set bit 5)
   ret

#endasm

/*** Beginheader islower,isupper,isdigit,isxdigit,ispunct,isspace,isprint */
/*** Endheader */

/* START FUNCTION DESCRIPTION ********************************************
islower                                                          <ctype.h>

SYNTAX: int islower(int c);

KEYWORDS:

DESCRIPTION:	Tests if "c" is a lower case character.
               ( a - z )

RETURN VALUE:   zero if not, non-zero if it is.

SEE ALSO:	isupper, isalpha, isdigit, isxdigit, isalnum,
				isspace, ispunct, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
isupper                                                          <ctype.h>

SYNTAX: int isupper(int c);

KEYWORDS:

DESCRIPTION:	Tests if "c" is an upper case character.
               ( A - Z )

RETURN VALUE:   zero if not, non-zero if it is.

SEE ALSO:	islower, isalpha, isdigit, isxdigit, isalnum,
				isspace, ispunct, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
isdigit                                                          <ctype.h>

SYNTAX: int isdigit(int c);

KEYWORDS:

DESCRIPTION:   Tests if "c" is a decimal digit.
               ( 0 - 9 )

RETURN VALUE:   zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isxdigit, isalnum,
				isspace, ispunct, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
isxdigit                                                         <ctype.h>

SYNTAX: int isxdigit(int c);

KEYWORDS:

DESCRIPTION:   Tests if "c" is a hexadecimal digit.
               ( 0 - 9, A - F, a - f)

RETURN VALUE:   zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isdigit, isalnum,
				isspace, ispunct, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
ispunct                                                          <ctype.h>

SYNTAX: int ispunct(int c);

KEYWORDS:

DESCRIPTION:   Tests if "c" is a punctuation character.

               !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~

               ( 33 <= c <= 47, 58 <= c <= 64, 91 <= c <= 96,
                and 123 <= c <= 126 )

RETURN VALUE:  zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isdigit, isxdigit, isalnum,
				isspace, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
isspace                                                          <ctype.h>

SYNTAX: int isspace(int c);

KEYWORDS:

DESCRIPTION:   Tests if "c" is a white space character.

               tab, return, newline, vertical tab, form feed, and space

               ( 9 <= c <= 13 and c == 32 )

RETURN VALUE:  zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isdigit, isxdigit, isalnum,
				ispunct, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
isprint                                                          <ctype.h>

SYNTAX: int isprint(int c);

KEYWORDS:

DESCRIPTION:   Tests if "c" is a printing character.
               ( 32 <= c <= 126 )

RETURN VALUE:	zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isdigit, isxdigit, isalnum,
				isspace, ispunct, isgraph, iscntrl
END DESCRIPTION **********************************************************/

#asm __root __nodebug

islower::
   ld e, _LOWER     ; Character Mask
   jp _ctype

isupper::
   ld e, _UPPER     ; Character Mask
   jp _ctype

isdigit::
   ld e, _DIGIT     ; Character Mask
   jp _ctype

isxdigit::
   ld e, _XDIGIT    ; Character Mask
   jp _ctype

ispunct::
   ld e, _PUNCT     ; Character Mask
   jp _ctype

isprint::
   ld e, _PRINT     ; Character Mask
   jp _ctype

isspace::
   ld e, _SPACE     ; Character Mask
   jp _ctype

#endasm

/*** Beginheader isalpha,isalnum,isgraph,iscntrl */
/*** Endheader */

/* START FUNCTION DESCRIPTION ********************************************
isalpha                                                          <ctype.h>

SYNTAX: int isalpha(int c);

KEYWORDS:

DESCRIPTION:    Tests if "c" is an alphabetic character.
                (A to Z, or a to z)

RETURN VALUE:   zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isdigit, isxdigit, isalnum,
				isspace, ispunct, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
isalnum                                                          <ctype.h>

SYNTAX: int isalnum(int c);

KEYWORDS:

DESCRIPTION:	Tests if "c" is an alphabetic or numeric character.
               (A to Z, a to z and 0 to 9)

RETURN VALUE:	zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isdigit, isxdigit,
				isspace, ispunct, isprint, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
isgraph                                                          <ctype.h>

SYNTAX: int isgraph(int c);

KEYWORDS:

DESCRIPTION:	Tests if "c" is any printing character other than a space.
               ( 33 <= c <= 126 )

RETURN VALUE:	zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isdigit, isxdigit, isalnum,
				isspace, ispunct, isgraph, iscntrl
END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
iscntrl                                                          <ctype.h>

SYNTAX: int iscntrl(int c);

KEYWORDS:

DESCRIPTION:  Tests if "c" is a control character.
              ( 0 <= c <= 31 or c == 127 )

RETURN VALUE:	zero if not, non-zero if it is.

SEE ALSO:	islower, isupper, isalpha, isdigit, isxdigit, isalnum,
				isspace, ispunct, isprint, isgraph
END DESCRIPTION **********************************************************/

#asm __root __nodebug

isalpha::
   ld e, _LOWER | _UPPER								; Character Mask
   jp _ctype

isalnum::
   ld e, _LOWER | _UPPER | _DIGIT					; Character Mask
   jp _ctype

isgraph::
   ld e, _LOWER | _UPPER | _DIGIT | _PUNCT		; Character Mask
   jp _ctype

iscntrl::
   ld e, _CNTRL											; Character Mask
   jp _ctype

#endasm



