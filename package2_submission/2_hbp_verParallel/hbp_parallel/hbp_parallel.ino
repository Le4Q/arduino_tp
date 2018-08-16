#define C1 keySize1
#define C2 keySize2

/*
 * Version with parallelization (see auth-mac.pdf, p9, 10)
 *
 * Suggested parameter values (Armknecht et. al):
 * 1: k = 512, e = 0.25, u = 0.348, n = 1164 ----
 * 2: k = 512, e = 0.125, u = 0.256, n = 441 ----
 * 3: k = 512, e = 0.125, u = 0.1875, n = 256 ----
 */

/* key length k */
const size_t keyLength1 = 80;
const size_t keyLength2 = 512;

/* epsilon in (0, 0.5), supports up to three decimals */
const float eps = 0.125;

/* acceptance threshold u in (epsilon, 0.5) */
const float u = 0.256;

/* iterations n */
const size_t n = 441;

/* number of challenges < n to send at a time  */
const unsigned maxChallenges = 18; // max: 18

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
  uint8_t candidate1[keySize1], candidate2[keySize2];
  generateKey1(&candidate1[0]); //prints TRUE KEY or RANDOM KEY
  generateKey2(&candidate2[0]);

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

    uint8_t b[bitsToSend][keySize1]; // blinding factors
    uint8_t a[bitsToSend][keySize2]; // random challenges R

    /* TAG: choose random blinding factors b */
    for(int i=0; i<bitsToSend; i++) {
      for(int j=0; j<keySize1; j++) {
        a[i][j] = random(256);
      }
    }

    /* READER: choose random challenges a */
    for(int i=0; i<bitsToSend; i++) {
      for(int j=0; j<keySize2; j++) {
        a[i][j] = random(256);
      }
    }

    /* TAG: Calculate response vector z in {0,1}^n as z = R * candidate XOR e */
    uint8_t z[bytesToSend] = {0};
    uint8_t z1[bytesToSend] = {0}, z2[bytesToSend] = {0};
    matrixVectorProductS(&z1[0], b, candidate1, bitsToSend);
    matrixVectorProduct(&z2[0], a, candidate2, bitsToSend);

    for(int i=0; i<bytesToSend; i++) {
      for(int j=0; j<8; j++) {
        setBit(&z[i], j, getBit(z1[i], j)^getBit(z2[i], j)^generateNoiseBit());
      }
    }

    /* READER: check condition, increse counter if unsuccessful*/
    uint8_t check[bytesToSend] = {0};
    uint8_t check1[bytesToSend] = {0}, check2[bytesToSend] = {0};
    matrixVectorProductS(&check1[0], b, key1, bitsToSend);
    matrixVectorProduct(&check2[0], a, key2, bitsToSend);

    for(int i=0; i<bytesToSend; i++) {
      for(int j=0; j<8; j++) {
        if(getBit(z[i], j)^getBit(check1[i], j)^getBit(check2[i], j) == 1) {
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
  for(int i=0; i<keySize1; i++) {
    key1[i] = random(256);
  }
  for(int i=0; i<keySize2; i++) {
    key2[i] = random(256);
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
 void matrixVectorProduct (uint8_t *result, uint8_t mat[][C2], uint8_t vect[C2], uint8_t challenges)
 {
   size_t count = 0;
   for(int i = 0; i<challenges; i++) {

     boolean sum = 0;

     for(int j = 0; j<C2; j++) {

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
  * Computes Matrix-Vector-Product of matrix mat and vector vect
  * and stores it in vector result.
  */
 void matrixVectorProductS (uint8_t *result, uint8_t mat[][C1], uint8_t vect[C1], uint8_t challenges)
 {
   size_t count = 0;
   for(int i = 0; i<challenges; i++) {

     boolean sum = 0;

     for(int j = 0; j<C1; j++) {

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
 * Returns noise bit v with probability of eps of being 1 and probability 1-eps
 * of being 0.
 */
boolean generateNoiseBit() {
  long rand = random(100);
  boolean v = rand < eps*100;

  return v;
}
