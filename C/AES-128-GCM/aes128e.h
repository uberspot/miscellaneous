#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void aes128e(unsigned char *c, const unsigned char *p, const unsigned char *k);

void expandKey(unsigned char *expandedKey, const unsigned char *key);

void subBytes(unsigned char *state);

void shiftRow(unsigned char *state, unsigned char nbr);

void addRoundKey(unsigned char *state, unsigned char *roundKey);

void runAESForBlock(unsigned char *state, unsigned char *expandedKey);

void shiftRows(unsigned char *state);

void mixColumn(unsigned char *column);

void mixColumns(unsigned char *state);

void printState(const unsigned char *label, const unsigned char *state, const int numOfChars);
