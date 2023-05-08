/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*** BeginHeader */
#include <string.h>

#ifdef STRING_DEBUG
	#define _string_debug	__debug
#else
	#define _string_debug	__nodebug
#endif
/*** EndHeader */

/* START FUNCTION DESCRIPTION ********************************************
memcpy                                                          <string.h>

NEAR SYNTAX: void *_n_memcpy(void *dst, const void *src, size_t n);
FAR SYNTAX: void far *_f_memcpy(void far *dst, const void far *src, size32_t n);

NOTE: By Default, memcpy and memmove are defined to _n_memcpy and
	_n_memmove, respectively.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

KEYWORDS: memory

DESCRIPTION:	Copies "n" characters from memory pointed to by "src" to
memory pointed to by "dst".  Overlap is handled correctly.

RETURN VALUE:	Returns "dst".
END DESCRIPTION **********************************************************/

/*** Beginheader _f_memcpy, _f_memmove */
/*** Endheader */
#asm __nodebug __root
   align odd  ; Aligns most of instructions on even boundary without the
              ; potential nop impacting performnce
_f_memcpy::
_f_memmove::
#if _RAB6K
	ld		pw, (sp+10)	; n
	tstnull pw          ; If someone is moving 0xFFFF0000 bytes we are out of our depth...
	ret	z

	ld		pz, px		; save for ret
	ld		py, (sp+6)	; src

	ld		jkhl, px	; dst
	cp		jkhl, py
	jr		z, .nomov	; if dst == src, we're done
	jr		gt, .movbak	; if dst > src, move from back to front

	ld		jkhl, pw	; n
	ld		bc, hl		; bc = n (LSW)
	ex		jk,hl			; hl = 64k count
	test	bc
	jr		nz,.contf
	dec	hl
.contf:
	pldir
	test	hl
	jr		z,.nomov
	dec	hl
	jr		.contf

    align even  ; Make next 8 instructions align for better performnce
.movbak:
	ld		bcde, pw	; n
    dec 	pw          ; n - 1
	ld		jkhl,px
	add	    jkhl,pw
	ld		px,jkhl
	ld		jkhl,py
	add	    jkhl,pw
	ld		py,jkhl
	ex		jkhl,bcde
	ld		bc, hl		; bc = n (LSW)
	ex		jk,hl			; hl = 64k count
	test	bc
	jr		nz,.contb
	dec	hl
.contb:
	plddr
	test	hl
	jr		z,.nomov
	dec	hl
	jr		.contb
.nomov:
	ld		px, pz		; reload px with dst for ret
	ret
#else
	ld		jkhl, (sp+10)	; n
	test	jkhl
	ret	z
	ld		pz, px		; save for ret
	ld		px, (sp+6)	; src

	ld		jkhl, pz		; dst
	ld		py, pz		; py = dst
	ld		bcde, px		; src
	cp		jkhl, bcde
	jr		z, .nomov	; if dst == src, we're done
	jr		gt, .movbak	; if dst > src, move from back to front

	ld		jkhl, (sp+10)	; n
	ld		bc, hl		; bc = n (LSW)
	ex		jk,hl			; hl = 64k count
	test	bc
	jr		nz,.contf
	dec	hl
.contf:
#if _BOARD_TYPE_ == 0x2700
	call copy_func
#else
	copy
#endif
	test	hl
	jr		z,.nomov
	dec	hl
	jr		.contf

.movbak:
	ld		bcde, (sp+10)	; n
	ld		jkhl,px
	add	jkhl,bcde
	ld		px,jkhl
	ld		px,px-1
	ld		jkhl,py
	add	jkhl,bcde
	ld		py,jkhl
	ld		py,py-1
	ex		jkhl,bcde

	ld		bc, hl		; bc = n (LSW)
	ex		jk,hl			; hl = 64k count
	test	bc
	jr		nz,.contb
	dec	hl
.contb:
#if _BOARD_TYPE_ == 0x2700
	call copyr_func
#else
	copyr
#endif
	test	hl
	jr		z,.nomov
	dec	hl
	jr		.contb
.nomov:
	ld		px, pz		; reload px with dst for ret
	ret
#endif
#endasm

/*** Beginheader _n_memcpy, _n_memmove */
/*** Endheader */
#asm __nodebug __root
_n_memcpy::
_n_memmove::
	ld		bcde, (sp+4)	; load second param to DE, third param to BC
   test	bc					; Check for Zero Count
   ret	z					; Quit if Zero

	; HL = dest, DE = src

	cp		hl, de
	ret	eq					; nothing to copy if equal, retval (HL) is set to dest

   jr		nc, .movbak		; if source lower (HL >= DE), move backwards

	ex		de, hl			; HL = src, DE = dest
   ldir
   ld		hl, (sp+2)		; return dest address
   ret						; Done

.movbak:						; add count to source & dest, then do a reverse copy
   add   hl, bc
   dec   hl
   ex		de, hl			; HL = src, DE = dest
   add   hl, bc
   dec   hl
   lddr						; copy backwards
   ld		hl, (sp+2)		; return dest address
   ret						; Done
#endasm

/* START FUNCTION DESCRIPTION ********************************************
strcpy                                                          <string.h>

NEAR SYNTAX: char * _n_strcpy(char * dst, const char * src);
FAR SYNTAX: char far * _f_strcpy(char far * dst, const char far * src);

NOTE: By Default, strcpy is defined to _n_strcpy.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

DESCRIPTION: Copies string "src" to string "dst", including the null
             terminator.

PARAMETER1: Pointer to location to receive string.
PARAMETER2: Pointer to location to supply string.

RETURN VALUE: Pointer to "dst".

KEYWORDS: string, copy
dma.lib
END DESCRIPTION **********************************************************/

