/*
	signal.h

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
	C90 - 7.7 Signal Handling
*/

#ifndef __SIGNAL_H
#define __SIGNAL_H

	typedef int sig_atomic_t;
	typedef void _signal_func(int);
	typedef void (*_signal_func_ptr)(int);

	// Macros which expand to constant expressions with distinct values that
	// have type compatible with the second argument to and the return value of
	// the signal function.  Note SIG_DFL is 0 so we can easily initialize
	// _signal_table to all SIG_DFL.
	#define SIG_DFL	(_signal_func *) 0			// default handling
	#define SIG_ERR	(_signal_func *) -1			// return value for signal()
	#define SIG_IGN	(_signal_func *) 1			// ignore signal

	// Macros which expand to positive integral constant expressions that are
	// the signal number corresponding to the specified condition.
	#define SIGABRT	0	// abnormal termination, such as initiated by abort()
	#define SIGFPE		1	// floating-point exception (divide by zero or overflow)
	#define SIGILL		2	// illegal instruction
	#define SIGINT		3	// interactive attention signal
	#define SIGSEGV	4	// invalid access to storage
	#define SIGTERM	5	// termination request sent to program

	#define _SIGNAL_COUNT 6
	extern _signal_func *_signal_table[];

	// Second parameter and return type of signal() are function pointers that
	// take a single int argument and return void.
	// DC can't handle standard declaration:
	// void (*signal( int sig, void (*func)(int)))(int);
	_signal_func *signal( int sig, _signal_func *func);

	int raise( int sig);

	#use "signal.c"

#endif