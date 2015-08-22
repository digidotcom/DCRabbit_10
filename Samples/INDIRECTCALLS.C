/***********************************************************

      samples\indirectCalls.c
      Z-World, 2000
	
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
