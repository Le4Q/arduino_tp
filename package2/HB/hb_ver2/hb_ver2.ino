// Version 2 with uint8_t arrays as key, saves space compared to boolean array

/* key length k (80 or 128) */
const size_t secPar = 128;

/* key size in Bytes */
const size_t keySize = secPar/8;

/* key in {0,1}^k */
uint8_t key[keySize];

/* epsilon in (0, 0.5), supports up to two decimals */
float eps = 0.1;

/* iterations n */
int n = 100;

void setup() {

  initializeKey();
  Serial.begin (9600) ;
  delay(100);
}

void loop() {
  Serial.print("Key of length ");
  Serial.print(secPar);
  Serial.println(": ");
  for(int i=keySize-1; i>=0; i--) {
    for(int j=0; j<8; j++) {
      Serial.print(!!((key[i]<<j) & 0x80));
    }
  }
  Serial.println();

  hbTest();
  delay(1000);
}

/*
 * HB protocol using either a random key or the true key with probability of 0.5 each.
 */
void hbTest() {

  /* counter for unsuccessful iteration */
  int counter=0;

  /* generate candidate key */
  uint8_t candidate[keySize];

  generateKey(&candidate[0]); //prints TRUE KEY or RANDOM KEY

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    /* random challenge a in {0,1}^k */
    uint8_t a[keySize];

    /* response z in {0, 1} */
    boolean z;

    /* choose random challenge a */
    for(int i=0; i<keySize; i++) {
      a[i] = random(256);
    }

    /* get z as candidate*a XOR v */
    z = getZ(candidate, a, keySize);

    /* check if z = candidate* XOR v ?= key*a */
    if(z != dotProduct(key, a, keySize)) {
      counter++;
    }

  }

  /* Print message depending on whether authentication was successful. */
  String s;
  if(counter<=(eps*n)) {
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
       sum ^= !!((x[i]<<j) & 0x80) & !!((y[i]<<j) & 0x80);
     }
   }

   return sum;
 }

/*
 * Computes a noisy dot product of two vectors x and y in {0,1}^k mod 2 with probability eps of the result bit being flipped.
 */
boolean getZ(uint8_t x[], uint8_t y[], size_t k)
{
  long rand = random (1, 100);
  boolean v = rand < eps*100;

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
    for(int i=keySize-1; i>=0; i--) {
      x[i] = random(256);
      for(int j=0; j<8; j++) {
        Serial.print(!!((x[i]<<j) & 0x80));
      }
    }
  } else {
    Serial.println("TRUE KEY: ");
    for(int i=keySize-1; i>=0; i--) {
      x[i] = key[i];
      for(int j=0; j<8; j++) {
        Serial.print(!!((x[i]<<j) & 0x80));
      }
    }
  }
  Serial.println();
}