/*** Beginheader _f_strcpy */
/*** Endheader */
__nodebug
char far *_f_strcpy(char far *dst, const char far *src)
{
	#asm
	ld		pz, px					; PZ = PX = dst (saved for return value)
	ld		py, (sp+@sp+src)		; PY = src
    clr     hl
	.cpy:
	ld		a, (py+hl)				; *dst = *src
#if _RAB6K
    pldi              				; *dst++ = *src++
#else
	ld		(px+hl), a				;  (strcpy includes nul terminator)
	ld		py, py+1					; dst++
	ld		px, px+1					; src++
#endif
	or		a							; end of src string?
	jr		nz, .cpy					; if no (Zero flag reset), go copy another char

	ld		px, pz					; PX = dst (restore the return value)
#endasm
	// char far * result is returned in PX
}


/*** Beginheader _n_strcpy */
/*** Endheader */
__nodebug
char *_n_strcpy(char *dst, const char *src)
{
	#asm
	ld		de, hl					; DE = dst
	ld		hl, (sp+@sp+src)		; HL = src
	xor	a							; A = 0
	.cpy:
	cp		(hl)						; end of src string?
	ldi								; *dst++ = *src++ (strcpy includes nul terminator)
	jr		nz, .cpy					; if no (Zero flag reset), go copy another byte

	ld		hl, (sp+@sp+dst)		; HL = dst (restore the return value)
	#endasm
	// char * result is returned in HL
}


/* START FUNCTION DESCRIPTION ********************************************
strncpy                                                         <string.h>

NEAR SYNTAX: char *_n_strncpy(char *dst, const char *src, size_t n);
FAR SYNTAX: char far *_f_strncpy(char far *dst, const char far *src, size_t n);

NOTE: By Default, strncpy is defined to _n_strncpy.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

DESCRIPTION: Copies "n" characters to dst, padding dst with null characters
if src null terminates before "n" characters are copied, or truncating if src
is "n" or more characters, leaving dst without a null terminator.

PARAMETER1: Pointer to location to receive string.
PARAMETER2: Pointer to location to supply string.
PARAMETER3: Maximum number of bytes to copy.
            if equal to zero, this function has no effect

RETURN VALUE: Pointer to "dst".

KEYWORDS: string, copy

END DESCRIPTION **********************************************************/


/*** Beginheader _f_strncpy */
/*** Endheader */
#asm __nodebug __root
_f_strncpy:: ; px = dst
	ld		hl, (sp+10) ; n
	test	hl
	ret	z
	ld		pz, px ; save for return
	ld		py, px
	ld		px, (sp+6) ; src
.cpy:
	dec	hl
	ld		a, (px)
	ld		bc, 1
#if _BOARD_TYPE_ == 0x2700
	call copy_func             ; *dst++ = *src++, bc--
#else
	copy
#endif
	or		a
	jr		z, .nullfound
	test	hl
	jr		nz, .cpy
	ld		px, pz ; restore dst
	ret
.nullfound:
	test	hl
   jr		z, .done
   ex		bc, hl
	ld		px, py - 1
#if _BOARD_TYPE_ == 0x2700
	call copy_func
#else
	copy
#endif
.done:
	ld		px, pz ; restore dst
	ret
#endasm

/*** Beginheader _n_strncpy */
/*** Endheader */
__root _string_debug
char * _n_strncpy(char * dst, const char * src, size_t n)
{
#asm
	ld		de, hl			; dest to de
	ld		hl, (sp+@SP+n)					; n to hl
	test	hl
	jr		z, .done	;jump if n = 0
	ld		bc, hl
	ld		hl, (sp+@SP+src)				; load src to hl
	xor	a			;clear a
.copy:			;a = 0, dst = de, src = hl, n = bc
	cp		(hl)		;test for null terminator
.pad:
	ldi				;*dst++ = *src++, n--
	jp		lz,.done	;exit when bc (n) = 0
	jr		nz,.copy	;resume loop if not null terminator
	dec	hl			;point src back to null terminator
	jr		.pad		;pad remainder of dst with nulls
.done:
	ld		hl, (sp+@SP+dst)				; load dst to hl for return value
#endasm
}


/* START FUNCTION DESCRIPTION ********************************************
strcat                                                          <string.h>

NEAR SYNTAX: char * _n_strcat(char * dst, const char * src);
FAR SYNTAX: char far * _f_strcat(char far * dst, const char far * src);

NOTE: By Default, strcat is defined to _n_strcat.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

DESCRIPTION: Concatenate string "src" to the end of "dst".

PARAMETER1: Pointer to location to receive string.
PARAMETER2: Pointer to location to supply string.

RETURN VALUE:	Pointer to "dst".

KEYWORDS: string, copy
END DESCRIPTION **********************************************************/

/*** Beginheader _f_strcat */
/*** Endheader */
_string_debug __useix
char __far * _f_strcat(char __far * dst, const char __far * src)
{
	_f_strcpy(_f_strchr(dst, '\0'), src);
	return dst;
}

/*** Beginheader _n_strcat */
/*** Endheader */
_string_debug __useix
char * _n_strcat(char * dst, const char * src)
{
	_n_strcpy(_n_strchr(dst, '\0'), src);
	return dst;
}

