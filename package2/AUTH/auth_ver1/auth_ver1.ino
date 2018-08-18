#define R n
#define C keySize/2
#define nBytes n/8 + (n%8 != 0)

/* key length l of secret key s in {0,1}^2l */
const size_t l = 256;

/* key size in bytes */
const size_t keySize = (2*l)/8;

/* key s in {0,1}^2l */
uint8_t s[keySize];

/* parameter of Bernoulli distribution epsilon in (0, 0.5) */
float eps = 0.2;

/* acceptance threshold 1/4 + eps/2 */
float eps_threshold = 0.25 + eps/2;

/* number of parallel iterations n <= l/2 */
const size_t n = 128;

void setup() {
  initializeKey();
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

void authTest() {

  /* __________ VERIFIER __________ */

  /* Generate v */
  uint8_t v[keySize] = {0};

  generateFixedHamNumber(&v[0], l);

  /* __________ PROVER __________ */

  // Abort if Hamming Weight of v is not l
  if(hammingWeight(v, keySize) != l) {
    Serial.println("Abort, Hamming Weight of v is not l.");
    return;
  }

  // Choose either random key or true key.
  uint8_t candidate[keySize];
  generateKey(&candidate[0]);

  // Compute R (R^T in the original version)
  uint8_t r[R][C];

  for(int i=0; i<R; i++) {
    for(int j=0; j<C; j++) {
      r[i][j] = random(256);
    }
  }

  // Compute e as Ber_eps^n
  uint8_t e[nBytes];

  for(int i=0; i<nBytes; i++) {
    for(int j=0; j<8; j++) {
      boolean v = generateNoiseBit();
      setBit(&e[i], j, v);
    }
  }

  /* Compute candidate_v in {0,1}^l which is derived from the key candidate by
     deleting all entries candidate[i] where v[i] = 0.*/
  uint8_t candidate_v[C];
  delete_v(&candidate_v[0], candidate, v);

  /* Compute z = R^T * candidate_v XOR e in {0,1}^n with R^T in {0,1}^(nxl) and
    candidate_v in {0,1}^l. */
  uint8_t z[nBytes] = {0};

  matrixVectorProduct(&z[0], r, candidate_v);

  for(int i=0; i<nBytes; i++) {
    for(int j=0; j<8; j++) {
      setBit(&z[i], j, getBit(z[i], j) ^ getBit(e[i], j));
    }
  }

  /* __________ VERIFIER __________ */

  /* Compute s_v in {0,1}^l which is derived from the key by deleting all
    entries s[i] where v[i] = 0.*/
  uint8_t s_v[C];
  delete_v(&s_v[0], s, v);

  // Verifier rejects if rank(R) != n or wt(z XOR R^T * s_v) > n*eps_threshold)
  int checksum;
  uint8_t check[nBytes] = {0};

  matrixVectorProduct(&check[0], r, s_v);

  for(int i=0; i<nBytes; i++) {
    check[i] ^= z[i];
      //setBit(&check[i], j, getBit(check[i], j) ^ getBit(z[i], j));
  }

  checksum = hammingWeight(check, nBytes);

  // TODO rankOfMatrix
  /*if(rankOfMatrix(r) != n)
    Serial.println("REJECTED (rank error)");
  else */
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
 * Initializes the key during setup.
 */
void initializeKey()
{
  for(int i=0; i<keySize; i++) {
    s[i] = random(256);
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
  for(int i=k-1; i>=0; i--) {
    for(int j=0; j<8; j++) {
      Serial.print(getBit(x[i], 7-j));
    }
  }
}

void transposeMatrix(uint8_t result[C*8][R/8], uint8_t mat[R][C])
{
  for(int i=0; i<R; i++) {
    for(int j=0; j<C; j++) {
      for(int q=0; q<8; q++) {
        setBit(&result[j*8+q][i/8], 7-i%8, getBit(mat[i][j], 7-q));
      }
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
 * Fills an array x with random entries with hamming weight k.
 */
void generateFixedHamNumber(uint8_t *x, size_t k)
{
  for(int i = 0; i<k; i++) {
    int r = random(2*l);
    while(getBit(x[r/8], r%8) == 1) {
      r = random(2*l);
    }
    setBit(&x[r/8], r%8, 1);
  }
}

/*
 * Returns Hamming Weight of boolean array x.
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
 * Compute x_v in {0,1}^l which is derived from x by deleting all entries x[i]
 * where v[i] = 0.
 */
void delete_v(uint8_t *x_v, uint8_t x[], uint8_t v[]) {
  int p = 0;
  for(int i = 0; i<keySize; i++) {
    for(int j = 0; j<8; j++) {
      if(p > l) {
        Serial.println("SIZE ERROR");
        return;
      }
      if(getBit(v[i], j) != 0) {
        setBit(&x_v[p/8], p%8, getBit(x[i], j));
        p++;
      }
    }
  }
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
