/*
 * Copyright (C) 2015 Digi International
 * Modified to include additional test vectors for SHA-1 (160-bit) and
 * SHA-224/256/384/512 (SHA-2, 224/256/384/512-bit), along with result timing.
 *
 * This sample demonstrates various secure hash algorithms with known
 * test vectors.
 *
 * Benchmark of hashing 1,000,000 bytes:
 *
 *           RCM6700
 * sha1      1,729ms
 * sha224    7,657ms
 * sha256    7,283ms
 * sha384   30,291ms
 * sha512   30,359ms
 *
 * sha384/sha512 currently slow due to limited assembly optimizations and
 * use of emulated 64-bit values in hashing algorithm.
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

// set to 160 (SHA-1), 224, 256, 384 or 512 (SHA-2)
#ifndef SHA_TEST
	#define SHA_TEST 256
#endif

#if SHA_TEST == 160
	#define sha_test_context sha_state
	#define sha_test_init    sha_init
	#define sha_test_add     sha_add
	#define sha_test_finish  sha_finish
	#define DIGEST_LENGTH    SHA_HASH_SIZE
#elif SHA_TEST == 224
	#define sha_test_context sha224_context
	#define sha_test_init    sha224_init
	#define sha_test_add     sha224_add
	#define sha_test_finish  sha224_finish
	#define DIGEST_LENGTH    SHA224_LENGTH
#elif SHA_TEST == 256
	#define sha_test_context sha256_context
	#define sha_test_init    sha256_init
	#define sha_test_add     sha256_add
	#define sha_test_finish  sha256_finish
	#define DIGEST_LENGTH    SHA256_LENGTH
#elif SHA_TEST == 384
	#define sha_test_context sha384_context
	#define sha_test_init    sha384_init
	#define sha_test_add     sha384_add
	#define sha_test_finish  sha384_finish
	#define DIGEST_LENGTH    SHA384_LENGTH
#elif SHA_TEST == 512
	#define sha_test_context sha512_context
	#define sha_test_init    sha512_init
	#define sha_test_add     sha512_add
	#define sha_test_finish  sha512_finish
	#define DIGEST_LENGTH    SHA512_LENGTH
#else
	#error Must define SHA_TEST to 160 (SHA-1), 224, 256, 384, or 512 (SHA-2).
#endif

#if SHA_TEST == 160
	#use "sha1.lib"
#elif SHA_TEST <= 256
	#use "sha2.lib"
#else
	#use "sha512.lib"
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
#elif SHA_TEST == 256
	"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
	"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
	"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
	"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"
#elif SHA_TEST == 384
	"cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7",
	"3391fdddfc8dc7393707a65b1b4709397cf8b1d162af05abfe8f450de5f36bc6b0455a8520bc4e6f5fe95b1fe3c8452b",
	"38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b",
	"9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985"
#elif SHA_TEST == 512
	"ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f",
	"204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445",
	"cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e",
	"e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b"
#endif
};

int main()
{
	int i, j;
	char output[DIGEST_LENGTH * 2 + 1];
	sha_test_context context;
	unsigned char buf[1000];
	unsigned char digest[DIGEST_LENGTH];
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

