#include "aes128e.h"

/* Multiplication by two in GF(2^8). Multiplication by three is xtime(a) ^ a */
#define xtime(a) ( ((a) & 0x80) ? (((a) << 1) ^ 0x1b) : ((a) << 1) )

/* The S-box table */
static const unsigned char sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

/* The round constant table (needed in KeyExpansion) */
static const unsigned char rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10,
    0x20, 0x40, 0x80, 0x1b, 0x36 };

/* 10 rounds for AES-128 */
static const int numOfRounds = 10;

/* The number of bytes in each block processed by aes */
static const int stateWordsNum = 16;

/* The total number of bytes used in the expanded key during all of the AES rounds (11 rounds * 16 bytes of key per
 * round) */
static const int expandedKeyWordsNum = 176;

/* Helper function to print with the given label the state each time. */
void printState(const unsigned char *label, const unsigned char *state, const int numOfChars)
{
    printf("%s \n", label);
    for (int i = 0; i < numOfChars; i++) {
        printf("%02x ", state[i] );
        if (i % stateWordsNum == 15) {
            printf("\n\n");
        } else if (i % stateWordsNum == 3 | 
            i % stateWordsNum == 7 | 
            i % stateWordsNum == 11) {
            printf("\n");
        }
    }
}

/* Under the 128 bit key at k, encrypt the 128 bit plaintext at p and store it at c. */
void aes128e(unsigned char *c, const unsigned char *p, const unsigned char *k) {

    // the 128 bit block to encode
    unsigned char block[stateWordsNum];

    // the expanded key
    unsigned char expandedKey[expandedKeyWordsNum];

    for (int i = 0; i < stateWordsNum; i++) {
        block[i] = p[i];
    }

    //printState("\nInput: ", block, stateWordsNum);
    //printState("Initial Key: ", k, stateWordsNum);

    /* expand the key to 176 bytes */
    expandKey(expandedKey, k);

    //printState("Expanded Key: ", expandedKey, expandedKeyWordsNum);

    /* encrypt the block using the expandedKey */
    runAESForBlock(block, expandedKey);

    /* unmap the block again into the output ciphertext */
    for (int i = 0; i < stateWordsNum; i++) {
        c[i] = block[i];
    }
    //printState("Output: ", c, stateWordsNum);
}

/* Runs all the aes rounds for the given block with the expanded key. */
void runAESForBlock(unsigned char *state, unsigned char *expandedKey)
{
    addRoundKey(state, expandedKey);

    for (int i = 1; i < numOfRounds; i++) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, expandedKey + stateWordsNum * i);
    }
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, expandedKey + stateWordsNum * numOfRounds);
}

/* add round key step, xor's the key with the state byte by byte */
void addRoundKey(unsigned char *state, unsigned char *roundKey)
{
    for (int i = 0; i < stateWordsNum; i++) {
        state[i] ^= roundKey[i];
    }
    //printState(" AddRoundKey: ", state, stateWordsNum);
}

/* substitute all the values from the state with the value in the SBox
 * using the state value as index for the SBox */
void subBytes(unsigned char *state)
{
    for (int i = 0; i < stateWordsNum; i++)
    {
        state[i] = sbox[ state[i] ];
    }
    //printState("  subBytes: ", state, stateWordsNum);
}

/* rotate the word one byte to the left
 * word is a char array of size 4 (32 bits)
 */
void rotateWord(unsigned char *word)
{
    unsigned char c = word[0];
    for (int i = 0; i < 3; i++) {
        word[i] = word[i+1];
    }
    word[3] = c;
}

void shiftRow(unsigned char *state, unsigned char size_of_row)
{
    /* each iteration shifts the row to the left by 1 */
    for (int i = 0; i < size_of_row; i++) {
        rotateWord(state);
    }
}

void shiftRows(unsigned char *state)
{
    unsigned char temp[4];
    /* iterate over the 4 rows and call shiftRow() with that row */
    for (int i = 1; i < 4; i++) {
        // move all the elements of the i-th row to our temp array
        temp[0] = state[i];
        temp[1] = state[i+4];
        temp[2] = state[i+8];
        temp[3] = state[i+12];

        // shift them by i steps
        shiftRow(temp, i);

        // move them again to the initial state positions as before
        state[i] = temp[0];
        state[i+4] = temp[1];
        state[i+8] = temp[2];
        state[i+12] = temp[3];

    }
    //printState("   ShiftRows: ", state, stateWordsNum);
}

void mixColumns(unsigned char *state)
{
    // original contains the column which we start with in each mixColumns step because the operations
    // in each step of the multiplications would change the original array if we did the calculations in place
    unsigned char temp[4], original[4];
    unsigned char tempChar;
    for(int i = 0; i < 4; i++)
    {
        memcpy(original, state + i * 4 , 4);
        memcpy(temp, original , 4);

        tempChar = temp[1];
        state[i*4] = xtime(temp[0]) ^ xtime(temp[1]) ^ tempChar ^ temp[2] ^ temp[3];
        memcpy(temp, original, 4);

        tempChar = temp[2];
        state[i*4 + 1] = temp[0] ^ xtime(temp[1]) ^ xtime(temp[2]) ^ tempChar ^ temp[3];
        memcpy(temp, original, 4);

        tempChar = temp[3];
        state[i*4 + 2] = temp[0] ^ temp[1] ^ xtime(temp[2]) ^ tempChar ^ xtime(temp[3]);
        memcpy(temp, original, 4);

        tempChar = temp[0];
        state[i*4 + 3] = tempChar ^ xtime(temp[0]) ^ temp[1] ^ temp[2] ^ xtime(temp[3]);
    }
    //printState("    MixColumns: ", state, stateWordsNum);
}

/* The core operations performed every 4-th column during the key expansion phase. */
void expandKeyCore(unsigned char *word, int iteration)
{
    /* rotate the 4 byte word one byte to the left */
    rotateWord(word);

    /* apply S-Box substitution on all 4 parts of the word */
    for (int i = 0; i < 4; i++) {
        word[i] = sbox[ word[i] ];
    }

    /* XOR the outputs first position with the rcon corresponding to that iteration */
    word[0] ^= rcon[iteration];
}

/* Key expansion step, expand the given 16 byte key to a 176 byte array containing the keys for
 * all of the aes rounds (11 keys total). */
void expandKey(unsigned char *expandedKey, const unsigned char *key)
{
    /* current expanded key size, in bytes */
    int currentSize = 0;

    unsigned char temp[4] = {0};

    /* set the first 16 bytes of the expanded key to the given input key */
    for (int i = 0; i < stateWordsNum; i++) {
        expandedKey[i] = key[i];
    }

    // advance the currentSize iterator to the position after the first input key
    currentSize += stateWordsNum;

    // while we haven't reached the desired expanded key size (in bytes)
    while (currentSize < expandedKeyWordsNum)
    {
        /* assign the previous 4 bytes (aka the previous column) to the temporary value temp */
        for (int i = 0; i < 4; i++)
        {
            temp[i] = expandedKey[(currentSize - 4) + i];
        }

        /* every 16 bytes (aka every 4th column) we apply the core expansion to our temp
        * and increment rconIteration afterwards */
        if(currentSize % stateWordsNum == 0)
        {
            expandKeyCore(temp, (currentSize / stateWordsNum) -1);
        }

        /* We XOR temp with the four-byte block 16 bytes before (aka 4 columns before)
         * the new expanded key. Save this as the next four bytes in the expanded key. */
        for(int i = 0; i < 4; i++) {
            expandedKey[currentSize] = expandedKey[currentSize - stateWordsNum] ^ temp[i];
            currentSize++;
        }
    }
}
