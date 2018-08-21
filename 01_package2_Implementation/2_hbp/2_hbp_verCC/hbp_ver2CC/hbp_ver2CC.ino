#include<Time.h>

/*
 * Suggested parameter values (Armknecht et. al):
 * 1: k1 = 80, k2 = 512, e = 0.25, u = 0.348, n = 1164 ---- 14617ms (tag: 1077ms + 2904ms)
 * 2: k1 = 80, k2 = 512, e = 0.125, u = 0.256, n = 441 ---- 6061ms (tag: 408ms + 1099ms)
 * 3: k1 = 80, k2 = 512, e = 0.125, u = 0.1875, n = 256 ----
 */

/* key length k (80 or 128) */
const size_t keyLength1 = 80;
const size_t keyLength2 = 512;

/* epsilon in (0, 0.5), supports up to two decimals */
const float eps = 0.125;

/* acceptance threshold u in (epsilon, 0.5) */
const float u = 0.256;

/* iterations n */
const size_t n = 441;

/* ===================================================================== */

/* key sizes in Bytes */
const size_t keySize1 = keyLength1/8;
const size_t keySize2 = keyLength2/8;

/* keys in {0,1}^k */
uint8_t key1[keySize1], key2[keySize2];

void setup() {
  initializeKey();

  Serial.begin (9600) ;
  while(!Serial) {;}

  Serial.print("Key 1 of length ");
  Serial.print(keyLength1);
  Serial.println(": ");
  printBitString(&key1[0], keySize1);
  Serial.println();

  Serial.print("Key 2 of length ");
  Serial.print(keyLength2);
  Serial.println(": ");
  printBitString(&key2[0], keySize2);
  Serial.println();
}

void loop() {

  /* Average Time */
  unsigned long sum = 0;

  for(int i=0; i<10; i++) {
    unsigned long t1 = millis();
    hbpTest();
    unsigned long t2 = millis();
    sum += t2 - t1;
  }

  Serial.print("Average of 10 Authentication: ");
  Serial.print(sum/10);
  Serial.println(" ms");
  Serial.println("==========================================================");

  delay(100);
}

/*
 * Simulation of HB+ protocol using either a random key or the true key with
 * probability of 0.5 each.
 */
void hbpTest()
{
  int counter=0; // counter for unsuccessful iteration
  unsigned long time0, time1 = 0, time2 = 0, time3 = 0, time4 = 0, tmp1, tmp2;

  /* TAG: generate candidate key */
  tmp1 = millis();
  uint8_t candidate1[keySize1], candidate2[keySize2];
  generateKey1(&candidate1[0]); //prints TRUE KEY or RANDOM KEY
  generateKey2(&candidate2[0]);
  tmp2 = millis(); time0 = tmp2-tmp1;

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    uint8_t b[keySize1]; // random blinding factor b in {0,1}^k

    uint8_t a[keySize2]; // random challenge a in {0,1}^k

    boolean z; // response z in {0, 1}

    /* TAG: choose random blinding factor b */
    tmp1 = millis();
    for(int i=0; i<keySize1; i++) {
      b[i] = random(256);
    }
    tmp2 = millis(); time1 += tmp2-tmp1;

    /* READER: choose random challenge a */
    tmp1 = millis();
    for(int i=0; i<keySize2; i++) {
      a[i] = random(256);
    }
    tmp2 = millis(); time2 += tmp2-tmp1;

    /* TAG: get z as candidate1*b XOR candidate2*a XOR v */
    tmp1 = millis();
    z = dotProduct(candidate1, b, keySize1)^dotProduct(candidate2, a, keySize2)^generateNoiseBit();
    tmp2 = millis(); time3 += tmp2-tmp1;

    /* READER: check if z = candidate1*b XOR candidate2*a XOR v ?= key1*b XOR key2*a */
    tmp1 = millis();
    if(z != dotProduct(key1, b, keySize1)^dotProduct(key2, a, keySize2)) {
      counter++;
    }
    tmp2 = millis(); time4 += tmp2-tmp1;
  }

  Serial.print("Candidate Key generation: ");
  Serial.println(time0);
  Serial.print("Blinding factors generation: ");
  Serial.println(time1);
  Serial.print("Challenges generation: ");
  Serial.println(time2);
  Serial.print("Tagging: ");
  Serial.println(time3);
  Serial.print("Verification: ");
  Serial.println(time4);

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
 * Initializes the key during setup.
 */
void initializeKey()
{
  for(int i=0; i<keySize1; i++) {
    key1[i] = random(256);
  }
  for(int i=0; i<keySize2; i++) {
    key2[i] = random(256);
  }
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

boolean generateNoiseBit()
{
  long rand = random(100);
  boolean v = rand < eps*100;

  return v;
}

/*
 * Generates either a random key1 or true key1 with probability of 0.5 for each.
 */
void generateKey1(uint8_t *x)
{
  long r = random(2);

  if(r == 0) {
    Serial.println("RANDOM KEY 1: ");
    for(int i=0; i<keySize1; i++) {
      x[i] = random(256);
      for(int j=0; j<8; j++) {
        Serial.print(getBit(x[i], j));
      }
    }
  }
  else {
    Serial.println("TRUE KEY 1: ");
    for(int i=0; i<keySize1; i++) {
      x[i] = key1[i];
      for(int j=0; j<8; j++) {
        Serial.print(getBit(x[i], j));
      }
    }
  }
  Serial.println();
}

/*
 * Generates either a random key2 or true key2 with probability of 0.5 for each.
 */
void generateKey2(uint8_t *x)
{
  long r = random(2);

  if(r == 0) {
    Serial.println("RANDOM KEY 2: ");
    for(int i=0; i<keySize2; i++) {
      x[i] = random(256);
      for(int j=0; j<8; j++) {
        Serial.print(getBit(x[i], j));
      }
    }
  }
  else {
    Serial.println("TRUE KEY 2: ");
    for(int i=0; i<keySize2; i++) {
      x[i] = key2[i];
      for(int j=0; j<8; j++) {
        Serial.print(getBit(x[i], j));
      }
    }
  }
  Serial.println();
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
