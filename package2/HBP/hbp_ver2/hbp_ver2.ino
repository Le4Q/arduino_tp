
/* key length k (80 or 128) */
const size_t secPar = 80;

/* key size in Bytes */
const size_t keySize = secPar/8;

/* keys in {0,1}^k */
uint8_t key1[keySize], key2[keySize];

/* epsilon in (0, 0.5), supports up to two decimals */
float eps = 0.1;

/* iterations n */
unsigned n = 100;

void setup() {

  initializeKey();
  Serial.begin (9600) ;

}

void loop() {
  Serial.print("Key 1 of length ");
  Serial.print(secPar);
  Serial.println(": ");
  for(int i=keySize-1; i>=0; i--) {
    for(int j=0; j<8; j++) {
      Serial.print(!!((key1[i]<<j) & 0x80));
    }
  }

  Serial.println();

  Serial.print("Key 2 of length ");
  Serial.print(secPar);
  Serial.println(": ");
  for(int i=keySize-1; i>=0; i--) {
    for(int j=0; j<8; j++) {
      Serial.print(!!((key2[i]<<j) & 0x80));
    }
  }

  Serial.println();

  hbTest();
  delay(1000);
}

/*
  HB protocol using either a random key or the true key with probability of 0.5 each.
*/
void hbTest()
{

  /* counter for unsuccessful iteration */
  int counter=0;

  /* generate candidate key */
  uint8_t candidate1[keySize], candidate2[keySize];
  generateKey1(&candidate1[0]); //prints TRUE KEY or RANDOM KEY
  generateKey2(&candidate2[0]);

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    /* random blinding factor b in {0,1}^k */
    uint8_t b[keySize];

    /* random challenge a in {0,1}^k */
    uint8_t a[keySize];

    /* response z in {0, 1} */
    boolean z;

    /* choose random blinding factor b */
    for(int i=0; i<keySize; i++) {
      b[i] = random(256);
    }

    /* choose random challenge a */
    for(int i=0; i<keySize; i++) {
      a[i] = random(256);
    }

    /* get z as candidate1*b XOR candidate2*a XOR v */
    z = dotProduct(candidate1, b, keySize)^dotProduct(candidate2, a, keySize)^generateNoiseBit();

    /* check if z = candidate1*b XOR candidate2*a XOR v ?= key1*b XOR key2*a */
    if(z != dotProduct(key1, b, keySize)^dotProduct(key2, a, keySize)) {
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
 * Initializes the key during setup.
 */
void initializeKey()
{
  for(int i=0; i<keySize; i++) {
    key1[i] = random(256);
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
     sum ^= !!((x[i]<<j) & 0x80) & !!((y[i]<<j) & 0x80);
   }
 }

 return sum;
}

boolean generateNoiseBit() {
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
    for(int i=keySize-1; i>=0; i--) {
      x[i] = random(256);
      for(int j=0; j<8; j++) {
        Serial.print(!!((x[i]<<j) & 0x80));
      }
    }
  }
  else {
    Serial.println("TRUE KEY 1: ");
    for(int i=keySize-1; i>=0; i--) {
      x[i] = key1[i];
      for(int j=0; j<8; j++) {
        Serial.print(!!((x[i]<<j) & 0x80));
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
    for(int i=keySize-1; i>=0; i--) {
      x[i] = random(256);
      for(int j=0; j<8; j++) {
        Serial.print(!!((x[i]<<j) & 0x80));
      }
    }
  }
  else {
    Serial.println("TRUE KEY 2: ");
    for(int i=keySize-1; i>=0; i--) {
      x[i] = key2[i];
      for(int j=0; j<8; j++) {
        Serial.print(!!((x[i]<<j) & 0x80));
      }
    }
  }
  Serial.println();
}