/* START FUNCTION DESCRIPTION ********************************************
strncat                                                         <string.h>

NEAR SYNTAX: char *_n_strncat(char *dst, const char *src, size_t n);
FAR SYNTAX: char far *_f_strncat(char far *dst, const char far *src, size_t n);

NOTE: By Default, strncat is defined to _n_strncat.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

DESCRIPTION: Appends "src" to "dst", up to and including the null terminator
or until "n" characters are transferred, followed by a null terminator.

PARAMETER1: Pointer to location to receive string.
PARAMETER2: Pointer to location to supply string.
PARAMETER3: Maximum number of bytes to copy.
            if equal to zero, this function has no effect

RETURN VALUE: Pointer to "dst".

KEYWORDS: string, copy

END DESCRIPTION **********************************************************/

/*** Beginheader _f_strncat */
/*** Endheader */
#asm __nodebug __root
_f_strncat::		; px = dst
	ld		hl, (sp+10)	; n
	test	hl
	jr		z, .done
	ld		pz, px	; save return
.search:
	ld		a, (px)
	or		a
	jr		z, .begin
	ld		px, px + 1
	jr		.search
.begin:
	ld		py, px
	ld		px, (sp+6)	; src
.cpy:
	ld		a, (px)
	or		a
	jr		z, .done
	ld		bc, 1
#if _BOARD_TYPE_ == 0x2700
	call copy_func				; *py++ = *px++
#else
	copy
#endif
	dec	hl
	test	hl
	jr		nz, .cpy
.done:
	xor	a
	ld		(py), a	; null terminator
	ld		px, pz	; return dst
	ret
#endasm

/*** Beginheader _n_strncat */
/*** Endheader */
#asm __nodebug __root
_n_strncat::
	ex		de, hl
	ld		hl, (sp+6)	; hl = n
	bool	hl
	jr		z, .done		; if n == 0, done
	ex		de, hl
.search:
	ld		a, (hl)
	or		a
	jr		z, .begin
	inc	hl
	jr		.search
.begin:
	ex		de, hl		; de = dst[end]
	ld		hl, (sp+6)	; hl = n
	ex		bc, hl		; bc = n
	ld		hl, (sp+4)	; hl = src
.cpy:
	ld		a, (hl)
	or		a
	jr		z, .done		; if (hl) == '\0', done
	ldi
	jr		v, .cpy
.done:
	xor	a
	ex		de, hl
	ld		(hl), a
	ld 	hl, (sp+2)
	ret
#endasm

/* START FUNCTION DESCRIPTION ********************************************
memcmp                                                          <string.h>

SYNTAX:	int memcmp( const void far * s1, const void far * s2, size_t n)

DESCRIPTION:    Performs unsigned character by character comparison of two
                memory blocks of length "n"

PARAMETER1:     Pointer to block 1.
PARAMETER2:     Pointer to block 2.
PARAMETER3:     Maximum number of bytes to compare
                if zero, both blocks are considered equal

RETURN VALUE:	< 0 if str1 is less than str2
                   char in str1 is less than corresponding char in str2
               = 0 if str1 is equal to str2
                   str1 is identical to str2
               > 0 if str1 is greater than str2
                   char in str2 is greater than corresponding char in str2

KEYWORDS: compare
END DESCRIPTION **********************************************************/

/*** Beginheader memcmp */
/*** Endheader */
_string_debug
int memcmp( const void __far * s1, const void __far * s2, size_t n)
{
#asm
#if _RAB6K
    align even ; Put 4 of the next 5 instructions on even boundary
	ld		hl, (sp+@sp+n)	;	n
	ld		py, (sp+@sp+s2)	;	py = str2
	ex		bc, hl
	test	bc
	jr		z, .countdone
    clr hl
.loop:
	ld		a, (px+hl)
	sub		a, (py)
    align even ; Align most of loop on even boundary for 16 bit mem performance
	jr		nz, .done
	inc		px
	inc		py
	dwjnz	.loop
.countdone:
	clr	hl
	jr		.over
.done:
	clr     hl
	ld		l, a		;load a sign extended into hl
	jr		nc, .over
	dec	h
.over:
#else
    align even  ; Improves performance for 16 bit memory
	ld		py, (sp+@sp+s2)	;	py = str2
	ld		hl, (sp+@sp+n)	;	n
    ld      bcde, 1
	ex		bc, hl
	test	bc
	jr		z, .countdone
.loop:
	ld		a, (px)
	ld		hl, (py)
	sub	L
	jr		nz, .done
	ld		px, px+de
	ld		py, py+de
	dwjnz	.loop
.countdone:
	clr	hl
	jr		.over
.done:
	ld		h, 0
	ld		l, a		;load a sign extended into hl
	jr		nc, .over
	dec	h
.over:
#endif
#endasm
}


/* START FUNCTION DESCRIPTION ********************************************
strcmp                                                          <string.h>

SYNTAX:	int strcmp( const char far * str1, const char far * str2)

DESCRIPTION:    Performs unsigned character by character comparison of two
                null terminated strings.

PARAMETER1:     Pointer to string 1.
PARAMETER2:     Pointer to string 2.

RETURN VALUE:	< 0 if str1 is less than str2
                   char in str1 is less than corresponding char in str2
                   str1 is shorter than but otherwise identical to str2
               = 0 if str1 is equal to str2
                   str1 is identical to str2
               > 0 if str1 is greater than str2
                   char in str2 is greater than corresponding char in str2
                   str2 is shorter than but otherwise identical to str1

KEYWORDS: string, compare
END DESCRIPTION **********************************************************/

/*** Beginheader strcmp */
/*** Endheader */
#asm __xmem
strcmp::					;		px = str1
	ld		py, (sp+7)	;		py = str2
.loop:
	ld		a, (px)
	ld		hl, (py)
	cp		L
	jr		c, .done
	jr		nz, .done
	or		a
	jr		z, .done
	ld		px, px+1
	ld		py, py+1
	jr		.loop
