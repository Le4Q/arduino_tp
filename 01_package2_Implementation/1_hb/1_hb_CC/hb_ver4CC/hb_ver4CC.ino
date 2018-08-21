/*
 * Suggested parameter values (Armknecht et. al):
 * 1: k = 512, e = 0.25, u = 0.348, n = 1164 ---- 12 seconds
 * 2: k = 512, e = 0.125, u = 0.256, n = 441 ---- 5 seconds
 * 3: k = 512, e = 0.125, u = 0.1875, n = 256 ---- 3 seconds (impl. in hb_ver2)
 */

/* key length k */
const size_t k = 768;

/* epsilon in (0, 0.5), supports up to three decimals*/
const float eps = 0.25;

/* acceptance threshold u in (epsilon, 0.5) */
const float u = 0.3315;

/* iterations n*/
const unsigned n = 1494;

/* ===================================================================== */

/* key size in Bytes */
const size_t keySize = k/8; // = 64 for k = 512

/* key in {0,1}^k */
uint8_t key[keySize];

void setup() {

  initializeKey();
  Serial.begin (9600) ;
  while(!Serial) {;}

  Serial.print("Key of length ");
  Serial.print(k);
  Serial.println(": ");
  for(int i=0; i<keySize; i++) {
    for(int j=0; j<8; j++) {
      Serial.print(getBit(key[i], j));
    }
  }
  Serial.println();
}

void loop() {

  /* Average Time */
  long sum = 0;

  for(int i=0; i<10; i++) {
    unsigned long t1 = millis();
    hbTest();
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
 * Simulation of HB protocol using either a random key or the true key with
 * probability of 0.5 each.
 */
void hbTest()
{
  int counter = 0; // counter for unsuccessful iteration
   unsigned long time0, time1 = 0, time2 = 0, time3 = 0, time4 = 0, tmp1, tmp2;

  /* TAG: generate candidate key */
  tmp1 = millis();
  uint8_t candidate[keySize];
  generateKey(&candidate[0]); //prints TRUE KEY or RANDOM KEY
  tmp2 = millis(); time0 = tmp2-tmp1;

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    uint8_t a[keySize]; // random challenge a in {0,1}^k

    boolean z; // response z in {0, 1}

    /* READER: choose random challenge a */
    tmp1 = millis();
    for(int j=0; j<keySize; j++) {
      a[j] = random(256);
    }
    tmp2 = millis(); time1 += tmp2-tmp1;
    /* TAG: get z as candidate*a XOR v */
    tmp1 = millis();
    z = getZ(candidate, a, keySize);
    tmp2 = millis(); time2 += tmp2-tmp1;
    
    /* READER: check if z = candidate* XOR v ?= key*a */
    tmp1 = millis();
    if(z != dotProduct(key, a, keySize)) {
      counter++;
    }
    tmp2 = millis(); time3 += tmp2-tmp1;
  }

  Serial.print("Candidate Key generation: ");
  Serial.println(time0);
  Serial.print("Challenges generation: ");
  Serial.println(time1);
  Serial.print("Tagging: ");
  Serial.println(time2);
  Serial.print("Verification: ");
  Serial.println(time3);
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
 * Computes a noisy dot product of two vectors x and y in {0,1}^k mod 2 with
 * probability eps of the result bit being flipped.
 */
boolean getZ(uint8_t x[], uint8_t y[], size_t k)
{
  long rand = random (1, 1000);
  boolean v = rand < eps*1000;

  boolean product = dotProduct(x, y, k);

  return product^v;
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
