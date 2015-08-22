/********************************************************************

	Define_error_handler.c
	Copyright (c) 2008 Digi International Inc., All Rights Reserved

	This program demonstrates a custom runtime-error handler.

	The function defineErrorHandler() allows the user to install
	their own custom function to handle runtime errors.
	Information about the runtime error is pushed on the stack
	and can be accessed by the function.  When the custom error
	handler is running, the following information will be on
	the stack:

				SP+0	return adress for error handler call
				SP+2	runtime error code
				SP+4	0x0000 (can be used for additional information)
				SP+6	XPC when exception() was called
				SP+8	address where exception() was called

	Test some different runtime errors by uncommenting different
	parts of the code.

	Notice that this particular user-defined error handler will
	pass execution back to the main program if a domain or
	range error occurs.  Most runtime errors called from Dynamic C
	libraries will exit if execution continues.  The domain and
	range errors in the math library, however, will continue on
	with the appropriate value (INF, zero, etc.) if the user-defined
	error handler passes control back to the executing program.
	In addition, the unexpected interrupt error handler will
	return from the interrupt and continue if execution is passed
	back to it.

 ********************************************************************/
#class auto

root void myRuntimeErrHandler(int error_code, int more_info,
                              int xpc_val, int address);

main()
{
	float	f;

	defineErrorHandler(myRuntimeErrHandler);


	// the default error handler will halt on these; but the
	// custom handler will pass execution back to the program
	f = acos(2);
	f = fmod(1, 0);

	// this runtime error will halt execution (ensure that NDEBUG is not defined)
#if defined NDEBUG
	#fatal "Ensure that NDEBUG is not defined!"
#endif
	assert(0);

	return(0);
}

/********************************************************************

	This runtime error handler will print out relevant information
	about the error that occurs then pass execution back to the
	function that called it (although in most cases that function
	will then exit to Dynamic C with a runtime error).

	Note that this function should always be in root memory!

 ********************************************************************/

root void myRuntimeErrHandler(int error_code, int more_info,
                              int xpc_val, int address)
{
	printf("%ls (run time exception %d) occurred at %03x:%04x\n",
	       error_message(error_code), error_code, xpc_val, address);

	// Now let's pass execution back if it is a domain or range error
	// but trap all the other runtime errors.

	if (error_code == -ERR_DOMAIN || error_code == -ERR_RANGE) {
		printf("  - Execution being passed back to program...\n");
		return;
	}

	printf("  - This error will halt the program.\n");

	///////////////////////////////////////////
	// halt all errors that we don't want to pass back to execution,
	//		i.e. everything except domain and range errors
   exit(error_code);
}