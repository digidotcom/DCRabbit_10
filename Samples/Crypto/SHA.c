/*
 * Copyright (C) 2015 Digi International
 * Modified to include additional test vectors for SHA-1 (160-bit),
 * and SHA-224 (SHA-2, 224-bit), along with result timing.
 *
 * This sample demonstrates various secure hash algorithms with known
 * test vectors.
 */
/*
 *  FIPS-180-2 compliant SHA-256 implementation
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#use "sha1.lib"
#use "sha2.lib"

// set to 160 (SHA-1), 224 or 256 (SHA-2)
#define SHA_TEST 160

#if SHA_TEST == 160
	#define sha_test_context sha_state
	#define sha_test_init    sha_init
	#define sha_test_add     sha_add
	#define sha_test_finish  sha_finish
#elif SHA_TEST == 224
	#define sha_test_context sha224_context
	#define sha_test_init    sha224_init
	#define sha_test_add     sha224_add
	#define sha_test_finish  sha224_finish
#elif SHA_TEST == 256
	#define sha_test_context sha256_context
	#define sha_test_init    sha256_init
	#define sha_test_add     sha256_add
	#define sha_test_finish  sha256_finish
#else
	#error Must define SHA_TEST to 160 (SHA-1), 224 or 256 (SHA-2).
#endif

// Test vectors and digests from http://www.di-mgt.com.au/sha_testvectors.html

const char *vector[] =
{
	"abc",
	"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	"",
	NULL
};

const char *expected_digest[] =
{
#if SHA_TEST == 160
	"a9993e364706816aba3e25717850c26c9cd0d89d",
	"84983e441c3bd26ebaae4aa1f95129e5e54670f1",
	"da39a3ee5e6b4b0d3255bfef95601890afd80709",
	"34aa973cd4c4daa4f61eeb2bdbad27316534016f"
#elif SHA_TEST == 224
	"23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7",
	"75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525",
	"d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f",
	"20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67"
#else
	"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
	"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
	"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
	"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"
#endif
};

int main()
{
	int i, j;
	char output[65];
	sha_test_context context;
	unsigned char buf[1000];
	unsigned char digest[32];
	unsigned long T0;

	memset(buf, 'a', 1000);
	
	printf("SHA-%u Validation Tests:\n\n", SHA_TEST);
	for (i = 0; i < 4; ++i)
	{
		printf("Test %d: ", i + 1);

		T0 = MS_TIMER;
		sha_test_init(&context);

		if (vector[i] != NULL)
		{
			j = strlen(vector[i]);
			sha_test_add(&context, vector[i], j);
		}
		else
		{
			for(j = 0; j < 1000; ++j)
			{
				sha_test_add(&context, buf, 1000);
			}
		}

		sha_test_finish(&context, digest);
		T0 = MS_TIMER - T0;
		
		if (vector[i] != NULL)
		{
			printf("%u bytes in %lums ", j, T0);
		}
		else
		{
			printf("1,000,000 bytes in %lums ", T0);
		}

		// convert digest to a string
		for (j = 0; j < (SHA_TEST / 8); j++)
		{
			sprintf(output + j * 2, "%02x", digest[j]);
		}

		if (strcmp(output, expected_digest[i]))
		{
			printf("failed!\n" );
			printf("  expected: %s\n", expected_digest[i]);
			printf("calculated: %s\n", output[i]);
			return 1;
		}

		printf( "passed.\n" );
	}

	return 0;
}

