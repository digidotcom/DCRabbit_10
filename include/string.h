/*
	string.h

	Copyright (c) 2009-10 Digi International Inc., All Rights Reserved

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
	C90 - 7.11 String handling
*/

#ifndef __STRING_H
#define __STRING_H

	// Because of the return value, many of these functions require near and
	// far versions.
	#ifdef USE_FAR_STRING_LIB
		#define memcpy		_f_memcpy
		#define memmove	_f_memmove
		#define strcpy		_f_strcpy
		#define strncpy	_f_strncpy
		#define strcat		_f_strcat
		#define strncat	_f_strncat
		#define memchr		_f_memchr
		#define strchr		_f_strchr
		#define strpbrk	_f_strpbrk
		#define strrchr	_f_strrchr
		#define strstr		_f_strstr
		#define strtok		_f_strtok
		#define memset		_f_memset
	#else
		#define memcpy		_n_memcpy
		#define memmove	_n_memmove
		#define strcpy		_n_strcpy
		#define strncpy	_n_strncpy
		#define strcat		_n_strcat
		#define strncat	_n_strncat
		#define memchr		_n_memchr
		#define strchr		_n_strchr
		#define strpbrk	_n_strpbrk
		#define strrchr	_n_strrchr
		#define strstr		_n_strstr
		#define strtok		_n_strtok
		#define memset		_n_memset
	#endif

	// Do not use the following listed deprecated function name macros, which
	// will be removed from a future Dynamic C release. Instead of using the
	// listed _f_ or _n_ prefixed function name macros, call the "ordinary,
	// unadorned" functions directly.
	#define _f_memcmp		memcmp
	#define _f_strcmp		strcmp
	#define _f_strncmp	strncmp
	#define _f_strlen		strlen
	#define _f_strspn		strspn
	#define _f_strcspn	strcspn

	#define _n_memcmp		memcmp
	#define _n_strcmp		strcmp
	#define _n_strncmp	strncmp
	#define _n_strlen		strlen
	#define _n_strspn		strspn
	#define _n_strcspn	strcspn
	// End of this list of deprecated function name macros.

	#define NULL				(void *) 0
	typedef unsigned short	size_t;
	typedef unsigned long	size32_t;

	// 7.11.2 Copying functions
	void __far *_f_memcpy( void __far *s1, const void __far *s2, size32_t n);
	void __far *_f_memmove( void __far *s1, const void __far *s2, size32_t n);
	char __far *_f_strcpy( char __far *s1, const char __far *s2);
	char __far *_f_strncpy( char __far *s1, const char __far *s2, size_t n);

	void *_n_memcpy( void *s1, const void *s2, size_t n);
	void *_n_memmove( void *s1, const void *s2, size_t n);
	char *_n_strcpy( char *s1, const char *s2);
	char *_n_strncpy( char *s1, const char *s2, size_t n);

	// 7.11.3 Concatenation functions
	char __far *_f_strcat( char __far *s1, const char __far *s2);
	char __far *_f_strncat( char __far *s1, const char __far *s2, size_t n);

	char *_n_strcat( char *s1, const char *s2);
	char *_n_strncat( char *s1, const char *s2, size_t n);

	// 7.11.4 Comparison functions
	int memcmp( const void __far *s1, const void __far *s2, size_t n);
	int strcmp( const char __far *s1, const char __far *s2);
	int strcoll( const char __far *s1, const char __far *s2);
	int strncmp( const char __far *s1, const char __far *s2, size_t n);
	size_t strxfrm( char __far *s1, const char __far *s2, size_t n);

	// 7.11.5 Search functions
	void __far *_f_memchr( const void __far *s, int c, size_t n);
	char __far *_f_strchr( const char __far *s, int c);
	char __far *_f_strpbrk( const char __far *s1, const char __far *s2);
	char __far *_f_strrchr( const char __far *s, int c);
	char __far *_f_strstr( const char __far *s1, const char __far *s2);
	char __far *_f_strtok( char __far *s1, const char __far *s2);

	void *_n_memchr( const void *s, int c, size_t n);
	char *_n_strchr( const char *s, int c);
	char *_n_strpbrk( const char *s1, const char *s2);
	char *_n_strrchr( const char *s, int c);
	char *_n_strstr( const char *s1, const char *s2);
	char *_n_strtok( char *s1, const char *s2);

	size_t strcspn( const char __far *s1, const char __far *s2);
	size_t strspn( const char __far *s1, const char __far *s2);

	// 7.11.6 Miscellaneous functions
	void __far *_f_memset( void __far *s, int c, size32_t n);
	void *_n_memset( void *s, int c, size_t n);
	char __far *strerror( int errnum);
	size_t strlen( const char __far *s);

	#use "string.c"
#endif