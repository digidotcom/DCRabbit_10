/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	C90 - 7.10 General Utilities
*/

#ifndef __STDLIB_H
#define __STDLIB_H
	// Because of the double pointer, our string conversion routines require
	// near and far versions.  Map strtod, strtol and strtoul to either the
	// near or far version.
	#ifdef USE_FAR_STRING_LIB
		#define strtod		_f_strtod
		#define strtol		_f_strtol
		#define strtoul	_f_strtoul
		#define qsort		_f_qsort
		#define bsearch	_f_bsearch
	#else
		#define strtod		_n_strtod
		#define strtol		_n_strtol
		#define strtoul	_n_strtoul
		#define qsort		_n_qsort
		#define bsearch	_n_bsearch
	#endif

	// deprecated function names
	#define _n_atof	atof
	#define _f_atof	atof
	#define _n_atoi	atoi
	#define _f_atoi	atoi
	#define _n_atol	atol
	#define _f_atol	atol


	#define NULL				(void *) 0
	typedef unsigned short	size_t;
	typedef unsigned long	size32_t;				// Dynamic C extension

	// 7.10.1 String conversion functions
	double atof( const char __far *nptr);
	int atoi( const char __far *nptr);
	long atol( const char __far *nptr);

	// double strtod( const char *nptr, char **endptr);
	double _n_strtod( const char * s, char ** tailptr);
	double _f_strtod( const char __far * s, char __far * __far * tailptr);

	// long strtol( const char *nptr, char **endptr, int base);
	long _n_strtol( const char *sptr, char **tailptr, int base);
	long _f_strtol( const char __far *sptr, char __far * __far *tailptr, int base);

	// unsigned long strtoul( const char *nptr, char **endptr, int base);
	unsigned long _n_strtoul( const char *nptr, char **endptr, int base);
	unsigned long _f_strtoul( const char __far *nptr, char __far * __far *endptr,
																						int base);

	// 7.10.2 Pseudo-random sequence generation functions
	#define RAND_MAX 32767

	int rand( void);
	void srand( unsigned int seed);


	// 7.10.3 Memory management functions
	void __far *calloc( size32_t nmemb, size32_t membsize);
	void free( void __far *ptr);
	void __far *malloc( size32_t bytes);
	void __far *realloc( void __far *ptr, size32_t bytes);

	// "root" API for calloc/free/malloc/realloc, Dynamic C extension
	void *_root_calloc( size_t nmemb, size_t membsize);
	void _root_free( void * ptr);
	void *_root_malloc(size_t bytes);
	void *_root_realloc( void *oldmem, size_t bytes);

	// 7.10.4 Communication with the environment
	#define EXIT_FAILURE		1
	#define EXIT_SUCCESS		0

	typedef void (*_exit_func)(void);

	#ifndef _ATEXIT_FUNC_COUNT
		#define _ATEXIT_FUNC_COUNT 4
	#endif
	#if _ATEXIT_FUNC_COUNT > 0
		extern _exit_func _atexit_func_table[];
	#endif

	// Dynamic C uses "abort" as a keyword for costates.  We can use a macro
	// to work around that limitation.
	#define __abort() _abort()
	void _abort( void);
	int atexit( _exit_func func);
	void exit( int status);
	char *getenv( const char __far *name);
	int system( const char __far *string);


	// 7.10.5 Searching and sorting utilities
	typedef int (*_compare_func_n)(const void *, const void *);
	typedef int (*_compare_func_f)(const void __far *, const void __far *);

	void *_n_bsearch( const void *key, const void *base,
								size_t nmemb, size_t membsize, _compare_func_n compar);
	void __far *_f_bsearch( const void __far *key, const void __far *base,
								size_t nmemb, size_t membsize, _compare_func_f compar);

	void _n_qsort( void *base, size_t nmemb, size_t membsize,
																		_compare_func_n compar);
	void _f_qsort( void __far *base, size_t nmemb, size_t membsize,
																		_compare_func_f compar);


	// 7.10.6 Integer arithmetic functions
	typedef struct div_t
	{
		int quot;		// quotient
		int rem;			// remainder
	} div_t;
	typedef struct ldiv_t
	{
		long quot;		// quotient
		long rem;		// remainder
	} ldiv_t;

	int abs(int j);
	div_t div(int numer, int denom);
	long int labs(long int j);
	ldiv_t ldiv(long int numer, long int denom);


	// 7.10.7 Multibyte character functions
	typedef char				wchar_t;
	#define MB_CUR_MAX 1

	int mblen( const char *s, size_t n);
	int mbtowc( wchar_t *pwc, const char *s, size_t n);
	int wctomb( char *s, wchar_t wchar);
	size_t mbstowcs( wchar_t *pwcs, const char *s, size_t n);
	size_t wcstombs( char *s, const wchar_t *pwcs, size_t n);

	#use "stdlib.c"
	#use "malloc.lib"

#endif