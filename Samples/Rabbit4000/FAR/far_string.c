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
/*****************************************************************************

     far_string.c

		This sample demonstrates the use of FAR strings for the Rabbit 4000.

		To call a FAR string or memory function, add _f_ to the beginning of
		the function name e.g. _f_strcpy.  If you wish only to use FAR versions
		of these functions, define the macro USE_FAR_STRING_LIB in the project
		options defines box.  This will define (with macros) each string function
		to its FAR equivalent.  See far_string2.c for an example.

		The FAR versions of these functions are backwards compatible, meaning
		they accept near pointers and upcast them to FAR pointers.  There are
		two exceptions: strtol and strtod.  See the function descriptions of
		these for more information (place cursor on strtol or strtod and press
		CTRL+H).

******************************************************************************/

// Storing strings in far saves root data space.
const far char msg[] = "Hello World!";

char far far_buffer[20];
char near_buffer[20];

int main()
{
	static far long long_var;
	static far float float_var;
	static far int int_var;

	far char * ptr;

	// use %ls to print a string stored in FAR
	printf("This is the message stored in FAR: %ls\n\n", msg);

	_f_strcpy(far_buffer, msg);
	ptr = _f_strchr(far_buffer, ' ');
	_f_strcpy(ptr, " Rabbit!");

	printf("This is the new message stored in FAR: %ls\n\n", far_buffer);

	// near_buffer is upcast automatically.
	_f_memcpy(near_buffer, far_buffer, strlen(far_buffer) + 1);

	printf("This is the new message stored in NEAR: %s\n\n", near_buffer);

	if(strcmp(far_buffer, near_buffer) != 0) {
		printf("Error, strcmp says the strings are different!\n");
	}

	// Lets convert some numbers.
	_f_strcpy(far_buffer, "16E7B 3.141593 120");

	// ptr MUST be of type far char *
	long_var = _f_strtol(far_buffer, &ptr, 16);
	float_var = _f_strtod(ptr, &ptr);
	int_var = atoi(ptr);

	printf("%ls => %lX %f %d\n", far_buffer, long_var, float_var, int_var);
}

