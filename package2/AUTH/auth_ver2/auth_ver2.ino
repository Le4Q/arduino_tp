#define R n
#define C keySize
#define nBytes n/8 + (n%8 != 0)

/*
 * Suggested parameter values:
 * l = 512, n = 250, choose eps freely ---- xxx seconds
 */

/* key length l of secret key s in {0,1}^2l */
const size_t l = 112;

/* number of parallel iterations n <= l/2 */
const unsigned n = 48;

/* parameter of Bernoulli distribution epsilon in (0, 0.5) */
const float eps = 0.1;

/* acceptance threshold 1/4 + eps/2 */
const float eps_threshold = 0.25 + eps/2;

/* ===================================================================== */

/* key size in bytes */
const size_t keySize = (2*l)/8;

/* key s in {0,1}^2l */
uint8_t s[keySize];

/* blinding values */
uint8_t bv[keySize];
uint8_t bz[nBytes];

void setup() {
  initializeKey();
  randomSeed(analogRead(A0));
  Serial.begin(9600);

}

void loop() {

  Serial.println("============================================");
  Serial.print("Key of length ");
  Serial.print(2*l);
  Serial.println(": ");
  printBitString(&s[0], keySize);
  Serial.println();

  authTest();
  delay(1000);

}

void authTest() {

  /* __________ VERIFIER __________ */

  /* Generate v */
  uint8_t v[keySize] = {0};

  for(int i = 0; i<keySize; i++) {
    v[i] = random(256);
  }

  /* __________ PROVER __________ */

  /* Choose either random key or true key. */
  uint8_t candidate[keySize];
  generateKey(&candidate[0]);

  /* Compute R (R^T in the original version) and e */
  uint8_t r[n][keySize];
  uint8_t e[nBytes];

  for(int i=0; i<n; i++) {
    for(int j=0; j<keySize; j++) {
      r[i][j] = random(256);
    }
  }

  for(int i = 0; i<nBytes; i++) {
    for(int j = 0; j<8; j++) {
      setBit(&e[i], j, generateNoiseBit());
    }
  }

  /* Compute s AND (v XOR bv).*/
  uint8_t temp[keySize];
  uint8_t z[nBytes] = {0};

  for(int i=0; i<keySize; i++) {
    temp[i] = candidate[i] & (v[i] ^ bv[i]);
  }

  /* Compute z = R^T * temp1 XOR bz XOR e */
  matrixVectorProduct(&z[0], r, temp);

  for(int i=0; i<(nBytes); i++) {
    z[i] ^= bz[i]^e[i];
  }

  /* __________ VERIFIER __________ */

  /* Verifier rejects if wt(R^T * (s AND (v XOR bv)) XOR bz) > n*eps_threshold */
  int checksum;
  uint8_t check[nBytes] = {0};

  for(int i=0; i<keySize; i++) {
    temp[i] = s[i] & (v[i] ^ bv[i]);
  }

  matrixVectorProduct(&check[0], r, temp);

  for(int i=0; i<nBytes; i++) {
    check[i] ^= bz[i]^z[i];
      //setBit(&check[i], j, getBit(check[i], j) ^ getBit(z[i], j));
  }

  checksum = hammingWeight(check, nBytes);

  if(checksum > n*eps_threshold) {
    Serial.println("REJECTED (threshold error)");
    Serial.print("Weight: ");
    Serial.print(checksum);
    Serial.print(" > ");
    Serial.println(n*eps_threshold);
  }
  else {
    Serial.println("ACCEPTED");
    Serial.print("Weight: ");
    Serial.print(checksum);
    Serial.print(" < ");
    Serial.println(n*eps_threshold);
  }

}

/*
 * Initializes the key and blinding vectors during setup.
 */
void initializeKey()
{
  for(int i=0; i<keySize; i++) {
    s[i] = random(256);
    bv[i] = random(256);
  }
  for(int i=0; i<nBytes; i++) {
    bz[i] = random(256);
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
 * Prints bit Strings (also for debugging purposes).
 */
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
void matrixVectorProduct (uint8_t *result, uint8_t mat[R][C], uint8_t vect[C])
{
  size_t count = 0;
  for(int i = 0; i<R; i++) {

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
      x[i] = s[i];
    }
    printBitString(&x[0], keySize);
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
