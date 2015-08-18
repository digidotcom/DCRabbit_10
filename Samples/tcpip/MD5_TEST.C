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
/*
 *   SAMPLES\TCPIP\md5_test.c
 *
 *   Simple demonstration of MD5 hashing library.
 *   MD5 takes any amount of data and produces a 16-byte hash value of it.
 *
 *   This demo will show how to generate a hash with the library functions
 *   Even though strings are used, MD5 will hash any data given to it.
 */
#class auto

#use "md5.lib"

const char string_a[] = "Buy low, sell high.";
const char string_b[] = "Buy low, sell high?";

md5_state_t hash_state;
char hash[16];

void hexprint(char *data, int len)
{
	auto int i;
	for(i = 0;i < len;i++)
	{
   	/* "%02x" for lowercase, "%02X" for uppercase hexadecimal letters */
		printf("%02x", data[i]);
	}
}

void main()
{
	md5_init(&hash_state);	//prepare for a new hash
	md5_append(&hash_state, string_a, strlen(string_a));
	md5_finish(&hash_state, hash); //calculate hash value
	printf("%s\n", string_a);
	printf("--hashes to---\n");
	hexprint(hash, 16);
	printf("\n");

	md5_init(&hash_state);	//prepare for a new hash
	md5_append(&hash_state, string_b, strlen(string_b));
	md5_finish(&hash_state, hash); //calculate hash value
	printf("%s\n", string_b);
	printf("--hashes to---\n");
	hexprint(hash, 16);
	printf("\n");
}