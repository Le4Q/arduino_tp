#define R l
#define C n
#define keySize 2*l

/* Parameters */

/* length of l in bits */
const size_t l_bits = 256;

/* parameter of Bernoulli distribution epsilon in (0, 0.5) */
const float eps = 0.2;

/* acceptance threshold 1/4 + eps/2 */
const float eps_threshold = 0.25 + eps/2;

/* number of parallel iterations n <= l/2 */
const unsigned n = 16;

/* length of randomness in bits */
const size_t v_bits = 128;

/* output length of hash function */
// const unsigned my = 32;

/* =============================================== */

/* l in bytes */
const size_t l = l_bits/8;

/* v in bytes */
const size_t v = v_bits/8;

/* key s in {0,1}^2l */
uint8_t s[2*l];

/* output length of hash function */
// const unsigned my = my_bits/8;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

void doMAC1()
{

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

// example hash function (knuth)
uint32_t hash(unsigned x, int p) {
    if(p < 0 || p > 32) {
      Serial.println("ERROR: p has to be in (0,32)");
      return 0;
    }

    const uint32_t knuth = 2654435769;
    const uint32_t y = x;
    return (y * knuth) >> (32 - p);
}

void permute()
{

}

void tag()
{
  // 1. Get R, R^T, b
  uint8_t r[l_bits][n], rt[n*8][l], b[v];

  for(int i=0; i<l_bits; i++) {
    for(int j=0; j<n; j++) {
      r[i][j] = random(256);
    }
  }

  transposeMatrix(rt, r);

  for(int i=0; i<v; i++) {
    b[i] = random(256);
  }

  // 2. get v as encoded hash of m and b
  uint8_t v[2*l];
  // TODO v = ...

  // 3. get tag
  uint8_t s_v[l];
  uint8_t z[n/8 + (n%8 != 0)] = {0};

  delete_v(&s_v[0], s, v);
  matrixVectorProduct(&z[0], rt, s_v);

  for(int i=0; i<(n/8 + (n%8 != 0)); i++) {
    boolean e = generateNoiseBit();
    z[i] ^= e;
      //setBit(&z[i], j, getBit(z[i], j) ^ e);
  }

  // TODO concatenate and permute

}

void verify()
{
  // 1. parse tag and check rank(R)
  // TODO

  // 2. get v
  // TODO v =  ...

  //3. check Weight
  int checksum;
  uint8_t check[R/8 + (R%8 != 0)] = {0};

  matrixVectorProduct(&check[0], rt, s_v);

  for(int i=0; i<R; i++) {
    check[i] ^= z[i];
      //setBit(&check[i], j, getBit(check[i], j) ^ getBit(z[i], j));
  }

  checksum = hammingWeight(check, R/8 + (R%8 != 0));

  // TODO rankOfMatrix
  if(rankOfMatrix(r) != n)
    Serial.println("REJECTED (rank error)");
  else if(checksum > n*eps_threshold) {
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

void cEncode(uint8_t *result, uint32_t x)
{
  // Z_2^32 -> Z_2^2l
  for(int i=0; i<l; i++) {
    uint8_t pos = ?; //TODO
    setBit(result, pos, 1);
  }
}

/*
 * Returns Hamming Weight of array x.
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
 * Computes Matrix-Vector-Product of boolean matrix mat and boolean vector R
 * and stores it in boolean vector result.
 */
void matrixVectorProduct (uint8_t *result, uint8_t mat[R][C], uint8_t vect[C])
{
  for(int i = 0; i<R; i++) {
    for(int j = 0; j<C; j++) {
      for(int q = 0; q<8; q++) {
        setBit(&result[i], q, getBit(result[i], q) ^ getBit(mat[i][j], q)
                            & getBit(vect[j], q));
      }
    }
  }
}

boolean generateNoiseBit() {
  long rand = random(100);
  boolean v = rand < eps*100;

  return v;
}

void swap(boolean mat[R][C], int row1, int row2,
          int col)
{
    for (int i = 0; i < col; i++)
    {
        int temp = mat[row1][i];
        mat[row1][i] = mat[row2][i];
        mat[row2][i] = temp;
    }
}

/*
 * Returns rank of a matrix. Courtesy of
 * https://www.geeksforgeeks.org/program-for-rank-of-matrix/
 */
int rankOfMatrix(uint8_t mat[R][C])
{
   int rank = 0;

   for (int row = 0; row < rank; row++)
   {
       // Before we visit current row 'row', we make
       // sure that mat[row][0],....mat[row][row-1]
       // are 0.

       // Diagonal element is not zero
       if (mat[row][row])
       {
          for (int col = 0; col < R; col++)
          {
              if (col != row)
              {
                // This makes all entries of current
                // column as 0 except entry 'mat[row][row]'
                double mult = (double)mat[col][row] /
                                      mat[row][row];
                for (int i = 0; i < rank; i++)
                  mat[col][i] -= mult * mat[row][i];
             }
          }
       }

       // Diagonal element is already zero. Two cases
       // arise:
       // 1) If there is a row below it with non-zero
       //    entry, then swap this row with that row
       //    and process that row
       // 2) If all elements in current column below
       //    mat[r][row] are 0, then remvoe this column
       //    by swapping it with last column and
       //    reducing number of columns by 1.
       else
       {
           bool reduce = true;

           /* Find the non-zero element in current
               column  */
           for (int i = row + 1; i < R;  i++)
           {
               // Swap the row with non-zero element
               // with this row.
               if (mat[i][row])
               {
                   swap(mat, row, i, rank);
                   reduce = false;
                   break ;
               }
           }

           // If we did not find any row with non-zero
           // element in current columnm, then all
           // values in this column are 0.
           if (reduce)
           {
               // Reduce number of columns
               rank--;

               // Copy the last column here
               for (int i = 0; i < R; i ++)
                   mat[i][row] = mat[i][rank];
           }

           // Process this row again
           row--;
       }

      // Uncomment these lines to see intermediate results
      // display(mat, R, C);
      // printf("\n");
   }
   return rank;
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
