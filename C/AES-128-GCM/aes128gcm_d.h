/* Under the 16-byte (128-bit) key at k and the 12-byte (96-bit) IV , encrypt the plaintext at p and store it at c. 
   The length of the plaintext is a multiple of 16 bytes given at len (e.g., len = 2 for a 32-byte p). The length of the
   ciphertext c is (len+1)*16 bytes. */
void aes128gcm(
unsigned char *c, const unsigned char *k, const unsigned char *IV, const unsigned char *p, const unsigned int len);
