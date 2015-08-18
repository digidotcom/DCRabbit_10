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
/**********************************************************************
	Samples\AES_Enncryption\aes_kat.c

	KAT (known answer tests)
	compare results with Rijndael reference

	The specification for the Rijndael cipher contains a set of known answer
	tests that are used to verify that an implementation of the cipher is
	correct. A known answer test consists of a block of plaintext, a key, and
	a corresponding block of cipher text. A proper implementation of the
	Rijndael cipher must generate the given ciphertext when encrypting the
	plaintext with the corresponding key.

	The following sample runs three tests each for 128, 192, and 256 bit keys.
	The plaintext is encrypted, checked against the given ciphertext, and then
	decrypted and checked against the original plaintext.

***********************************************************************/
#class auto

#use "aes_crypt.lib"

//128-bit key known answer tests
const char *kat128_pt[3] = {
	"506812A45F08C889B97F5980038B8359",
	"5C6D71CA30DE8B8B00549984D2EC7D4B",
	"53F3F4C64F8616E4E7C56199F48F21F6" };

const char *kat128_key[3] = {
	"00010203050607080A0B0C0D0F101112",
	"14151617191A1B1C1E1F202123242526",
	"28292A2B2D2E2F30323334353738393A" };

const char *kat128_ct[3] = {
	"D8F532538289EF7D06B506A4FD5BE9C9",
	"59AB30F4D4EE6E4FF9907EF65B1FB68C",
	"BF1ED2FCB2AF3FD41443B56D85025CB1" };


//192-bit key known answer tests
const char *kat192_pt[3] = {
	"2D33EEF2C0430A8A9EBF45E809C40BB6",
	"6AA375D1FA155A61FB72353E0A5A8756",
	"BC3736518B9490DCB8ED60EB26758ED4" };

const char *kat192_key[3] = {
	"00010203050607080A0B0C0D0F10111214151617191A1B1C",
	"1E1F20212324252628292A2B2D2E2F30323334353738393A",
	"3C3D3E3F41424344464748494B4C4D4E5051525355565758" };

const char *kat192_ct[3] = {
	"DFF4945E0336DF4C1C56BC700EFF837F",
	"B6FDDEF4752765E347D5D2DC196D1252",
	"D23684E3D963B3AFCF1A114ACA90CBD6" };


//256-bit key known answer tests
const char *kat256_pt[3] = {
	"834EADFCCAC7E1B30664B1ABA44815AB",
	"D9DC4DBA3021B05D67C0518F72B62BF1",
	"A291D86301A4A739F7392173AA3C604C" };

const char *kat256_key[3] = {
	"00010203050607080A0B0C0D0F10111214151617191A1B1C1E1F202123242526",
	"28292A2B2D2E2F30323334353738393A3C3D3E3F41424344464748494B4C4D4E",
	"50515253555657585A5B5C5D5F60616264656667696A6B6C6E6F707173747576" };

const char *kat256_ct[3] = {
	"1946DABF6A03A2A2C3D0B05080AED6FC",
	"5ED301D747D3CC715445EBDEC62F2FB4",
	"6585C8F43D13A6BEAB6419FC5935B9D0" };

//helper function to turn hex strings into byte arrays
void convert_hex(const char *hex, char *data, int bytes)
{
	auto int i;
	auto char digit_string[3];

	digit_string[2] = 0; //NULL terminator
	for(i = 0;i < bytes;i++)
	{
		memcpy(digit_string, hex + 2*i, 2);
		data[i] = (char)(strtol(digit_string, NULL, 16) & 0xff);
	}
}

//helper function for printing byte arrays in hex format
void hexprint(const char *input, int insize)
{
	auto int i;
	auto int value;

	for(i = 0;i < insize;i++)
	{
		value = input[i] & 0xff;
		printf( "%02x", value );
	}
	printf("\n");
}


void main()
{
	static int i;
	static char expanded_key[240]; //max expanded key is 4*4*15=240
	static char key[32];
	static char pt[16];
	static char ct[16];
	static char expected[16];

	//do 128-bit tests
	printf("test with Nb=4, Nk=4:\n");
	for(i = 0;i < 3;i++)
	{
		convert_hex(kat128_key[i], key, 16);
		convert_hex(kat128_pt[i], pt, 16);

		//block functions need to use expanded key
		//cipher uses 10 rounds for 128-bit key
		AESexpandKey(expanded_key, key, 4, 4);

		printf("PT %d=\n", i+1);
		hexprint(pt, 16);
		memcpy(ct, pt, 16);
		AESencrypt(ct, expanded_key, 4, 4);
		printf("CT %d=\n", i+1);
		hexprint(ct, 16);
		printf("expected=\n");
		convert_hex(kat128_ct[i], expected, 16);
		hexprint(expected, 16);
		if(memcmp(expected, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		AESdecrypt(ct, expanded_key, 4, 4);
		printf("decrypt... PT %d=\n", i+1);
		hexprint(ct, 16);
		if(memcmp(pt, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		printf("\n");
	}

	printf("test with Nb=4, Nk=6:\n");
	for(i = 0;i < 3;i++)
	{
		convert_hex(kat192_key[i], key, 24);
		convert_hex(kat192_pt[i], pt, 16);

		//block functions need to use expanded key
		//cipher uses 12 rounds for 192-bit key
		AESexpandKey(expanded_key, key, 4, 6);

		printf("PT %d=\n", i+1);
		hexprint(pt, 16);
		memcpy(ct, pt, 16);
		AESencrypt(ct, expanded_key, 4, 6);
		printf("CT %d=\n", i+1);
		hexprint(ct, 16);
		printf("expected=\n");
		convert_hex(kat192_ct[i], expected, 16);
		hexprint(expected, 16);
		if(memcmp(expected, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		AESdecrypt(ct, expanded_key, 4, 6);
		printf("decrypt... PT %d=\n", i+1);
		hexprint(ct, 16);
		if(memcmp(pt, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		printf("\n");
	}

	printf("test with Nb=4, Nk=8:\n");
	for(i = 0;i < 3;i++)
	{
		convert_hex(kat256_key[i], key, 32);
		convert_hex(kat256_pt[i], pt, 16);

		//block functions need to use expanded key
		//cipher uses 14 rounds for 256-bit key
		AESexpandKey(expanded_key, key, 4, 8);

		printf("PT %d=\n", i+1);
		hexprint(pt, 16);
		memcpy(ct, pt, 16);
		AESencrypt(ct, expanded_key, 4, 8);
		printf("CT %d=\n", i+1);
		hexprint(ct, 16);
		printf("expected=\n");
		convert_hex(kat256_ct[i], expected, 16);
		hexprint(expected, 16);
		if(memcmp(expected, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		AESdecrypt(ct, expanded_key, 4, 8);
		printf("decrypt... PT %d=\n", i+1);
		hexprint(ct, 16);
		if(memcmp(pt, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		printf("\n");
	}

}