/**********************************************************************
	Samples\AES_Enncryption\aes_streamtest.c
	Zworld, 2001

	stream test

	tests out CFB(cipher feedback) stream encryption

	encrypts and decrpyts a test string '0123456789'
	and prints the decrypted result, which should be
	the original string.
	Uses MD5 to hash a pass phrase into a 128-bit key.

	Cipher feedback encryption is a method for using a block cipher like Rijndael
	to encrypt a stream of characters. It does this by maintaining a feedback
	register which is the same size as the cipher block size. The feedback
	register is loaded with a known initial set of data (the init vector) and
	then encrypted. Each byte in the feedback register is XORed with a plain text
	byte to produce the cipher text. When a feedback byte is used it is shifted
	out of the feedback register and the new ciphertext byte is shifted in.
	Each time a full block of new ciphertext has been shifted in to the
	feedback register, it is encrypted again. This scheme allows any number
	of bytes to be encrypted when needed. Since XORing the ciphertext with
	matching bytes from the feedback register will recover the plaintext,
	decryption is a similar process, where the decrypter's feedback register
	is kept synchonized with the encrypter. For a more complete explanation
	of this see 'Applied Crytography, 2nd Edition' by Bruce Schneier, pg 200.

	The Rabbit implementation of cipher feedback mode uses an AESstreamState
	structure to hold the state of the feedback register. As the sample below
	indicates, seperate states must be used for encrypting and decrypting.

***********************************************************************/
#class auto

#use "aes_crypt.lib"
#use "md5.lib"

const char passphrase[] = "lampshade quarter 23";

void main()
{
	char key[16];
	char init_vector[16];
	AESstreamState encrypt_state;
	AESstreamState decrypt_state;
	md5_state_t md5_state;
	char data[11];
	int i;

	//initialize data
	for(i = 0;i < 10;i++)
	{
		data[i] = '0' + i;
	}
	data[10] = 0; //Null terminator for printing

	//init vector
	for(i = 0;i < 16;i++)
	{
		init_vector[i] = 0;
	}

	//hash phrase into key
	md5_init(&md5_state);
	md5_append(&md5_state, passphrase, strlen(passphrase));
	md5_finish(&md5_state, key);	//turns passphrase into 128-bit key

	//need seperate stream states for encryption and decryption
	AESinitStream(&encrypt_state, key, init_vector);
	AESinitStream(&decrypt_state, key, init_vector);

	while(1)
	{
		//encrypts data in place
		AESencryptStream(&encrypt_state, data, 10);
		//decrypts data in place
		AESdecryptStream(&decrypt_state, data, 10);
		printf("%s\n", data);
	}
}