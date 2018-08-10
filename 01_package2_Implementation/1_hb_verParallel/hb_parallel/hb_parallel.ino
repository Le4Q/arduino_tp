#define R n
#define C keySize
#define nBytes n/8 + (n%8 != 0)

#include<Time.h>

/*
 * Version with parallelization (see auth-mac.pdf, p9, 10)
 *
 * Suggested parameter values (Armknecht et. al):
 * 1: k = 512, e = 0.25, u = 0.348, n = 1164 ---- 13 seconds
 * 2: k = 512, e = 0.125, u = 0.256, n = 441 ---- 5334 ms
 * 3: k = 512, e = 0.125, u = 0.1875, n = 256 ----
 */

/* key length k */
const size_t k = 512; // = l

/* epsilon in (0, 0.5), supports up to three decimals */
const float eps = 0.125;

/* acceptance threshold u in (epsilon, 0.5) */
const float u = 0.256;

/* iterations n */
const size_t n = 441;

/* number of challenges < n to send at a time  */
const unsigned maxChallenges = 22; // max: 22

/* ===================================================================== */

/* key size in bytes */
const size_t keySize = k/8; // = 16 for k = 128

/* key in {0,1}^k */
uint8_t key[keySize];


void setup() {
  initializeKey();

  Serial.begin (9600) ;
  while(!Serial) {;}

  Serial.print("Key of length ");
  Serial.print(k);
  Serial.println(": ");
  printBitString(&key[0], keySize);
  Serial.println();
}

void loop() {

  hbTest();

  delay(1000);
}

/*
 * Simulation of HB protocol using either a random key or the true key with
 * probability of 0.5 each.
 */
void hbTest()
{
  int counter = 0; // counter for unsuccessful iteration

  /* READER: generate candidate key */
  uint8_t candidate[keySize];
  generateKey(&candidate[0]); // prints TRUE KEY or RANDOM KEY

  // Begin
  for(int toSend = n; toSend > 0; toSend-=maxChallenges) {

    size_t bytesToSend, bitsToSend;

    if(toSend > maxChallenges) {
      bitsToSend = maxChallenges;
      bytesToSend = maxChallenges/8 + (maxChallenges%8 != 0);
    }
    else {
      bitsToSend = toSend;
      bytesToSend = toSend/8 + (toSend%8 != 0);
    }

    uint8_t r[bitsToSend][keySize]; // random challenges R

    /* READER: choose random challenges R */
    for(int i=0; i<bitsToSend; i++) {
      for(int j=0; j<keySize; j++) {
        r[i][j] = random(256);
      }
    }

    /* TAG: Calculate response vector z */
    uint8_t z[bytesToSend] = {0};
    matrixVectorProduct(&z[0], r, candidate, bitsToSend);

    for(int i=0; i<bytesToSend; i++) {
      for(int j=0; j<8; j++) {
        setBit(&z[i], j, getBit(z[i], j)^generateNoiseBit());
      }
    }

    /* READER: check condition, increse counter if unsuccessful*/
    uint8_t check[bytesToSend] = {0};
    matrixVectorProduct(&check[0], r, key, bitsToSend);

    for(int i=0; i<bytesToSend; i++) {
      for(int j=0; j<8; j++) {
        if(getBit(z[i], j)^getBit(check[i], j) == 1) {
          counter++;
        }
      }
    }
  }


  /* Print message depending on whether authentication was successful. */
  String s;
  if(counter<=(u*n)) {
    s = "Authentication ACCEPTED";
  } else {
    s = "Authentication REJECTED";
  }

  Serial.println(s);
  Serial.print("Unsuccessful iterations: ");
  Serial.print(counter);
  Serial.print(" / ");
  Serial.println(n);
  Serial.println("==========================================================");

}

/*
 * Initializes the key during setup.
 */
void initializeKey()
{
  for(int i=0; i<keySize; i++) {
    key[i] = random(256);
  }
}

/*
 * Returns the bit of a byte x at position pos (0 = least significant).
 */
boolean getBit(uint8_t x, uint8_t pos) // 0<= pos <= 7
{
  return !!((x<<(7-pos)) & 0x80);
}

/*
 * Sets the bit of x at position pos to value val.
 */
void setBit(uint8_t *x, uint8_t pos, boolean val)
{
  *x ^= (-val ^ *x) & (1UL << pos);
}

void printBitString(uint8_t *x, size_t k)
{
  for(int i=0; i<k; i++) {
    for(int j=0; j<8; j++) {
      Serial.print(getBit(x[i], j));
    }
  }
}

 /*
  * Computes Matrix-Vector-Product of matrix mat and vector vect
  * and stores it in vector result.
  */
 void matrixVectorProduct (uint8_t *result, uint8_t mat[][C], uint8_t vect[C], uint8_t challenges)
 {
   size_t count = 0;
   for(int i = 0; i<challenges; i++) {

     boolean sum = 0;

     for(int j = 0; j<C; j++) {

       boolean temp = 0;

       for(int q = 0; q<8; q++) {
         temp ^= getBit(mat[i][j], q) & getBit(vect[j], q);
       }
       sum ^= temp;
     }

     setBit(&result[count], i%8, sum);
     if((i+1) % 8 == 0) {
       count++;
     }
   }
 }

/*
 * Generates either a random key or true key with probability of 0.5 for each.
 */
void generateKey(uint8_t *x)
{
  boolean r = random(2);

  if(r == 0) {
    Serial.println("RANDOM KEY: ");
    for(int i=0; i<keySize; i++) {
      x[i] = random(256);
    }
    printBitString(&x[0], keySize);

  } else {
    Serial.println("TRUE KEY: ");
    for(int i=0; i<keySize; i++) {
      x[i] = key[i];
    }
    printBitString(&x[0], keySize);
  }
  Serial.println();
}

/*
 * Returns noise bit v with probability of eps of being 1 and probability 1-eps
 * of being 0.
 */
boolean generateNoiseBit() {
  long rand = random(100);
  boolean v = rand < eps*100;

  return v;
}
