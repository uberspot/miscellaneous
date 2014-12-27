
#include "aes128gcm.h"

/* Increments the last 4 bytes of the given char array as an integer */
void incr32(unsigned char* data)
{
    // convert the last 4 bytes to an int
    int value = data[15] | data[14] << 0x8 | data[13] << 0x10 | data[12] << 0x18;
    // increment it
    value++;
    // convert the integer back to the original char array positions
    data[12] = (value>>24) & 0xFF;
    data[13] = (value>>16) & 0xFF;
    data[14] = (value>>8) & 0xFF;
    data[15] = value & 0xFF;
}

/* Under the 16-byte (128-bit) key "k",
and the 12-byte (96-bit) initial value "IV",
encrypt the plaintext "plaintext" and store it at "ciphertext".
The length of the plaintext is a multiple of 16-byte (128-bit) given by len_p (e.g., len_p = 2 for a 32-byte plaintext).
The length of the ciphertext "ciphertext" is len_p*16 bytes.
The authentication tag is obtained by the 16-byte tag "tag".
For the authentication an additional data "add_data" can be added.
The number of blocks for this additional data is "len_ad" (e.g., len_ad = 1 for a 16-byte additional data).
*/
void aes128gcm(unsigned char *ciphertext, unsigned char *tag, const unsigned char *k, const unsigned char *IV, const unsigned char *plaintext, const unsigned long len_p, const unsigned char* add_data, const unsigned long len_ad) {
    //printState("\nKey: ", k, 16);
    //printState("IV: ", IV, 12);
    //printState("\nPlaintext: ", plaintext, len_p * 16);
    //printState("ADD: ", add_data, len_ad * 16);

    unsigned char J0[16] = { 0x00 };  // initialize to zeros
    unsigned char INCJ0[16]; // incremented J0
    memcpy(J0, IV, 12);  // copy the IV to the first 12 bytes of J0

    incr32(J0); // increment to : IV || 0x00 0x00 0x00 0x01

    memcpy(INCJ0, J0, 16);

    incr32(INCJ0); // increment to : IV || 0x00 0x00 0x00 0x02

    //printState("J0: ", J0, 16);
    //printState("INCJ0: ", INCJ0, 16);

    // call gctr to get final cipher text
    aes_gctr(ciphertext, plaintext, k, INCJ0, len_p);

    //printState("ciphertext: ", ciphertext, len_p * 16);

    // create H
    unsigned char H[16];
    unsigned char GHashOutput[16];
    unsigned char zeros[16]  = { 0 };
    aes128e(H, zeros, k);  // use aes with zeros as plaintext and k as key to get H

    aes_ghash(GHashOutput, H, ciphertext, len_p, add_data, len_ad);  // run GHASH on ADD|CIPHERTEXT|LENGTH_ADD|LENGTH_P
    //printState("\nGHashOutput: ", GHashOutput, 16);
    aes_gctr(tag, GHashOutput, k, J0, 1);  // Run GCTR to get final tag based on the output of GHASH
    //printState("tag: ", tag, 16);
}

/* Shifts the whole given block of chars by 1 to the right */
void shift_right(unsigned char *block)
{
    // start from the last block
    for (int i = 15; i > 0; i--) {
        // shift it by one
        block[i] = block[i] >> 1;
        // if the first bit of the previous block is one
        if ((block[i-1] & 1)) {
            // move it to the last bit of the current block (basically carry the bit)
            block[i] = block[i] | 0x80;
        }
    }
    // also shift the first block irregardless of carry
    block[0] = block[0] >> 1;
}

/* Multiplies X, Y blocks in the Galois Field 2^128 based on the specification */
void mult_block(unsigned char *X, const unsigned char *Y)
{
    unsigned char V[16];
    unsigned char Z[16]  = { 0 };
    memcpy(V, Y, 16);

    // traverse each block
    for (int i = 0; i < 16; i++)
    {
        // traverse each bit in the current block
		for (int j = 0; j < 8; j++)
		{
            if(X[i] & ((0x80) >> j)) {
                xorBlocks(Z, V, 16);
            }
            if (V[15] & 0x01) {
                shift_right(V);

                V[0] ^= 0xe1; // XOR with the R bit string from the specification: 11100001 || 0^120
            } else {
                shift_right(V);
            }
        }
    }

    memcpy(X,Z,16);
}


void aes_ghash(unsigned char *GHashOutput, const unsigned char *H,  const unsigned char *ciphertext, const unsigned long len_p, const unsigned char *add_data, const unsigned long len_ad)
{
    // temp used to keep the result between each round of ghash, initialized to zeros for the first round
    unsigned char temp[16] = { 0x00 }, tempLengths[16];
    // the lengths of add and ciphertext in bits:
    unsigned long bitlen_ad = len_ad * 8 * 16;
    unsigned long bitlen_p = len_p * 8 * 16;
    // two temporary buffers
    unsigned char t1[8], t2[8];

    //printf("longad: %lu ", bitlen_ad);
    //printf("longp: %lu \n", bitlen_p);

    //printState("lenaHEX: ", &bitlen_ad, 8);

    // do xor and multiply with H for each block of ADD
    for (unsigned long i = 0 ; i < len_ad; i++) {
        xorBlocks(temp, add_data + i * 16, 16);
        mult_block(temp, H);
    }

    // do xor and multiply with H for each block of the ciphertext
    for (unsigned long  i= 0 ; i < len_p; i++) {
        xorBlocks(temp, ciphertext + i * 16, 16);
        mult_block(temp, H);
    }

    // copy the lenghts of add and ciphertext to the temporary buffers.
    // this effectively copies the hex bytes from the long
    memcpy(t1, &bitlen_ad, 8);
    memcpy(t2, &bitlen_p, 8);

    // reverse the t1, t2 buffers (cause of little endian) and store them in
    // one final 16 char buffer called tempLengths
    for (int i = 0; i < 8; i++) {
        tempLengths[i] = t1[7-i];
        tempLengths[i + 8] = t2[7-i];
    }

    // xor and multiply the templengths buffer with H and keep the final result
    xorBlocks(temp, tempLengths, 16);
    mult_block(temp, H);

    memcpy(GHashOutput, temp, 16); // store to the output
}

void aes_gctr(unsigned char *ciphertext, const unsigned char *plaintext,  const unsigned char *k, unsigned char *INCJ0, const unsigned long len_p)
{
    // for each block of the plaintext
    for(unsigned long i = 0; i < len_p ; i++) {
        //printState("ciphertext + i*16: ", ciphertext + i*16, 16);
        //printState("plaintext + i*16: ", plaintext + i*16, 16);

        // do AES on the counter with the k key
        aes128e(ciphertext + i*16, INCJ0, k);
        // xor the result with the plaintext
        xorBlocks(ciphertext + i*16, plaintext + i*16, 16);
        // increment the counter
        incr32(INCJ0);
    }
}

/* XORs two blocks of the given size */
void xorBlocks(unsigned char *destination, const unsigned char *addedBlock, unsigned int size)
{
    for (unsigned int i = 0; i < size; i++) {
        destination[i] ^= addedBlock[i];
    }
    //printState(" xorBlocks: ", destination, size);
}

