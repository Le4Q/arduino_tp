 /*
 * Implementation of Lapin authentication protocol
 *
 * Ring based variant which has higher code size but requires less
 * computation time than field based version.
 * (Arduino is not particularly flash constrained)
 */

/* Security parameter */
const int secPar = 80;

/* reduced f */ // transform into 16 byte form?
const uint8_t f[5][5] = {{127, 8, 7, 3, 0}, {126, 9, 6, 5, 0}, {125, 9, 7, 4, 0},
                        {122, 7, 4, 3, 0}, {121, 8, 5, 1, 0}};

/* degree of f */
const int degf = 621;

/* epsilon in (0, 0.5), supports up to three decimals */
const float eps = 0.166666667;

/* acceptance threshold u in (epsilon, 0.5) */
const float u = 0.29;

/* ===================================================================== */

/* CRT representation of keys s1, s2 */
uint8_t s1[5][16], s2[5][16];

/* CRT representaion of f */
uint8_t f_crt[5][16];

void setup() {
  initializeKeys();
  initializeF();

  Serial.begin(9600);
  while(!Serial) {;}

  randomSeed(analogRead(0));

}

void loop() {

  lapinTest();

  delay(100);
}

/*
 * Maps array c to CRT representaion structure.
 */
void mapC(uint8_t result[5][16], uint8_t c[secPar/8]) // {0,1}^secPar -> R
{
  for(int i=0; i<5; i++) {
    for(int j=0; j<16; j++) {
      if(j<secPar/8)
        result[i][j] = c[j];
      else
        result[i][j] = 0;
    }
  }
}

void lapinTest()
{
  unsigned long temp1, temp2, time0, time1, time2, time3;

  // sample c
  uint8_t c[secPar/8];
  for(int i = 0; i<secPar/8; i++) {
    c[i] = random(256);
  }

  temp1 = millis();
  // map c to CRT form using pi
  uint8_t c_crt[5][16];
  mapC(c_crt, c);
  temp2 = millis(); time0=temp2-temp1;

  temp1 = millis();
  // sample r directly in CRT form
  uint8_t r_crt[5][16];
  for(int i=0; i<5; i++) {
    for(int j=0; j<15; j++) {
      r_crt[i][j] = random(256);
    }
    r_crt[i][15] = random(256 / pow(2, 128-f[i][0]));
    // resulting polynomial is of rank deg(f_i) - 1
  }
  temp2 = millis(); time1=temp2-temp1;

  temp1 = millis();
  // get e from ring according to Ber distribution
  uint8_t e[degf/8 + 1];
  for(int i=0; i<degf/8 + 1; i++) {
    for(int j=0; j<8; j++) {
      if(i*8+j < degf+1)
        setBit(&e[i], j, generateNoiseBit());
    }
  }

  // transform e into CRT form
  uint8_t e_crt[5][16];
  toCRT(e_crt, e, degf/8 + 1);
  temp2 = millis(); time2=temp2-temp1;

  temp1 = millis();
  // Calculate z
  uint8_t z_crt[5][16];
  uint8_t tmp1[5][16], tmp2[5][16], tmp3[5][16];
  sparsemult(tmp1, s1, c_crt);
  add(tmp2, tmp1, s2);
  mult(tmp3, r_crt, tmp2);
  add(z_crt, tmp3, e_crt);
  temp2 = millis(); time3 = temp2-temp1;

  Serial.println("Tagging done!");
  Serial.print("Map c to CRT form: ");
  Serial.println(time0);
  Serial.print("Sample r: ");
  Serial.println(time1);
  Serial.print("Sample e: ");
  Serial.println(time2);
  Serial.print("Tagging Time: ");
  Serial.println(time3);

}

/*
 * Initializes random keys s1 and s2.
 */
void initializeKeys()
{
  // sample keys directly in CRT form
  uint8_t r_crt[5][16];
  for(int i=0; i<5; i++) {
    for(int j=0; j<15; j++) {
      s1[i][j] = random(256);
      s2[i][j] = random(256);
    }
    s1[i][15] = random(256 / pow(2, 128-f[i][0]));
    s2[i][15] = random(256 / pow(2, 128-f[i][0]));
    // resulting polynomial is of rank deg(f_i) - 1

  }
}

/*
 * Initializes f_crt as CRT representation of f.
 */
void initializeF()
{
  for(int i=0; i<5; i++) {
    for(int j=0; j<16; j++) {
      f_crt[i][j] = 0;
    }
  }

  for(int i=0; i<5; i++) {
    for(int j=0; j<5; j++) {
      setBit(&f_crt[i][ f[i][j]/8 ], f[i][j]%8, 1);
    }
  }
}

/*
 * Adds two elements of Ring R in CRT representation.
 */
void add(uint8_t result[5][16], uint8_t a[5][16], uint8_t b[5][16])
{
  for(int i=0; i<5; i++) {
    for(int j=0; j<16; j++) {
      result[i][j] = a[i][j] ^ b[i][j];
    }
  }
}

/*
 * Multiplies two elements of Ring R in CRT representation.
 */
