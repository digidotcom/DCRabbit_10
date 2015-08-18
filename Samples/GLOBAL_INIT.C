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
/*********************************************************

     Samples\global_init.c

     This sample demonstrates the use of #GLOBAL_INIT
     sections to run initialization code such as
     initializing static variables.
     
     Because static variables are placed in flash if
     they are initialized upon declaration, e.g. int x=0;,
     #GLOBAL_INIT sections are the correct method in
     Dynamic C to pre-initialize static variables whose
     values will be modified later in the program.

**********************************************************/
#class auto

void foo();
int y;

void main(){
	// The default storage type is auto (but we don't want to depend on any such
	// assumptions) and we need this to be static. Hence, the explicit static.
	static int x;  

	// #GLOBAL_INIT sections must appear after variable
	//  declarations and before executable code
	#GLOBAL_INIT
	{
		x = 1234;
	}

	// note that the execution cursor stops here after 
	//  compilation with F5
	printf("x = %d\n\n",x);
	printf("y = %d\n\n",y);
	x = 0;
	foo();
	printf("x = %d\n\n",x);
	printf("y = %d\n\n",y);	
}


void foo()
{
	// #GLOBAL_INIT sections must appear after variable
	//  declarations and before executable code
	#GLOBAL_INIT
	{
		y = 4321;
	}
	y = 0;
}
