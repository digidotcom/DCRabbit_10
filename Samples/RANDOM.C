/*************************************************************************

	Samples\Random.c

	Rabbit Semiconductor, Inc. 2000

	This program generates and displays pseudo-random integers between 2000
	and 2999, inclusive, using the seed_init() and rand16_range() functions
	from RAND.LIB.  Each time a match value is selected the program reports
	the number of test values generated before the next occurrence of the
	selected match value is found in the sequence of range-limited pseudo-
	random integers.

	Please see the descriptive comments at the top of RAND.LIB for more
	information about its API functions and their suitability for different
	kinds of applications.  In particular, application developers concerned
	about data security should note the comments concerning the seed_hash()
	and seed_hash_secret() functions.

	Note that the randf(), randb() and randg() functions from MATH.LIB all
	return floating point values (not POSIX).

*************************************************************************/
#class auto

// All Dynamic C versions since 8.01 have included the MD5.LIB library, so
//  we can define RAND_USE_MD5 even though we don't demonstrate RAND.LIB's
//  seed_hash() functionality in this sample program.
#define RAND_USE_MD5

// uncomment the following line to make RAND.LIB functions debuggable
//#define RAND_DEBUG

#use "rand.lib"

void main ()
{
	auto char *salt;
	auto unsigned matchValue, testValue;
	auto unsigned long count;

	// initialize the count value to zero for the following comparison
	//  as well as for entry into the while loop later on . . .
	count = 0ul;

	if (memcmp(&SysIDBlock.macAddr[2], &count, sizeof(count))) {
		// if the System ID block contains an actual (non-zero) MAC address,
		//  then use it as our pseudo-random seed "salt" value
		salt = &SysIDBlock.macAddr[2];
	} else {
		// otherwise, don't bother with a psuedo-random seed "salt" value at all
		salt = NULL;
	}

	// NB: An application which requires a secure pseudo random number generator
	//     wouldn't (shouldn't!) call seed_init with anything other than a NULL
	//     "salt" parameter.
	seed_init(salt);

	while (1) {
		// get the pseudo-random integer sequence's next value from our range of
		//  [2000, 2999] inclusive
		testValue = rand16_range(2000u, 1000u);

		// catch and report any (impossible) errors here
		if (2000u > testValue || 2999u < testValue) {
			printf("\n\nError, %u is not in the range [2000, 2999] inclusive!\n\n",
			       testValue);
			exception(-ERR_RANGE);
			exit(-ERR_RANGE);
		}

		// display test and / or match values information
		if (count) {
			if (matchValue == testValue) {
				// found a match!
				printf("\n%8u  (Match after %lu pseudo-random sequence values.)\n",
				       testValue, count);
				printf("Press a key to continue.\n");
				while(!kbhit());	// wait for the User to press a key
				getchar();	// throw the keypress away and continue on . . .
				count = 0ul;	// reset the count for the next match test
			} else {
				// no match, just display the test value
				printf("%8u", testValue);
				++count;
			}
		} else {
			// count is zero so select a new match value to search for
			matchValue = testValue;
			printf("\n%8u  (Searching for the next occurrence of %u.)\n",
			       testValue, matchValue);
			++count;
		}
	}
}