void mult(uint8_t result[5][16], uint8_t a[5][16], uint8_t b[5][16])
{
  for(int i=0; i<5; i++) {

    // a[i] * b[i] using right-to-left comb method

    // 1.
    uint8_t c[32] = {0};
    // 2.
    for(int k=0; k<8; k++) {
      // 2.1
      for(int j=0; j<16; j++) {
        if(getBit(a[i][j], k) == 1) {
          for(int p=0; p<16; p++) {
            c[j+p] ^= b[i][p];
          }
        }
      }
      // 2.2
      if(k != 7) {
        shiftLeft(b[i], 1, 16);
      }
    }
    // 3.
    uint8_t *result_ptr = polyMod(c, f_crt[i], 32, 16);
    for(int j=0; j<16; j++) {
      result[i][j] = result_ptr[j];
    }
  }
}

/*
 * Multiplies two elements of Ring R in CRT representation where only the
 * first secPar bits of b can be non-zero.
 */
void sparsemult(uint8_t result[5][16], uint8_t a[5][16], uint8_t b[5][16])
{
  for(int i=0; i<5; i++) {

    // a[i] * b[i] using right-to-left comb method

    // 1.
    uint8_t c[16+secPar/8] = {0};
    // 2.
    for(int k=0; k<8; k++) {
      // 2.1
      for(int j=0; j<16; j++) {
        if(getBit(a[i][j], k) == 1) {
          for(int p=0; p<secPar/8; p++) {
            c[j+p] ^= b[i][p];
          }
        }
      }
      // 2.2
      if(k != 7) {
        shiftLeft(b[i], 1, secPar/8);
      }
    }
    // 3.
    uint8_t *result_ptr = polyMod(c, f_crt[i], 16+secPar/8, 16);
    for(int j=0; j<16; j++) {
      result[i][j] = result_ptr[j];
    }
  }
}

/*
 * Transforms polynomial a into CRT representation using f. //TODO Test
 */
void toCRT(uint8_t result[5][16], uint8_t a[], size_t lenA)
{
  uint8_t **result_ptr = new uint8_t*[5]();
  for(int i=0; i<5; i++) {
    result_ptr[i] = new uint8_t[16]();
    result_ptr[i] = polyMod(a, f_crt[i], lenA, 16);
  }

  for(int i=0; i<5; i++) {
    for(int j=0; j<16; j++) {
      result[i][j] = result_ptr[i][j];
    }
  }

  for(int i=0; i<16; i++) {
    delete[] result_ptr[i];
  }
  delete[] result_ptr;
}

/*
 * Reduces polynomial n mod polynomial d and returns remainder polynomial.
 */
uint8_t* polyMod(uint8_t n[], uint8_t d[], size_t lenN, size_t lenD)
{
  uint8_t *result = new uint8_t();

  int degN = deg(n, lenN);
  int degD = deg(d, lenD);
  size_t len = degN/8+1;

  uint8_t n0[len] = {0};
  uint8_t d0[len] = {0}; //bigger array with values of d
  //int iter = 0;

  for(int i=0; i<len; i++) {
    n0[i] = n[i];
  }

  for(int iter = 0; iter <= degN-degD; iter++) {

    if(getBit(n0[len-1-(iter+(8-(degN+1)%8)%8)/8],
      7-(iter+(8-(degN+1)%8)%8)%8) == 1) {

        //Serial.println(deg(n0, len));

      int shift = degN-degD-iter;

      for(int i=0; i<len; i++) {
        d0[i] = 0;
      }

      // shift left by (K-N-i) / 8
      for(int i=0; i<lenD; i++){
        d0[i+shift/8] = d[i];
      }

      // shift by (K-N-i) % 8
      shiftLeft(d0, shift%8, len);

      // add d0 to n
      for(int i=0; i<len; i++) {
        n0[i] ^= d0[i];
        //Serial.print(n0[i]);
        //Serial.print(" ");
      }
      //Serial.println();
    }
  }

  //Serial.println(deg(n0, len));

  for(int i=0; i<lenD+1; i++) {
    result[i] = n0[i];
    //Serial.print(result[i]);
    //Serial.print(" ");
  }
  //Serial.println();

  return result;
}

/*
 * Returns degree of polynomial poly of length len.
 */
int deg(uint8_t poly[], size_t len)
{
  int degree = len*8-1;
  for(int i=len-1; i>=0 ; i--) {
    for(int j=7; j>=0; j--) {
      if(getBit(poly[i], j) != 0) {
        return degree;
      }
      degree--;
    }
  }

  return degree;
}

/*
 * Shifts array a of length len by x bits to the left, 0<x<8.
 */
uint8_t* shiftLeft(uint8_t a[], uint8_t x, size_t len)
{
  if(x > 0 && x < 8) {

    uint8_t bits1=0, bits2=0;
    uint8_t mask=0;

    for(int i=7; i>=8-x; i--) {
      mask += (uint8_t) (pow(2,i) + 0.5);
    }
    for(int i=0; i<len; i++) {
      bits2 = a[i] & mask;
      a[i] <<= x;
      a[i] |= bits1 >> 8-x;
      bits1 = bits2;
    }
  }
}

/*
 * Returns a noise bit according to Bernoulli distribution using eps.
 */
boolean generateNoiseBit()
{
  long rand = random(100);
  boolean v = rand < eps*100;

  return v;
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