.done:
	ld		h, 0
	ld		l, a		;load a sign extended into hl
	jr		nc, .over
	dec	h
.over:
	lret
#endasm

/* START FUNCTION DESCRIPTION ********************************************
strncmp                                                         <string.h>

SYNTAX:	int strncmp( const char far * str1, const char far * str2, unsigned n)

DESCRIPTION:    Performs unsigned character by character comparison of two
                strings of length "n"

PARAMETER1:     Pointer to string 1.
PARAMETER2:     Pointer to string 2.
PARAMETER3:     Maximum number of bytes to compare
                if zero, both strings are considered equal

RETURN VALUE:	< 0 if str1 is less than str2
                   char in str1 is less than corresponding char in str2
               = 0 if str1 is equal to str2
                   str1 is identical to str2
               > 0 if str1 is greater than str2
                   char in str2 is greater than corresponding char in str2

KEYWORDS: string, compare
END DESCRIPTION **********************************************************/

/*** Beginheader strncmp */
/*** Endheader */
_string_debug
int strncmp( const char __far *str1, const char __far *str2, unsigned n)
{
#asm
	ld		py, (sp+@sp+str2)	;	py = str2
	ld		hl, (sp+@sp+n)	;	n
	ex		bc, hl
	clr	hl
	test	bc
	jr		z, .over
.loop:
	ld		a, (py)
	ld		d,a
	ld		a, (px)
	cp		d
	jr		nz, .done
	or		a
	jr		z, .over
	ld		px, px+1
	ld		py, py+1
	dwjnz	.loop
	jr		.over
.done:
	sbc	hl,hl		; -1 if carry, else 0
	jr		nz,.over
	inc	hl			; set +1.
.over:
#endasm
}

/* START FUNCTION DESCRIPTION ********************************************
memchr                                                          <string.h>

NEAR SYNTAX: void * _n_memchr(const void * src, int ch, size_t n);
FAR SYNTAX: void far * _f_memchr(const void far * src, int ch, size_t n);

NOTE: By Default, memchr is defined to _n_memchr.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

KEYWORDS:	memory, search

DESCRIPTION:	Searches up to "n" characters at memory pointed to by "src"
for character "ch".

RETURN VALUE:	Pointer to first occurence of "ch" if found within "n"
characters.  Otherwise returns NULL.

SEE ALSO:	strchr, strpbrk, strrchr, strstr, strtok, strcspn, strspn

END DESCRIPTION **********************************************************/

/*** Beginheader _f_memchr */
/*** Endheader */
#asm __nodebug __root
   align odd ; Aligns most of loop on even boundary to improve 16 bit mem speed
#if _RAB6K
_f_memchr::				; px = src
	ld		bcde, (sp+6)	; load second param to DE, third param to BC
   test	bc      ; check for n = 0
   jr		z, .notfound
   ld       a, e
.find:
	cp		a, (px)
	jr	z, .find_exit
	inc		px
	dwjnz	.find
.notfound:
	ld     jkhl,0   ; quicker than ld px, 0
    ld px, jkhl
.find_exit:
	ret
#else
_f_memchr::				; px = src
	ld		bcde, (sp+6)	; load second param to DE, third param to BC
    ld      jkhl, 1
   test	bc      ; check for n = 0
   jr		z, .notfound
.find:
	ld		a, (px)
	cp		e
	jr	z, .find_exit
	ld		px, px+hl     ; hl is 1...
	dwjnz	.find
.notfound:
	clr hl
    ld px, jkhl
;	ld		px, 0
.find_exit:
	ret
#endif
#endasm

/*** Beginheader _n_memchr */
/*** Endheader */
#asm __nodebug __root
_n_memchr::
	ld		bcde, (sp+4)	; load second param to DE, third param to BC
   test	bc      ; check for n = 0
   jr		z, .notfound
   ld		a,e      ; Load A with Search Character
.findchr:
   cp		(hl) 	 ; Search for char
   ret	eq			; return on a match, HL is address of match
   inc	hl
   dwjnz	.findchr
.notfound:
   clr	hl     ; Not Found, Return NULL
   ret
#endasm


/* START FUNCTION DESCRIPTION ********************************************
strchr                                                          <string.h>

NEAR SYNTAX: char * _n_strchr(const char * src, char ch);
FAR SYNTAX: char far * _f_strchr(const char far * src, char ch);

NOTE: By Default, strchr is defined to _n_strchr.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

KEYWORDS: string, search

DESCRIPTION:	Scans "src" for the first occurance of "ch".

RETURN VALUE:	Returns pointer pointing to the first occurance of "ch" in
"src".  Returns NULL if "ch" is not found.

SEE ALSO:	memchr, strpbrk, strrchr, strstr, strtok, strcspn, strspn

END DESCRIPTION **********************************************************/

/*** Beginheader _f_strchr */
/*** Endheader */
#asm __nodebug __root
_f_strchr:: ; px = src
	ld		hl, (sp+6) ; L = ch
.find:
	ld		a, (px)
	cp		L
	ret	z
	ld		px, px+1
	or		a
	jr		nz, .find
	ld		px, 0
	ret
#endasm

/*** Beginheader _n_strchr */
/*** Endheader */
#asm __nodebug
_n_strchr::
	ld		bcde, (sp+4)	; load second param to DE, junk to BC
_strchr:
   ld    a,(hl)   ; Get Next Character
   cp    e        ; Check for Match
   ret   z        ; If Found, Done
   or    a        ; Return NULL if End of String
   inc   hl       ; Bump Pointer
   jp    nz,_strchr
   clr	hl
   ret
#endasm


