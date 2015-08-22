/*
 *   SAMPLES\TCPIP\md5_test.c
 *   Copyright 2007, Rabbit Semiconductor
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