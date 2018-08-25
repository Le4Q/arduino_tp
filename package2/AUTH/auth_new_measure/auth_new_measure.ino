#define R maxChallenges
#define C keySize/2

/*
 * New Version of AUTH that sends several challenges at the same time.
 * Typical Parameters: l = 512, n = 256, eps = 0.2
 * maxChallenges = 1 - 7587 ms : 1539 + 2371 + 2371
 * maxChallenges = 5 - 4532 ms : 1534 + 933 + 933
 * maxChallenges = 8 - 4232 ms : 1526 + 794 + 790
 * maxChallenges = 12 - 4177 ms : 1545 + 736 + 735
 * maxChallenges = 15 - 4158 ms : 1536 + 721 + 720
 */

/* key length l of secret key s in {0,1}^2l */
const size_t l = 512;

/* key size in bytes */
const size_t keySize = (2*l)/8;

/* key s in {0,1}^2l */
uint8_t s[keySize];

/* parameter of Bernoulli distribution epsilon in (0, 0.5) */
float eps = 0.2;

/* acceptance threshold 1/4 + eps/2 */
float eps_threshold = 0.25 + eps/2;

/* number of iterations n <= l/2 */
const size_t n = 256;

/* number of challenges < n to send at a time  */
const unsigned maxChallenges = 15;

void setup() {

  initializeKey();
  Serial.begin(9600);
  while(!Serial) {;}

  Serial.println("============================================");
  Serial.print("Key of length ");
  Serial.print(2*l);
  Serial.println(": ");
  printBitString(&s[0], keySize);
}

void loop() {

  /* Average Time */
  long sum = 0;

  for(int i=0; i<10; i++) {
    unsigned long t1 = millis();
    authTest();
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
    //Serial.println(i);
  }
}

void authTest() {

  /* __________ VERIFIER __________ */

  /* Generate v */
  uint8_t v[keySize] = {0};

  generateFixedHamNumber(&v[0], l);

  // Abort if Hamming Weight of v is not l
  if(hammingWeight(v, keySize, keySize*8) != l) {
    Serial.println("Abort, Hamming Weight of v is not l.");
    return;
  }

  unsigned long time0 = 0, time1 = 0, time2 = 0, tmp1, tmp2;

  // Choose either random key or true key.
  uint8_t candidate[keySize];
  generateKey(&candidate[0]);

  int checksum = 0;

  for(int toSend = n; toSend > 0; toSend-=maxChallenges) {

    //Serial.println(toSend);

    size_t bytesToSend, bitsToSend;

    if(toSend > maxChallenges) {
      bitsToSend = maxChallenges;
      bytesToSend = maxChallenges/8 + (maxChallenges%8 != 0);
    }
    else {
      bitsToSend = toSend;
      bytesToSend = toSend/8 + (toSend%8 != 0);
    }

    /* __________ TAG __________ */

    // Compute R (R^T in the original version)
    tmp1 = millis();
    uint8_t r[bitsToSend][keySize/2];

    for(int i=0; i<bitsToSend; i++) {
      for(int j=0; j<keySize/2; j++) {
        r[i][j] = random(256);
      }
    }

    // Compute e as Ber_eps^n
    uint8_t e[bytesToSend];

    for(int i=0; i<bytesToSend; i++) {
      for(int j=0; j<(bitsToSend>=8 ? 8 : bitsToSend); j++) {
        boolean v = generateNoiseBit();
        setBit(&e[i], j, v);
      }
    }
    tmp2 = millis(); time0 += tmp2 - tmp1;

    // Compute candidate_v
    tmp1 = millis();
    uint8_t candidate_v[C];
    delete_v(candidate_v, candidate, v);

    // Compute z = R^T * candidate_v XOR e in {0,1}^m
    uint8_t z[bytesToSend] = {0};

    matrixVectorProduct(z, r, candidate_v);

    for(int i=0; i<bytesToSend; i++) {
      for(int j=0; j<(bitsToSend>=8 ? 8 : bitsToSend); j++) {
        setBit(&z[i], j, getBit(z[i], j) ^ getBit(e[i], j));
      }
    }
    tmp2 = millis(); time1 += tmp2 - tmp1;

    /* __________ VERIFIER __________ */

    /* Compute s_v in {0,1}^l which is derived from the key by deleting all
      entries s[i] where v[i] = 0.*/
    tmp1 = millis();
    uint8_t s_v[C];
    delete_v(s_v, s, v);

    // Verifier rejects if rank(R) != n or wt(z XOR R^T * s_v) > n*eps_threshold)
    uint8_t check[bytesToSend] = {0};

    matrixVectorProduct(check, r, s_v);

    for(int i=0; i<bytesToSend; i++) {
      check[i] ^= z[i];
    }
    checksum += hammingWeight(check, bytesToSend, bitsToSend);
    tmp2 = millis(); time2 += tmp2 - tmp1;
  }

  Serial.print("Sample R and e: ");
  Serial.println(time0);
  Serial.print("Tagging: ");
  Serial.println(time1);
  Serial.print("Verification: ");
  Serial.println(time2);

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
  Serial.println("============================================");
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
  Serial.println();
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
 * Returns Hamming Weight of boolean array x of length considering only the
 * first range bits.
 */
int hammingWeight(uint8_t x[], size_t length, int range) {
  int sum = 0;
  for(int i=0; i<length; i++) {
    for(int j = 0; j<8; j++) {
      if(i*8 + j == range) {
        return sum;
      }
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
        Serial.print("p = ");
        Serial.println(p);
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