/* START FUNCTION DESCRIPTION ********************************************
strpbrk                                                         <string.h>

NEAR SYNTAX: char * _n_strpbrk(const char * s1, const char * s2);
FAR SYNTAX: char far * _f_strpbrk(const char far * s1, const char far * s2);

NOTE: By Default, strpbrk is defined to _n_strpbrk.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

KEYWORDS: string, search

DESCRIPTION:	Scans "s1" for the first occurence of any characters
specified in "s2".

RETURN VALUE:	Pointer pointing to the first occurence of a character
contained in "s2" in "s1".  Returns NULL if not found.

SEE ALSO:	memchr, strchr, strrchr, strstr, strtok, strcspn, strspn

END DESCRIPTION **********************************************************/

/*** Beginheader _f_strpbrk */
/*** Endheader */
#asm __nodebug
_f_strpbrk::
	ld		py, (sp+6)
	ld		a, (px)
	or		a
	ld		c, a
	jr		z, .notfound
	ld		a, (py)
	or		a
	jr		z, .notfound
.outerloop:				; px = s1
	ld		pz, py		; py = s2
	ld		a, (pz)
.innerloop:
	cp		c
	ret	z
	ld		pz, pz+1
	ld		a, (pz)
	or		a
	jr		nz, .innerloop
	ld		px, px+1
	ld		a, (px)
	or		a
	ld		c, a
	jr		nz, .outerloop
.notfound:
	ld		px, 0
	ret
#endasm

/*** Beginheader _n_strpbrk */
/*** Endheader */
#asm __nodebug
_n_strpbrk::

	ld		bcde, (sp+4)	; load second param to DE, junk to BC
	push	hl					; Save parameters
	push	de

	xor	a					; Is brk NULL ?
	ex		de,hl
	cp		(hl)
	ex		de,hl
	jr		NZ,.strpbrkr
	pop	de
	pop	de
	clr	hl
	ret						; Return NULL if it is

.strpbrkr:
	ld		a,(hl)			; Get next src
	or		a					; At end of src?
	jr		NZ,.findchar
	pop	de					; Cleanup the stack
	pop	de
	clr	hl				; Return NULL pointer
	ret

.findchar:
	ex		de,hl
	cp		(hl)				; src = brk ?
	ex		de,hl
	jr		Z,.retpos
	inc	de					; Point to next brk
	ld		b,a
	xor	a
	ex		de,hl
	cp		(hl)				; End of brk string?
	ex		de,hl
	ld		a,b
	jr		NZ,.findchar

	pop	de					; Get original brk pointer
	push	de
	inc	hl					; Point to next src
	jr		.strpbrkr

.retpos:
	pop	de					; Cleanup the stack
	pop	de					;
	ret						; Return HL
#endasm


/* START FUNCTION DESCRIPTION ********************************************
strrchr                                                         <string.h>

NEAR SYNTAX: char * _n_strrchr(const char * s, int c);
FAR SYNTAX: char far * _f_strrchr(const char far * s, int c);

NOTE: By Default, strrchr is defined to _n_strrchr.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

DESCRIPTION:	Similar to strchr, except this function searches backward
from the end of "s" to the beginning.

RETURN VALUE:	Pointer to last occurence of "c" in "s".  If "c" is not
found in "s", return NULL.

KEYWORDS: string, search

SEE ALSO:	memchr, strchr, strpbrk, strstr, strtok, strcspn, strspn

END DESCRIPTION **********************************************************/

/*** Beginheader _f_strrchr */
/*** Endheader */
__nodebug
char far *_f_strrchr(const char far *s, int c)
{
	#asm
	ld		hl, (sp+@sp+c)			; HL = c (really just L = c)
	ld		pz, 0						; PZ = NULL (i.e. the default result)
	.find:
	ld		a, (px+0)				; A = current char from string s
	cp		L							; is the current char from s a match for c?
	jr		nz, .next				; if no (Zero flag reset), go check for end of s

	ld		pz, px					; PZ = updated result (pointer to latest c found)
	.next:
	ld		px, px+1					; PX = pointer to the next char in s
	or		a							; end of s?
	jr		nz, .find				; if no (Zero flag reset), go check next char in s

	ld		px, pz					; PX = PZ = NULL vs. pointer to last c found
	#endasm
	// char far * result is returned in PX
}


/*** Beginheader _n_strrchr */
/*** Endheader */
_string_debug
char * _n_strrchr(const char *str, int c)
{
	char __far *ret;

	ret = _f_strrchr(str, c);
	return (char *)(uint16)(unsigned long)ret;
}


/* START FUNCTION DESCRIPTION ********************************************
strstr                                                          <string.h>

NEAR SYNTAX: char * _n_strstr(const char * s1, const char * s2);
FAR SYNTAX: char far * _f_strstr(const char far * s1, const char far * s2);

NOTE: By Default, strstr is defined to _n_strstr.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

KEYWORDS: string, search

DESCRIPTION:	Finds a substring specified by "s2" in string "s1".

RETURN VALUE:	Pointer pointing to the first occurence of substring "s2"
in "s1".  Returns NULL if "s2" is not found in "s1".

SEE ALSO:	memchr, strchr, strpbrk, strrchr, strtok, strcspn, strspn

END DESCRIPTION **********************************************************/

