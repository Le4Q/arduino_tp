

/*
 * Version 2 with uint8_t arrays as key, saves space compared to boolean array
 *
 * Suggested parameter values (Armknecht et. al):
 * 1: k = 512, e = 0.25, u = 0.348, n = 1164 ---- 12 seconds
 * 2: k = 512, e = 0.125, u = 0.256, n = 441 ---- 5 seconds
 * 3: k = 512, e = 0.125, u = 0.1875, n = 256 ---- //TODO implement version3
 */

/* key length k */
const size_t k = 512;

/* key size in Bytes */
const size_t keySize = k/8; // = 64 for k = 512

/* key in {0,1}^k */
uint8_t key[keySize];

/* epsilon in (0, 0.5), supports up to three decimals*/
float eps = 0.25;

/* acceptance threshold u in (epsilon, 0.5) */
float u = 0.348;

/* iterations n*/
unsigned n = 1164;

unsigned long time;

void setup() {

  initializeKey();
  Serial.begin (9600) ;
  while(!Serial) {;}
}

void loop() {
  Serial.print("Key of length ");
  Serial.print(k);
  Serial.println(": ");
  for(int i=keySize-1; i>=0; i--) {
    for(int j=0; j<8; j++) {
      Serial.print(!!((key[i]<<j) & 0x80));
    }
  }
  Serial.println();

  /* Average Time */
  int sum = 0;

  for(int i=0; i<10; i++) {
//    time_t t1 = now();
    hbTest();
  //  time_t t2 = now();
  //  sum += t2 - t1;
  }
Serial.print("Time in micro: ");
time = micros();

Serial.println(time);
  
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

  /* TAG: generate candidate key */
  uint8_t candidate[keySize];
  generateKey(&candidate[0]); //prints TRUE KEY or RANDOM KEY

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    uint8_t a[keySize]; // random challenge a in {0,1}^k

    boolean z; // response z in {0, 1}

    /* READER: choose random challenge a */
    for(int i=0; i<keySize; i++) {
      a[i] = random(256);
    }

    /* TAG: get z as candidate*a XOR v */
    z = getZ(candidate, a, keySize);

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
Serial.print("Time in micro: ");
time = micros();

Serial.println(time);

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
