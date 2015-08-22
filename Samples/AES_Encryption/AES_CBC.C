/**********************************************************************
	Samples\AES_Enncryption\AES_CBC.c
	Rabbit Semiconductor, 2006

   This sample runs the test cases for AES CBC (Cipher-block Chaining)
   from RFC3602, both forward (encryption) and backward (decryption).
   Unlike AES_KAT.C, the decryption step is separate from the encrryption
   step.  For each case, the expected and calculated results are printed.

   Note that unlike the default CFB streams, CBC streams may not contain
   partial blocks: all cipher and plaintext blocks are multiples of the
   block size.  Smaller blocks must be padded.  The only block size
   supported by the current implementation is 16 bytes.

   The sample cases are

      case 1: one 16-byte (128 bit) block is encrypted or decrypted
      case 2: two 16-byte blocks (32 bytes) are encrypted / decrypted
      case 3: three 16-byte blocks (48 bytes) are encrypted / decrypted
      case 4: four 16-byte blocks (64 bytes) are encrypted / decrypted

***********************************************************************/

#use AES_CRYPT.LIB
#define AES_CBC_BLOCK_SIZE 16

/* Beginning of constants */
/*
   Do some testing.  RFC3602 has some test vectors:
Case #1: Encrypting 16 bytes (1 block) using AES-CBC with 128-bit key
Key       : 0x06a9214036b8a15b512e03d534120006
IV        : 0x3dafba429d9eb430b422da802c9fac41
Plaintext : "Single block msg"
Ciphertext: 0xe353779c1079aeb82708942dbe77181a
*/
// s/\(..\)/'\\x\1', /g
const char key1[AES_CBC_BLOCK_SIZE] = {
   '\x06', '\xa9', '\x21', '\x40', '\x36', '\xb8', '\xa1', '\x5b',
   '\x51', '\x2e', '\x03', '\xd5', '\x34', '\x12', '\x00', '\x06',
};
const char iv1[AES_CBC_BLOCK_SIZE] = {
   '\x3d', '\xaf', '\xba', '\x42', '\x9d', '\x9e', '\xb4', '\x30',
   '\xb4', '\x22', '\xda', '\x80', '\x2c', '\x9f', '\xac', '\x41',
};
const char plntxt1[AES_CBC_BLOCK_SIZE * 1] =
{
   //"Single block msg" (no trailing '\0');
   'S', 'i', 'n', 'g', 'l', 'e', ' ', 'b',
   'l', 'o', 'c', 'k', ' ', 'm', 's', 'g',
};
const char cyptxt1[AES_CBC_BLOCK_SIZE * 1] = {
   '\xe3', '\x53', '\x77', '\x9c', '\x10', '\x79', '\xae', '\xb8',
   '\x27', '\x08', '\x94', '\x2d', '\xbe', '\x77', '\x18', '\x1a',
};

/*
Case #2: Encrypting 32 bytes (2 blocks) using AES-CBC with 128-bit key
Key       : 0xc286696d887c9aa0611bbb3e2025a45a
IV        : 0x562e17996d093d28ddb3ba695a2e6f58
Plaintext : 0x000102030405060708090a0b0c0d0e0f
              101112131415161718191a1b1c1d1e1f
Ciphertext: 0xd296cd94c2cccf8a3a863028b5e1dc0a
              7586602d253cfff91b8266bea6d61ab1
*/
const char key2[AES_CBC_BLOCK_SIZE] = {
   '\xc2', '\x86', '\x69', '\x6d', '\x88', '\x7c', '\x9a', '\xa0',
   '\x61', '\x1b', '\xbb', '\x3e', '\x20', '\x25', '\xa4', '\x5a',
};
const char iv2[AES_CBC_BLOCK_SIZE] = {
   '\x56', '\x2e', '\x17', '\x99', '\x6d', '\x09', '\x3d', '\x28',
   '\xdd', '\xb3', '\xba', '\x69', '\x5a', '\x2e', '\x6f', '\x58',
};
const char plntxt2[AES_CBC_BLOCK_SIZE * 2] = {
   '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
   '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
   '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
   '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
};
const char cyptxt2[AES_CBC_BLOCK_SIZE * 2] = {
   '\xd2', '\x96', '\xcd', '\x94', '\xc2', '\xcc', '\xcf', '\x8a',
   '\x3a', '\x86', '\x30', '\x28', '\xb5', '\xe1', '\xdc', '\x0a',
   '\x75', '\x86', '\x60', '\x2d', '\x25', '\x3c', '\xff', '\xf9',
   '\x1b', '\x82', '\x66', '\xbe', '\xa6', '\xd6', '\x1a', '\xb1',
};