/*** Beginheader _f_strstr */
/*** Endheader */
__nodebug
char far *_f_strstr(const char far *s1, const char far *s2)
{
	#asm
	ld		pz, px				; PZ = PX = s1 (string to search)
	.outerloop:
	ld		py, (sp+@sp+s2)	; PY = s2 (string to find)
	.innerloop:
	ld		a, (py+0)			; A = s2[current offset]
	or		a						; end of s2 string?
	jr		nz, .check_s1_end	; if no (Zero flag reset), go check for end of s1

	ld		px, pz				; PX = PZ = current start of string match in s1
	jr		.done					; go return string match address

	.check_s1_end:
	ld		e, a					; save s2[current offset]
	ld		a, (px+0)			; A = s1[current start+offset]
	or		a						; end of s1 string?
	jr		z, .notfound		; if yes (Zero flag set), go return no match found

	ld		px, px+1				; increment s1 start+offset
	ld		py, py+1				; increment s2 offset
	cp		e						; s1[start+offset] == s2[offset]?
	jr		z, .innerloop		; if yes (Zero flag set), go check next chars' match

	ld		pz, pz+1				; PZ = incremented start position in s1
	ld		px, pz				; PX = updated start position in s1
	jr		.outerloop

	.notfound:
	ld		px, 0					; PX = NULL
	.done:
	#endasm
	// char far * result is returned in PX
}


/*** Beginheader _n_strstr */
/*** Endheader */
__nodebug
char *_n_strstr(const char *s1, const char *s2)
{
	char __far *ret;

	ret = _f_strstr(s1, s2);
	return (char *)(uint16)(unsigned long)ret;
}


/* START FUNCTION DESCRIPTION ********************************************
strtok                                                          <string.h>

NEAR SYNTAX: char * _n_strtok(char * src, const char * brk);
FAR SYNTAX: char far * _f_strtok(char far * src, const char far * brk);

NOTE: By Default, strtok is defined to _n_strtok.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	To explicitly call the near version when the USE_FAR_STRING_LIB macro
	is defined and all pointers are near pointers, append _n_ to the
	function name, e.g. _n_strtod. For more information about FAR pointers,
	see the Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

	Because the near version of this function is a wrapper around the far
	version, the near version of this function will run slightly slower
	than the near version.

	Warning: The _f_strtok function saves state between calls. Because the
	_n_strtok near version is a wrapper around the _f_strtok far version
	and thus the single saved state is shared, the near and far versions
	must not be called in an interleaved manner.

KEYWORDS: string, search

DESCRIPTION:	Scans "src" for tokens separated by delimiter characters
specified in "brk".

First call with non-NULL for "src".  Subsequent calls with NULL for "src"
continues to search tokens in the string. If a token is found (i.e. a
delimiter is found), replaces the first delimiter in "src" with a null
character so that "src" points to a proper null-terminated token.  This
function is not task reentrant.

RETURN VALUE:	Pointer to a token.  If no delimiter (therefore no token)
is found, returns NULL.

SEE ALSO:	memchr, strchr, strpbrk, strrchr, strstr, strcspn, strspn

END DESCRIPTION **********************************************************/

/*** Beginheader _f_strtok */
/*** Endheader */
__nodebug
char far *_f_strtok(char far *src, const char far *brk)
{
	static char __far * __near prior = NULL;

	#asm
	ld		bcde, px					; BCDE = PX = src
	test	bcde						; src == NULL?
	jr		nz, .st2					; if no (Zero flag reset), go handle initial call

	ld		bcde, (prior)			; src == NULL, reload prior call's end of token
	ld		px, bcde					; PX = BCDE = prior
	test	bcde						; prior == NULL?
	jr		z, .done					; if last result was NULL, go return NULL again

	.st2:
	ld		py, (sp+@sp+brk)		; PY = brk
	push	px							; preserve strspn's src across the strspn call
	push	py							; stack up strspn call's brk
	push	px							; stack up strspn call's src
	call	strspn					; result in HL = segment length
	add	sp, 8						; clean up strspn call's stack residue
	pop	bcde						; recover strspn call's src
	ld		jk, 0						; JKHL = segment length (delimiters skip count)
	add	jkhl, bcde				; JKHL = strspn call's src + strspn(src, brk)
	ld		px, jkhl					; PX = JKHL = address of token's first character
	ld		a, (px)					; get the token's first character
	or		a							; is it a null?
	jr		nz, .st1					; if no (Zero flag reset), go find the token's end

	ld		bcde, 0					; NULL result for empty token
	ld		(prior), bcde			; prior = NULL
	ld		px, bcde					; PX = NULL
	jr		.done						; delimited token not found, go return NULL

	.st1:
	ld		py, (sp+@sp+brk)		; PY = brk
	push	px							; preserve this token's physical start address
	push	py							; stack up _f_strpbrk call's s2
	push	px							; stack up _f_strpbrk call's s1
	call	_f_strpbrk				; result in PX = address of end of token vs. NULL
	add	sp, 8						; clean up _f_strpbrk call's stack residue
	ld		bcde, px					; BCDE = address of end of token
	test	bcde						; does address of end of token == NULL?
	jr		z, .st3					; if yes (Zero flag set), go set prior = NULL

	xor	a							; A = null character
	ld		(px), a					; change token's delimiter to a string terminator
	ld		px, px+1					; point to next address after token's terminator
	ld		bcde, px					; BCDE = next token search's start address
	.st3:
	ld		(prior), bcde			; prior = NULL vs. next token search's start addr
	pop	px							; recover this token's physical start address
	.done:
	#endasm
	// char far * result is returned in PX
}

/*** Beginheader _n_strtok */
/*** Endheader */
__nodebug
char *_n_strtok( char *s1, const char *s2)
{
	char __far *ret;

	ret = _f_strtok( s1, s2);
	return (char *)(word)(unsigned long)ret;
}


