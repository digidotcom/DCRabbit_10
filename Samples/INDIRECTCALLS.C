/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/***********************************************************

      samples\indirectCalls.c
	
This program demonstrates the use of pointers to functions
to do indirect calls. Two ways of declaring a pointer to a
function are shown. Dynamic C differs from standard C in that
argument lists are not used in the declarations of pointers
to functions, although arguments may be used in indirect calls.
         
************************************************************/
#class auto


int intfunc(int x, int y);


void main(){

	int x,y; 

	// create a pointer to int function type
	typedef int (*fnptr)(); 

	// declare a variable as fnptr using the above typedef.
	fnptr  fp2;     

	// declare fnc1 as a pointer to an int function
	int (*fnc1)();  

	fnc1 = intfunc; 	// initialize fnc1 to point to intfunc
	fp2 =  intfunc;  	// init. fp2 to point to the same function

	x = (*fnc1)(1,2); // call intfunc via fnc1
	y = (*fp2)(3,4);  // call intfunc via fp2

	printf("x = %d\n", x);
	printf("y = %d\n", y);
}

int intfunc(int x, int y){
	return x+y;
}