/*
Frankel, et al.             Standards Track                     [Page 6]

RFC 3602        AES-CBC Cipher Algorithm Use with IPsec   September 2003


Case #3: Encrypting 48 bytes (3 blocks) using AES-CBC with 128-bit key
Key       : 0x6c3ea0477630ce21a2ce334aa746c2cd
IV        : 0xc782dc4c098c66cbd9cd27d825682c81
Plaintext : "This is a 48-byte message (exactly 3 AES blocks)"
Ciphertext: 0xd0a02b3836451753d493665d33f0e886
              2dea54cdb293abc7506939276772f8d5
              021c19216bad525c8579695d83ba2684
*/
const char key3[AES_CBC_BLOCK_SIZE] = {
   '\x6c', '\x3e', '\xa0', '\x47', '\x76', '\x30', '\xce', '\x21', '\xa2',
   '\xce', '\x33', '\x4a', '\xa7', '\x46', '\xc2', '\xcd',
};
const char iv3[AES_CBC_BLOCK_SIZE] = {
   '\xc7', '\x82', '\xdc', '\x4c', '\x09', '\x8c', '\x66', '\xcb', '\xd9',
   '\xcd', '\x27', '\xd8', '\x25', '\x68', '\x2c', '\x81',
};
// :s/\(.\)/'\1', /g
const char plntxt3[AES_CBC_BLOCK_SIZE * 3] = {
   'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', '4', '8', '-', 'b',
   'y', 't', 'e', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', ' ', '(', 'e',
   'x', 'a', 'c', 't', 'l', 'y', ' ', '3', ' ', 'A', 'E', 'S', ' ', 'b',
   'l', 'o', 'c', 'k', 's', ')',
};
const char cyptxt3[AES_CBC_BLOCK_SIZE * 3] = {
   '\xd0', '\xa0', '\x2b', '\x38', '\x36', '\x45', '\x17', '\x53', '\xd4',
   '\x93', '\x66', '\x5d', '\x33', '\xf0', '\xe8', '\x86', '\x2d', '\xea',
   '\x54', '\xcd', '\xb2', '\x93', '\xab', '\xc7', '\x50', '\x69', '\x39',
   '\x27', '\x67', '\x72', '\xf8', '\xd5', '\x02', '\x1c', '\x19', '\x21',
   '\x6b', '\xad', '\x52', '\x5c', '\x85', '\x79', '\x69', '\x5d', '\x83',
   '\xba', '\x26', '\x84',
};

/*
Case #4: Encrypting 64 bytes (4 blocks) using AES-CBC with 128-bit key
Key       : 0x56e47a38c5598974bc46903dba290349
IV        : 0x8ce82eefbea0da3c44699ed7db51b7d9
Plaintext : 0xa0a1a2a3a4a5a6a7a8a9aaabacadaeaf
              b0b1b2b3b4b5b6b7b8b9babbbcbdbebf
              c0c1c2c3c4c5c6c7c8c9cacbcccdcecf
              d0d1d2d3d4d5d6d7d8d9dadbdcdddedf
Ciphertext: 0xc30e32ffedc0774e6aff6af0869f71aa
              0f3af07a9a31a9c684db207eb0ef8e4e
              35907aa632c3ffdf868bb7b29d3d46ad
              83ce9f9a102ee99d49a53e87f4c3da55
*/
const char key4[AES_CBC_BLOCK_SIZE] = {
   '\x56', '\xe4', '\x7a', '\x38', '\xc5', '\x59', '\x89', '\x74', '\xbc',
   '\x46', '\x90', '\x3d', '\xba', '\x29', '\x03', '\x49', };
const char iv4[AES_CBC_BLOCK_SIZE] = {
   '\x8c', '\xe8', '\x2e', '\xef', '\xbe', '\xa0', '\xda', '\x3c', '\x44',
   '\x69', '\x9e', '\xd7', '\xdb', '\x51', '\xb7', '\xd9', };
const char plntxt4[AES_CBC_BLOCK_SIZE * 4] = {
   '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6', '\xa7', '\xa8',
   '\xa9', '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf', '\xb0', '\xb1',
   '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7', '\xb8', '\xb9', '\xba',
   '\xbb', '\xbc', '\xbd', '\xbe', '\xbf', '\xc0', '\xc1', '\xc2', '\xc3',
   '\xc4', '\xc5', '\xc6', '\xc7', '\xc8', '\xc9', '\xca', '\xcb', '\xcc',
   '\xcd', '\xce', '\xcf', '\xd0', '\xd1', '\xd2', '\xd3', '\xd4', '\xd5',
   '\xd6', '\xd7', '\xd8', '\xd9', '\xda', '\xdb', '\xdc', '\xdd', '\xde',
   '\xdf',
};
const char cyptxt4[AES_CBC_BLOCK_SIZE * 4] = {
   '\xc3', '\x0e', '\x32', '\xff', '\xed', '\xc0', '\x77', '\x4e', '\x6a',
   '\xff', '\x6a', '\xf0', '\x86', '\x9f', '\x71', '\xaa', '\x0f', '\x3a',
   '\xf0', '\x7a', '\x9a', '\x31', '\xa9', '\xc6', '\x84', '\xdb', '\x20',
   '\x7e', '\xb0', '\xef', '\x8e', '\x4e', '\x35', '\x90', '\x7a', '\xa6',
   '\x32', '\xc3', '\xff', '\xdf', '\x86', '\x8b', '\xb7', '\xb2', '\x9d',
   '\x3d', '\x46', '\xad', '\x83', '\xce', '\x9f', '\x9a', '\x10', '\x2e',
   '\xe9', '\x9d', '\x49', '\xa5', '\x3e', '\x87', '\xf4', '\xc3', '\xda',
   '\x55',
};

/* End of constants */

int main(void) {
   auto int i;
   auto char text[256];
   auto AESstreamState state;

   // Test case 1 - encrypt
   printf("Test case 1 - encrypt \n");
   AESinitStream(&state, key1, iv1);

   memcpy(text, plntxt1, sizeof(plntxt1));
   AESencryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE);
   printf("Encrypted text:\n");
   for (i = 0; i < sizeof(plntxt1); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(cyptxt1); i++) {
      printf("%02x.", (int) cyptxt1[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   // Test case 2 - encrypt
   printf("Test case 2 - encrypt \n");
   AESinitStream(&state, key2, iv2);

   memcpy(text, plntxt2, sizeof(plntxt2));
   AESencryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE * 2);
   printf("Encrypted text:\n");
   for (i = 0; i < sizeof(plntxt2); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(cyptxt2); i++) {
      printf("%02x.", (int) cyptxt2[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   // Test case 3 - encrypt
   printf("Test case 3 - encrypt \n");
   AESinitStream(&state, key3, iv3);

   memcpy(text, plntxt3, sizeof(plntxt3));
   AESencryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE * 3);
   printf("Encrypted text:\n");
   for (i = 0; i < sizeof(plntxt3); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(cyptxt3); i++) {
      printf("%02x.", (int) cyptxt3[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   // Test case 4 - encrypt
   printf("Test case 4 - encrypt \n");
   AESinitStream(&state, key4, iv4);

   memcpy(text, plntxt4, sizeof(plntxt4));
   AESencryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE * 4);
   printf("Encrypted text:\n");
   for (i = 0; i < sizeof(plntxt4); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(cyptxt4); i++) {
      printf("%02x.", (int) cyptxt4[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   // Test case 1 - decrypt
   printf("Test case 1 - decrypt \n");
   AESinitStream(&state, key1, iv1);

   memcpy(text, cyptxt1, sizeof(cyptxt1));
   AESdecryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE);
   printf("Decrypted text:\n");
   for (i = 0; i < sizeof(cyptxt1); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(plntxt1); i++) {
      printf("%02x.", (int) plntxt1[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   // Test case 2 - decrypt
   printf("Test case 2 - decrypt \n");
   AESinitStream(&state, key2, iv2);

   memcpy(text, cyptxt2, sizeof(cyptxt2));
   AESdecryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE * 2);
   printf("Decrypted text:\n");
   for (i = 0; i < sizeof(cyptxt2); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(plntxt2); i++) {
      printf("%02x.", (int) plntxt2[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   // Test case 3 - decrypt
   printf("Test case 3 - decrypt \n");
   AESinitStream(&state, key3, iv3);

   memcpy(text, cyptxt3, sizeof(cyptxt3));
   AESdecryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE * 3);
   printf("Decrypted text:\n");
   for (i = 0; i < sizeof(cyptxt3); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(plntxt3); i++) {
      printf("%02x.", (int) plntxt3[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   // Test case 4 - decrypt
   printf("Test case 4 - decrypt \n");
   AESinitStream(&state, key4, iv4);

   memcpy(text, cyptxt4, sizeof(cyptxt4));
   AESdecryptStream_CBC(&state, text, AES_CBC_BLOCK_SIZE * 4);
   printf("Decrypted text:\n");
   for (i = 0; i < sizeof(cyptxt4); i++) {
      printf("%02x.", (int) text[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("Expected text:\n");
   for (i = 0; i < sizeof(plntxt4); i++) {
      printf("%02x.", (int) plntxt4[i]);
      if (0 == ((i+1) % AES_CBC_BLOCK_SIZE)) printf("\n");
   }
   printf("\n");

   return 0;
}