/* START FUNCTION DESCRIPTION ********************************************
strcspn                                                         <string.h>

SYNTAX:	size_t strcspn( const char far * src, const char far * brk);

KEYWORDS: string, search

DESCRIPTION:	Scans a string for the initial segment in "src" containing
               only characters NOT specified in "brk".

RETURN VALUE:	Returns the length of the segment.

SEE ALSO:	memchr, strchr, strpbrk, strrchr, strstr, strtok, strspn

END DESCRIPTION **********************************************************/

/*** Beginheader strcspn */
/*** Endheader */
__nodebug
size_t (strcspn)(const char far *src, const char far *brk)
{
	#asm
	ld		pz, (sp+@sp+brk)		; PZ = brk (save brk for fast reload)
.outerloop:
	ld		a, (px+0)				; A = src[current span]
	or		a							; end of src string?
	jr		z, .done					; if yes (Zero flag set), go return segment length

	ld		c, a						; save src[current span] character for comparison
	ld		py, pz					; PY = PZ = brk (fast reload of original value)
.innerloop:
	ld		a, (py+0)				; A = brk[current offset]
	cp		c							; src[current span] == brk[current offset]?
	jr		z, .done					; if yes (Zero flag set), go return segment length

	ld		py, py+1					; increment offset
	or		a							; end of brk string?
	jr		nz, .innerloop			; if no (Zero flag reset), go check next brk char

	ld		px, px+1					; increment span
	jr		.outerloop

	.done:
	ld		bcde, (sp+@sp+src)	; BCDE = src
	ld		jkhl, px					; JKHL = PX = src + segment length (span)
	sub	jkhl, bcde				; JKHL = string segment length
	#endasm
	// the "truncated" size_t result is returned in HL
}


/* START FUNCTION DESCRIPTION ********************************************
strspn                                                          <string.h>

SYNTAX:	size_t strspn( const char far * src, const char far * brk);

KEYWORDS: string, search

DESCRIPTION:	Scans a string for the initial segment in "src" containing
               only characters specified in "brk".

RETURN VALUE:	Returns the length of the segment.

SEE ALSO:	memchr, strchr, strpbrk, strrchr, strstr, strtok, strcspn

END DESCRIPTION **********************************************************/

/*** Beginheader strspn */
/*** Endheader */
__nodebug
size_t (strspn)(const char far *src, const char far *brk)
{
	#asm
	ld		bcde, (sp+@sp+brk)	; BCDE = brk
	test	bcde						; brk == NULL?
	jr		z, .done					; if yes (Zero flag set), go return zero length

	ld		py, bcde					; PY = BCDE = brk
	ld		pz, py					; PZ = brk (save brk for fast reload)

	.strspan:
	ld		a, (px+0)				; get character from src + current segment length
	or		a							; nul (string terminator, i.e. no more src chars)?
	jr		z, .done					; if yes (Zero flag set), go return segment length

	ld		e, a
	.findchar:
	ld		a, (py+0)				; get character from brk + current offset
	or		a							; nul (string terminator, i.e. no more brk chars)?
	jr		z, .done					; if yes (Zero flag set), go return segment length

	cp		e							; char from src == char from brk?
	jr		z, .getnext				; if yes (Zero flag set), go check next src char

	ld		py, py+1					; no match, increment brk + current offset
	jr		.findchar				; go check next brk char

	.getnext:
	ld		py, pz					; PY = PZ = brk (fast reload of original value)
	ld		px, px+1					; increment src + current segment length
	jr		.strspan					; go check next src char

	.done:
	ld		bcde, (sp+@sp+src)	; BCDE = src
	ld		jkhl, px					; JKHL = PX = src + segment length (span)
	sub	jkhl, bcde				; JKHL = string segment length
	#endasm
	// the "truncated" size_t result is returned in HL
}


/* START FUNCTION DESCRIPTION ********************************************
memset                                                          <string.h>

NEAR SYNTAX: void * _n_memset( void * dst, int chr, size_t n);
FAR SYNTAX: void far * _f_memset( void far * dst, int chr, size32_t n);

NOTE: By Default, memset is defined to _n_memset.

	For Rabbit 4000+ users, this function supports FAR pointers.
	The macro USE_FAR_STRING_LIB will change all calls to functions in
	this library to their far versions by default. The user may also
	explicitly call the far version with _f_strfunc where strfunc is
	the name of the string function.

	Because FAR addresses are larger, the far versions of this function
	will run slightly slower than the near version.  To explicitly call
	the near version when the USE_FAR_STRING_LIB macro is defined and all
	pointers are near pointers, append _n_ to the function name, e.g.
	_n_strtod. 	For more information about FAR pointers, see the
	Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

DESCRIPTION: Sets the first "n" bytes of "dst" to the character "chr"

RETURN VALUE: dst

KEYWORDS: memory

END DESCRIPTION **********************************************************/

/*** Beginheader _f_memset */
/*** Endheader */
#asm __nodebug __root
_f_memset:: ; px contains dst
	ld		hl, (sp+6) ; chr
   ld		a, L
	ld		jkhl, (sp+8) ; n
	test	jkhl
	ret	z			; n == 0
	ld		bcde,-1
	add	jkhl,bcde
	test	jkhl			; set zero flag if n == 1
	ld		(px), a
	ret	z			; n == 1
	add	jkhl,bcde
   test	jkhl
   ld		(px+1), a
   ret	z			; n == 2
   ld		bc, hl   ;
   ex		jk,hl		; Get extra 64k multiples in hl
   test	bc
   jr		nz,.start
   dec	hl			; If BC was zero, then dec 64k count, since will move initial 64k
.start:
	ld		pz, px	; save for return
	ld		py, px+2
.loop:
#if _BOARD_TYPE_ == 0x2700
	call copy_func
