/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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