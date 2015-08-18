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
/********************************************************************
   userblock_sample.c

	This program demonstrates the use of the writeUserBlockArray()
	and readUserBlockArray() functions.  writeUserBlockArray() allows
	the writing of sets of data to the user block at once.  This is
	particularly useful for mirrored user blocks when you need a
	coherent snapshot of data that is scattered across memory.  If
	you used writeUserBlock() in a loop over the data, you would not
	have a coherent snapshot if a power cycle happened in the middle
	of writing.  writeUserBlockArray(), however, does not validate
	the new data until it has all been completely written, hence
	guaranteeing a coherent snapshot even if a power cycle happens
	in the middle of the write.

********************************************************************/
#class auto

#use "idblock_api.lib"

/* Set the size of the test string */
#define TEST_STRING_LEN		20

void main(void)
{
	/* Create variables for our test data */
	struct test_struct {
		int foo;
		long bar;
	} test_data;
	long test_long;
	char test_string[TEST_STRING_LEN];

	/*
	 * Create arrays to hold pointers to the data we want to save, as well
	 * as the lengths
	 */
	void* save_data[3];
	unsigned int save_lens[3];

	/* Initialize the test data */
	test_data.foo = 12;
	test_data.bar = 34;
	test_long = 5678;
	strcpy(test_string, "Hello!");

	/* Print out what we are saving */
	printf("Saving...\n");
	printf("test_data.foo = %d\n", test_data.foo);
	printf("test_data.bar = %ld\n", test_data.bar);
	printf("test_long = %ld\n", test_long);
	printf("test_string = %s\n", test_string);

	/* Save the data to the user block */
	save_data[0] = &test_data;
	save_lens[0] = sizeof(test_data);
	save_data[1] = &test_long;
	save_lens[1] = sizeof(test_long);
	save_data[2] = test_string;
	save_lens[2] = TEST_STRING_LEN;
	writeUserBlockArray(0, (const void * const *) save_data, save_lens, 3);

	/*
	 * Clear our variables (to ensure that when we read the data back, it
	 * is not correct simply because nothing was read).
	 */
	test_data.foo = 0;
	test_data.bar = 0;
	test_long = 0;
	strcpy(test_string, "Not correct!");

	/*
	 * Read back our saved values (note that you could also just use
	 * readUserBlock() in a loop)
	 */
	readUserBlockArray(save_data, save_lens, 3, 0);

	/* Print out what we are loading */
	printf("\nLoading...\n");
	printf("test_data.foo = %d\n", test_data.foo);
	printf("test_data.bar = %ld\n", test_data.bar);
	printf("test_long = %ld\n", test_long);
	printf("test_string = %s\n", test_string);
}