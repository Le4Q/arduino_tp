#include<printf.h>
#include <stdint.h>
#include <stdio.h>


//--------Declerations-------/
#ifndef PRESENT_H
#define PRESENT_H


#define PRESENT_128_KEY_SIZE_BYTES 16
#define PRESENT_BLOCK_SIZE_BYTES 8
void present128_encryptBlock( unsigned char *block, const unsigned char *key );
void present128_decryptBlock( unsigned char *block, const unsigned char *key );
#endif

#define ROUNDS               32
#define ROUND_KEY_SIZE_BYTES  8
// In reality, if you want to use 128 bit present, you should probably just use aes...

unsigned char sBox[16] = {
    0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD, 0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2 };

unsigned char sBoxInverse[16] = {
    0x5, 0xE, 0xF, 0x8, 0xC, 0x1, 0x2, 0xD, 0xB, 0x4, 0x6, 0x3, 0x0, 0x7, 0x9, 0xA };

void copyKey( const unsigned char *from, unsigned char *to, const unsigned char keyLen ){
    int i;
    for( i = 0; i < keyLen; i++ ){
        to[i] = from[i];
    }
}


void generateRoundKeys128( const unsigned char *suppliedKey, unsigned char keys[32][ROUND_KEY_SIZE_BYTES]){
    // trashable key copies
    unsigned char key[PRESENT_128_KEY_SIZE_BYTES];
    unsigned char newKey[PRESENT_128_KEY_SIZE_BYTES];
    unsigned char i, j;
    copyKey( suppliedKey, key, PRESENT_128_KEY_SIZE_BYTES );
    copyKey( key, keys[0], ROUND_KEY_SIZE_BYTES );
    for( i = 1; i < ROUNDS; i++ ){
        // rotate left 61 bits
        for( j = 0; j < PRESENT_128_KEY_SIZE_BYTES; j++ ){
            newKey[j] = (key[(j+7) % PRESENT_128_KEY_SIZE_BYTES] << 5) | (key[(j+8) % PRESENT_128_KEY_SIZE_BYTES] >> 3);
        }
        copyKey( newKey, key, PRESENT_128_KEY_SIZE_BYTES );

        // pass leftmost 8-bits through sBoxes
        key[0] = (sBox[key[0] >> 4] << 4) | (sBox[key[0] & 0xF]);

        // xor roundCounter into bits 62 through 66
        key[8] ^= i << 6; // bits 63-62
        key[7] ^= i >> 2; // bits 66-64

        copyKey( key, keys[i], ROUND_KEY_SIZE_BYTES );
    }
}


void addRoundKey( unsigned char *block, unsigned char *roundKey ){
    unsigned char i;
    for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
        block[i] ^= roundKey[i];
    }
}

void pLayer( unsigned char *block ){
    unsigned char i, j, indexVal, andVal;
    unsigned char initial[PRESENT_BLOCK_SIZE_BYTES];
    copyKey( block, initial, PRESENT_BLOCK_SIZE_BYTES );
    for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
        block[i] = 0;
        for( j = 0; j < 8; j++ ){
            indexVal = 4 * (i % 2) + (3 - (j >> 1));
            andVal = (8 >> (i >> 1)) << ((j % 2) << 2);
            block[i] |= ((initial[indexVal] & andVal) != 0) << j;
        }
    }
}

void pLayerInverse( unsigned char *block ){
    unsigned char i, j, indexVal, andVal;
    unsigned char initial[PRESENT_BLOCK_SIZE_BYTES];
    copyKey( block, initial, PRESENT_BLOCK_SIZE_BYTES );
    for( i = 0; i < PRESENT_BLOCK_SIZE_BYTES; i++ ){
        block[i] = 0;
        for( j = 0; j < 8; j++ ){
            indexVal = (7 - ((2*j)%8)) - (i < 4);
            andVal = (7-((2*i)%8)) - (j < 4);
            block[i] |= ((initial[indexVal] & (1 << andVal)) != 0) << j;
        }
    }
}

void present128_encryptBlock( unsigned char *block, const unsigned char *key ){
    unsigned char roundKeys[ROUNDS][ROUND_KEY_SIZE_BYTES];
    unsigned char i, j;
    generateRoundKeys128( key, roundKeys );
    for( i = 0; i < ROUNDS-1; i++ ){
        addRoundKey( block, roundKeys[i] );
        for( j = 0; j < PRESENT_BLOCK_SIZE_BYTES; j++ ){
            block[j] = (sBox[block[j] >> 4] << 4) | sBox[block[j] & 0xF];
        }
        pLayer( block );
    }
    addRoundKey( block, roundKeys[ROUNDS-1] );
}

void present128_decryptBlock( unsigned char *block, const unsigned char *key ){
    unsigned char roundKeys[ROUNDS][ROUND_KEY_SIZE_BYTES];
    unsigned char i, j;
    generateRoundKeys128( key, roundKeys );
    for( i = ROUNDS-1; i > 0; i-- ){
        addRoundKey( block, roundKeys[i] );
        pLayerInverse( block );
        for( j = 0; j < PRESENT_BLOCK_SIZE_BYTES; j++ ){
            block[j] = (sBoxInverse[block[j] >> 4] << 4) | sBoxInverse[block[j] & 0xF];
        }
    }
    addRoundKey( block, roundKeys[0] );
}

#undef ROUNDS
#undef ROUND_KEY_SIZE_BYTES

typedef struct testCase{
    unsigned char *workingCopy;
    unsigned char *plainText;
    unsigned char *key;
    unsigned char *cipherText;
} testCase_t;
#define TESTCASE_COUNT_128 2
testCase_t testCases128[TESTCASE_COUNT_128] = {
    {
        (unsigned char[8])  {0, 0, 0, 0, 0, 0, 0, 0 },
        (unsigned char[8])  {0, 0, 0, 0, 0, 0, 0, 0 },
        (unsigned char[16]) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        (unsigned char[8])  {0x96, 0xDB, 0x70, 0x2A, 0x2E, 0x69, 0x00, 0xAF}
    },
    {
        (unsigned char[8])  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        (unsigned char[8])  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        (unsigned char[16]) {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        (unsigned char[8])  {0x62, 0x8D, 0x9F, 0xBD, 0x42, 0x18, 0xE5, 0xB4}
    }
};

int test( const unsigned char *a, const unsigned char *b, const int length ){
    int i;
    for( i = 0; i < length; i++ ){
        if( a[i] != b[i] ){
            return 1;
        }
    }
    return 0;
}
unsigned long time; 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  printf_begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  int i, pass = 0;

    for( i = 0; i < TESTCASE_COUNT_128; i++ ){
        testCase_t tc = testCases128[i];
        pass |= test( tc.workingCopy, tc.plainText, 8 );
        present128_encryptBlock( tc.workingCopy, tc.key );
        pass |= test( tc.workingCopy, tc.cipherText, 8 );
        present128_decryptBlock( tc.workingCopy, tc.key );
        pass |= test( tc.workingCopy, tc.plainText, 8 );
    }
    printf( "Present-128 %s\n", (pass == 0) ? "PASS" : "FAIL");

  Serial.print("Time: "); 
  time = micros();
  Serial.print(time);

  delay(1000);
  
}

