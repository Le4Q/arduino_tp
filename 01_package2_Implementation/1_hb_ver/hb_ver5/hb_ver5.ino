#define nBytes n/8 + (n%8 != 0)

#include<Time.h>

/*
 * Improved version suggested by Gilbert et. al to use 3rd set of parameters.
 *
 * Suggested parameter values (Armknecht et. al):
 * 1: k = 512, e = 0.25, u = 0.348, n = 1164 ---- 12 seconds (impl. in hb)
 * 2: k = 512, e = 0.125, u = 0.256, n = 441 ---- 5 seconds (impl. in hb)
 * 3: k = 512, e = 0.125, u = 0.1875, n = 256 ---- 3 seconds
 * /

/* key length k */
const size_t k = 768;

/* epsilon in (0, 0.5), supports up to three decimals*/
float eps = 0.25;

/* acceptance threshold u in (epsilon, 0.5) */
float u = 0.3315;

/* iterations n*/
unsigned n = 1494;

/* ===================================================================== */

/* key size in Bytes */
const size_t keySize = k/8; // = 64 for k = 512

/* key in {0,1}^k */
uint8_t key[keySize];

void setup() {

  initializeKey();
  Serial.begin (9600) ;
  while(!Serial) {;}
}

void loop() {
  Serial.print("Key of length ");
  Serial.print(k);
  Serial.println(": ");
  for(int i=0; i<keySize; i++) {
    for(int j=0; j<8; j++) {
      Serial.print(getBit(key[i], j));
    }
  }
  Serial.println();

  /* Average Time */
  int sum = 0;

  for(int i=0; i<10; i++) {
 //   time_t t1 = now();
    hbTest();
 //   time_t t2 = now();
 //   sum += t2 - t1;
  }

  Serial.print("Average of 10 Authentication: ");
  Serial.print(sum/10);
  Serial.println(" seconds");
  Serial.println("==========================================================");

  delay(100);
}

/*
 * Simulation of HB protocol using either a random key or the true key with
 * probability of 0.5 each.
 */
void hbTest()
{
  int counter = 0; // counter for unsuccessful iteration

  /* TAG: generate candidate key and noise bits*/
  uint8_t candidate[keySize];
  uint8_t e[nBytes];

  generateKey(&candidate[0]); //prints TRUE KEY or RANDOM KEY
  generateNoiseArray(&e[0], nBytes);

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    uint8_t a[keySize]; // random challenge a in {0,1}^k

    boolean z; // response z in {0, 1}

    /* READER: choose random challenge a */
    for(int j=0; j<keySize; j++) {
      a[j] = random(256);
    }

    /* TAG: get z as candidate*a XOR v */
    //z = getZ(candidate, a, keySize);
    z = dotProduct(candidate, a, keySize)^getBit(e[i/8], i%8);

    /* READER: check if z = candidate* XOR v ?= key*a */
    if(z != dotProduct(key, a, keySize)) {
      counter++;
    }

  }

  /* Print message depending on whether authentication was successful. */
  String s;
  if(counter<=(u*n)) {
    s = "Authentication ACCEPTED";
  }
  else
  {
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
 *  Initializes the key during setup.
 */
void initializeKey()
{
  for(int i=0; i<keySize; i++) {
    key[i] = random(256);
  }
}

/*
 * As suggested by Gilbert et. al, computes set of noise bits of length length
 * and keeps them only if the number of 1s is less than u*n.
 */
void generateNoiseArray(uint8_t *x, size_t length) {

  do {
    for(int i=0; i<length; i++) {
      for(int j=0; j<8; j++) {
        setBit(&x[i], j, generateNoiseBit());
      }
    }
  } while(hammingWeight(x, length) > u*n);
}

/*
 * Computes the dot product of two vectors x and y in {0,1}^k mod 2.
 */
 boolean dotProduct (uint8_t x[], uint8_t y[], size_t k)
 {
   boolean sum=0;

   for(int i=0; i<k; i++) {
     for(int j=0; j<8; j++) {
       sum ^= getBit(x[i], j) & getBit(y[i], j);
     }
   }

   return sum;
 }

/*
 * Generates either a random key or true key with probability of 0.5 for each.
 */
void generateKey(uint8_t *x)
{
  long r = random(2);

  if(r == 0) {
    Serial.println("RANDOM KEY: ");
    for(int i=0; i<keySize; i++) {
      x[i] = random(256);
      for(int j=0; j<8; j++) {
        Serial.print(getBit(x[i], j));
      }
    }
  } else {
    Serial.println("TRUE KEY: ");
    for(int i=0; i<keySize; i++) {
      x[i] = key[i];
      for(int j=0; j<8; j++) {
        Serial.print(getBit(x[i], j));
      }
    }
  }
  Serial.println();
}

/*
 * Returns Hamming Weight of uint8_t array x of length length.
 */
int hammingWeight(uint8_t x[], size_t length) {
  int sum = 0;
  for(int i=0; i<length; i++) {
    for(int j = 0; j<8; j++) {
      sum += getBit(x[i], j);
    }
  }
  return sum;
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
