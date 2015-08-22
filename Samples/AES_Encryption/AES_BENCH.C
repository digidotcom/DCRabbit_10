/**********************************************************************
	Samples\AES_Enncryption\aes_bench.c
	Zworld, 2005

   This is the same as aes_kat.c, plus does benchmarking for speed of
   8192 byte blocks.

	KAT (known answer tests)
	compare results with Rijndael reference


***********************************************************************/
#class auto

//#define NO_KAT			// Define to bypass KAT, just do benchmark
#define AES_ONLY		// Define to use only official AES variants of Rijndael
							// (saves code).
//#define AES_DEBUG

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


// Benchmark block
#define BBSIZE	8192
#define BBNUM	4		// Number of repeats per test
char bblock[BBSIZE];
AESstreamState ass;

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


void main()
{
	static int i;
	static char expanded_key[240]; //max expanded key is 4*4*15=240
	static char key[32];
	static char pt[16];
	static char ct[16];
	static char expected[16];
	static char tag[16];
   long tstart;
   long tend;
   word j;


#ifndef NO_KAT
	//do 128-bit tests
	printf("========================== test with Nb=4, Nk=4 (near):\n");
	for(i = 0;i < 3;i++)
	{
		convert_hex(kat128_key[i], key, 16);
		convert_hex(kat128_pt[i], pt, 16);

		//block functions need to use expanded key
		//cipher uses 10 rounds for 128-bit key
		AESexpandKey(expanded_key, key, 4, 4);

		printf("PT %d=\n", i+1);
		mem_dump(pt, 16);
		memcpy(ct, pt, 16);
		AESencrypt(ct, expanded_key, 4, 4);
		printf("CT %d=\n", i+1);
		mem_dump(ct, 16);
		printf("expected=\n");
		convert_hex(kat128_ct[i], expected, 16);
		mem_dump(expected, 16);
		if(memcmp(expected, ct, 16))
		{
			printf("ERROR: KAT test failed near 4x4 encrypt\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		AESdecrypt4(ct, expanded_key, 4);
		printf("decrypt... PT %d=\n", i+1);
		mem_dump(ct, 16);
		if(memcmp(pt, ct, 16))
		{
			printf("ERROR: KAT test failed near 4x4 decrypt\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		printf("\n");
	}

	printf("========================== test with Nb=4, Nk=4 (far):\n");
	for(i = 0;i < 3;i++)
	{
		convert_hex(kat128_key[i], key, 16);
		convert_hex(kat128_pt[i], pt, 16);

		//block functions need to use expanded key
		//cipher uses 10 rounds for 128-bit key
		AESexpandKey4(expanded_key, key);

		printf("PT %d=\n", i+1);
		mem_dump(pt, 16);
		memcpy(ct, pt, 16);
		AESencrypt4x4(expanded_key, ct, ct);
		printf("CT %d=\n", i+1);
		mem_dump(ct, 16);
		printf("expected=\n");
		convert_hex(kat128_ct[i], expected, 16);
		mem_dump(expected, 16);
		if(memcmp(expected, ct, 16))
		{
			printf("ERROR: KAT test failed far 4x4 encrypt\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		AESdecrypt4x4(expanded_key, ct, ct);
		printf("decrypt... PT %d=\n", i+1);
		mem_dump(ct, 16);
		if(memcmp(pt, ct, 16))
		{
			printf("ERROR: KAT test failed far 4x4 decrypt\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		printf("\n");
	}

	printf("========================== test with Nb=4, Nk=6:\n");
	for(i = 0;i < 3;i++)
	{
		convert_hex(kat192_key[i], key, 24);
		convert_hex(kat192_pt[i], pt, 16);

		//block functions need to use expanded key
		//cipher uses 12 rounds for 192-bit key
		AESexpandKey(expanded_key, key, 4, 6);

		printf("PT %d=\n", i+1);
		mem_dump(pt, 16);
		memcpy(ct, pt, 16);
		AESencrypt(ct, expanded_key, 4, 6);
		printf("CT %d=\n", i+1);
		mem_dump(ct, 16);
		printf("expected=\n");
		convert_hex(kat192_ct[i], expected, 16);
		mem_dump(expected, 16);
		if(memcmp(expected, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		AESdecrypt4(ct, expanded_key, 6);
		printf("decrypt... PT %d=\n", i+1);
		mem_dump(ct, 16);
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

	printf("========================== test with Nb=4, Nk=8:\n");
	for(i = 0;i < 3;i++)
	{
		convert_hex(kat256_key[i], key, 32);
		convert_hex(kat256_pt[i], pt, 16);

		//block functions need to use expanded key
		//cipher uses 14 rounds for 256-bit key
		AESexpandKey(expanded_key, key, 4, 8);

		printf("PT %d=\n", i+1);
		mem_dump(pt, 16);
		memcpy(ct, pt, 16);
		AESencrypt(ct, expanded_key, 4, 8);
		printf("CT %d=\n", i+1);
		mem_dump(ct, 16);
		printf("expected=\n");
		convert_hex(kat256_ct[i], expected, 16);
		mem_dump(expected, 16);
		if(memcmp(expected, ct, 16))
		{
			printf("ERROR: KAT test failed\n");
			exit(-1);
		}
		else
		{
			printf("OK\n");
		}
		AESdecrypt4(ct, expanded_key, 8);
		printf("decrypt... PT %d=\n", i+1);
		mem_dump(ct, 16);
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
#endif

	// Now do benchmarks
	memset(bblock, 'A', sizeof(bblock));
	printf("\nBenchmarks:\n");
   printf("type\tnb\tnk\tms\tbyte/sec\n");
   printf("------- ------- ------- ------- ---------\n");
	convert_hex(kat128_key[0], key, 16);
	AESexpandKey(expanded_key, key, 4, 4);
   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
			AESencrypt(bblock+j, expanded_key, 4, 4);
   tend = MS_TIMER;
	printf("Encr\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
		AESdecrypt4(bblock+j, expanded_key, 4);
   tend = MS_TIMER;
	printf("Decr\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   // Verify back to start
   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }

   // Using far 4/4
	memset(bblock, 'A', sizeof(bblock));
   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
			AESencrypt4x4(expanded_key, bblock+j, bblock+j);
   tend = MS_TIMER;
	printf("Encrfar\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
			AESdecrypt4x4(expanded_key, bblock+j, bblock+j);
   tend = MS_TIMER;
	printf("Decrfar\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   // Verify back to start
   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }


   // Using CBC mode 4x4
	memset(bblock, 'A', sizeof(bblock));
   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
		aes_128_cbc_encrypt(kat128_key[0], "XXXXXXXXXXXXXXXX", bblock, BBSIZE);
   tend = MS_TIMER;
	printf("EncrCBC\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
		aes_128_cbc_decrypt(kat128_key[0], "XXXXXXXXXXXXXXXX", bblock, BBSIZE);
   tend = MS_TIMER;
	printf("DecrCBC\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   // Verify back to start
   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }



	memset(bblock, 'A', sizeof(bblock));
	convert_hex(kat192_key[0], key, 24);
	AESexpandKey(expanded_key, key, 4, 6);
   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
			AESencrypt(bblock+j, expanded_key, 4, 6);
   tend = MS_TIMER;
	printf("Encr\t4\t6\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
		AESdecrypt4(bblock+j, expanded_key, 6);
   tend = MS_TIMER;
	printf("Decr\t4\t6\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   // Verify back to start
   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }

	memset(bblock, 'A', sizeof(bblock));
	convert_hex(kat256_key[0], key, 32);
	AESexpandKey(expanded_key, key, 4, 8);
   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
			AESencrypt(bblock+j, expanded_key, 4, 8);
   tend = MS_TIMER;
	printf("Encr\t4\t8\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   tstart = MS_TIMER;
	for(i = 0;i < BBNUM;i++)
   	for (j = 0; j < BBSIZE; j += 16)
		AESdecrypt4(bblock+j, expanded_key, 8);
   tend = MS_TIMER;
	printf("Decr\t4\t8\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE*BBNUM/(tend-tstart));

   // Verify back to start
   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }

   // Using optimized stream encryption
	memset(bblock, 'A', sizeof(bblock));
	convert_hex(kat128_key[0], key, 16);
   AESinitStream(&ass, key, key);
   tstart = MS_TIMER;
	//for(i = 0;i < BBNUM;i++)
		AESencryptStream(&ass, bblock, sizeof(bblock));
   tend = MS_TIMER;
	printf("StreamE\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE/(tend-tstart));

   AESinitStream(&ass, key, key);
   tstart = MS_TIMER;
	//for(i = 0;i < BBNUM;i++)
		AESdecryptStream(&ass, bblock, sizeof(bblock));
   tend = MS_TIMER;
	printf("StreamD\t4\t4\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE/(tend-tstart));


   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }

	memset(bblock, 'A', sizeof(bblock));
	convert_hex(kat192_key[0], key, 24);
   AESinitStream192(&ass, key, key);
   tstart = MS_TIMER;
	//for(i = 0;i < BBNUM;i++)
		AESencryptStream(&ass, bblock, sizeof(bblock));
   tend = MS_TIMER;
	printf("StreamE\t4\t6\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE/(tend-tstart));

   AESinitStream192(&ass, key, key);
   tstart = MS_TIMER;
	//for(i = 0;i < BBNUM;i++)
		AESdecryptStream(&ass, bblock, sizeof(bblock));
   tend = MS_TIMER;
	printf("StreamD\t4\t6\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE/(tend-tstart));


   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }

	memset(bblock, 'A', sizeof(bblock));
	convert_hex(kat256_key[0], key, 32);
   AESinitStream256(&ass, key, key);
   tstart = MS_TIMER;
	//for(i = 0;i < BBNUM;i++)
		AESencryptStream(&ass, bblock, sizeof(bblock));
   tend = MS_TIMER;
	printf("StreamE\t4\t8\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE/(tend-tstart));

   AESinitStream256(&ass, key, key);
   tstart = MS_TIMER;
	//for(i = 0;i < BBNUM;i++)
		AESdecryptStream(&ass, bblock, sizeof(bblock));
   tend = MS_TIMER;
	printf("StreamD\t4\t8\t%lu\t%lu\n",
   	tend-tstart, 1000L*BBSIZE/(tend-tstart));


   for (i = 0; i < BBSIZE; ++i)
   	if (bblock[i] != 'A') {
      	printf("***ERROR*** decryption failed at offset %u\n", i);
         break;
      }
}