#else
   copy
#endif
	test	hl
	jr		z,.done
	dec	hl
	jr		.loop	; BC remains zero from last iteration, hence will move another 64k
.done:
	ld		px, pz ; reload dst for return
   ret
#endasm

/*** Beginheader _n_memset */
/*** Endheader */
#asm __nodebug __root
_n_memset::
	ld		bcde, (sp+4)	; load second param to DE, third param to BC
   test	bc
   ret   z          ; nothing to move, hl is already set to dst

	push	hl         ; save copy of destination for return value

   ld 	(hl),e     ; seed destination w/ chr (also handles case of count == 1)

   dec	bc         ; set up ldir  bc = count = n - 1
			           ;              hl = source = dest
	test	bc         ;   make a last check to see if there is anything to move
	jr		z,.memsetx ;   if count was 1, don't need to copy any more

   ld		de,hl      ; finish setting up ldir  de = destination = dest + 1
   inc	de

   ldir             ; fill block:  do { *de++ = *hl++; } while (--bc)

.memsetx:
   pop	hl			  ; reload destination for return value
   ret
#endasm


/* START FUNCTION DESCRIPTION ********************************************
strlen                                                          <string.h>

SYNTAX:	size_t strlen( const char far * s);

DESCRIPTION:	Calculate the length of "s".

RETURN VALUE:	Number of bytes in a string.

KEYWORDS: string

END DESCRIPTION **********************************************************/

/*** Beginheader strlen */
/*** Endheader */
__nodebug
size_t strlen(const char far *s)
{
	#asm
#if _RAB6K
	align odd ; Nicely aligns majority of code on even boundary
    clr hl
	ld		py, px					; PY = PX = s
	.find_end:
	ld		a, (px + hl)            ; Unroll the loop a bit for speed
	or		a
	jr		z, .found_end
	inc		px

	ld		a, (px + hl)
	or		a
	jr		z, .found_end
	inc		px

	ld		a, (px + hl)
	or		a
	jr		z, .found_end
	inc 	px

	ld		a, (px + hl)
	or		a
	jr		z, .found_end
	inc		px

	jr		.find_end

	.found_end:
	ld		jkhl, px		; JKHL = PX = src + string length
	sub	jkhl, py	; JKHL = string length
#else
	ld		bcde, px					; BCDE = PX = s
	.find_end:
	ld		a, (px+0)
	or		a
	jr		z, .found_end

	ld		px, px+1
	jr		.find_end

	.found_end:
	ld		jkhl, px		; JKHL = PX = src + string length
	sub	jkhl, bcde	; JKHL = string length
#endif
	#endasm
	// the "truncated" size_t result is returned in HL
}


/*** BeginHeader strcoll */
#define strcoll( str1, str2)		strcmp( str1, str2)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
strcoll                                                         <string.h>

SYNTAX:	int strcoll( const char far *str1, const char far *str2)

DESCRIPTION:	Compare two strings using the current locale.  Since Dynamic
					C only supports the "C" locale, this function is the same
					as calling strcmp().

PARAMETER 1:	Pointer to string 1.

PARAMETER 2:	Pointer to string 2.

RETURN VALUE:	< 0 if str1 is less than str2
                   char in str1 is less than corresponding char in str2
                   str1 is shorter than but otherwise identical to str2
               = 0 if str1 is equal to str2
                   str1 is identical to str2
               > 0 if str1 is greater than str2
                   char in str2 is greater than corresponding char in str2
                   str2 is shorter than but otherwise identical to str1

SEE ALSO:	strxfrm, setlocale

END DESCRIPTION **********************************************************/
/*
	http://www.gnu.org/s/libc/manual/html_node/Collation-Functions.html:

	"In the standard C locale, the collation sequence for strcoll is the same as
	that for strcmp."
*/
_string_debug
int (strcoll)( const char __far *str1, const char __far *str2)
{
	return strcmp( str1, str2);
}


/*** BeginHeader strxfrm */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
strxfrm                                                         <string.h>

SYNTAX:	size_t strxfrm( char far *s1, const char far *s2, size_t n)

NOTE:		Since Dynamic C only supports the "C" locale, this function is
			equivalent to snprintf( s1, n, "%ls", s2).  No transformation of
			characters is performed.

DESCRIPTION:	Transforms <s2> and places the resulting string in <s1>.  The
					transformation is such that if strcmp() is applied to two
					transformed strings, it returns the same result as calling
					strcoll() on the two original strings.

					No more than <n> characters are placed into <s1>, including the
					terminating null character.

PARAMETER 1:	Buffer to hold the transformed string.

PARAMETER 2:	String to transform.

PARAMETER 3:	Maximum number of bytes (including null terminator) to write
					to buffer <s1>.

RETURN VALUE:	Length of the transformed string (not including the null
					terminator).  If the value returned is <n> or more, the contents
					of <s1> are indeterminate.

END DESCRIPTION **********************************************************/
_string_debug
size_t strxfrm( char __far *s1, const char __far *s2, size_t n)
{
	return snprintf( s1, n, "%Fs", s2);
}


/*** BeginHeader strerror */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
strerror                                                        <string.h>

SYNTAX:	char far *strerror( int errnum)

DESCRIPTION:	Returns an error message string for the <errnum>.

PARAMETER 1:	Error number to look up.

RETURN VALUE:	String with error message.  This string should not be modified
					by the caller, and may be overwritten by a subsequent call to
					strerror().

SEE ALSO:	perror

END DESCRIPTION **********************************************************/
_string_debug
char __far *strerror( int errnum)
{
	static __far char buffer[40];

	return _error_message( errnum, buffer);
}