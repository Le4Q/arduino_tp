#include<Time.h>

/* key length k (80 or 128) */
const size_t secPar = 128;

/* key size in Bytes */
const size_t keySize = secPar/8;

/* key in {0,1}^k */
uint8_t key[keySize];

/* epsilon in (0, 0.5), supports up to two decimals */
float eps = 0.1;

/* iterations n */
int n = 10;

void setup() {

  initializeKey(); // TODO: get real key
  Serial.begin (9600) ;
  Serial.setTimeout(10000);

  while(!Serial) {;}

}

void loop() {

  hbTag();

}

void hbTag() {

  if(Serial.available() > 0) {

    /* Receive challenge */
    uint8_t r[n][keySize];
    size_t read = 0;

    for(int i=0; i<n; i++) {
      read += Serial.readBytes(r[i], keySize);
    }

    Serial.println("Number of Bytes of R read: ");
    Serial.println(read);

    time_t t1 = now();

    /* generate candidate key */
    uint8_t candidate[keySize];

    generateKey(&candidate[0]); //prints TRUE KEY or RANDOM KEY

    /* Compute z in {0,1}^n */
    size_t zsize = n/8 + ((n % 8) != 0);
    uint8_t z[zsize];

    for(int i=0; i<zsize; i++) {
      for(int j=0; j<8; j++) {
        if((i*8+j) < n) {
          setBit(&z[i], j, getZ(candidate, r[i*8+j], keySize));
        }
      }
    }

    /* Send z */
    size_t sent = 0;

    sent = Serial.write(z, zsize);

    Serial.println("Number of Bytes of z sent: ");
    Serial.println(sent);

    time_t t2 = now();

    float diff = ((float)(t2 - t1) / 1000000.0F ) * 1000;
    Serial.println(diff);

  }

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

/*
 * Prints a uint8_t array in bit representation (most significant bit first).
 */
void printBitString(uint8_t *x, size_t k)
{
  for(int i=k-1; i>=0; i--) {
    for(int j=0; j<8; j++) {
      Serial.print(getBit(x[i], 7-j));
    }
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
  long rand = random (0, 100);
  boolean v = rand < eps*100;

  boolean product = dotProduct(x, y, k);

  return product^v;
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
