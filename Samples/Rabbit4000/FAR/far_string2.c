/*****************************************************************************

     far_string2.c
     Z-World, 2006

		This sample is the same as far_string.c except with the functions
		renamed to assume the macro USE_FAR_STRING_LIB is defined.  Define this
		macro in your project options defines box and run this program.

******************************************************************************/

#ifndef USE_FAR_STRING_LIB
#error "Define USE_FAR_STRING_LIB in your project-options define box."
#endif

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

	strcpy(far_buffer, msg);
	ptr = strchr(far_buffer, ' ');
	strcpy(ptr, " Z-World!");

	printf("This is the new message stored in FAR: %ls\n\n", far_buffer);

	// near_buffer is upcast automatically.
	memcpy(near_buffer, far_buffer, strlen(far_buffer) + 1);

	printf("This is the new message stored in NEAR: %s\n\n", near_buffer);

	if(strcmp(far_buffer, near_buffer) != 0) {
		printf("Error, strcmp says the strings are different!\n");
	}

	// Lets convert some numbers.
	strcpy(far_buffer, "16E7B 3.141593 120");

	// ptr MUST be of type far char *
	long_var = strtol(far_buffer, &ptr, 16);
	float_var = strtod(ptr, &ptr);
	int_var = atoi(ptr);

	printf("%ls => %lX %f %d\n", far_buffer, long_var, float_var, int_var);
}

