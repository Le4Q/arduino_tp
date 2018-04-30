#define R l
#define C n

/* key length l of secret key s in {0,1}^2l */
const int l = 40;

/* key s in {0,1}^2l */
boolean s[2*l];

/* parameter of Bernoulli distribution epsilon in (0, 0.5) */
float eps = 0.2;

/* acceptance threshold 1/4 + eps/2 */
float eps_threshold = 0.25 + eps/2;

/* number of parallel iterations n <= l/2 */
const int n = 14;


void setup() {

  initializeKey();
  Serial.begin(9600);

}

void loop() {

  Serial.println("============================================");
  Serial.print("Key of length ");
  Serial.print(2*l);
  Serial.println(": ");
  for(int i=0; i<2*l; i++) {
    Serial.print(s[i]);
  }
  Serial.println();

  authTest();
  delay(1000);

}

//TODO implement memory allocation for nested loop to compute R
void authTest() {

  /* __________ VERIFIER __________ */

  /* Generate v */
  boolean v[2*l] = {0};

  generateFixedHamNumber(&v[0], l);

  /* __________ PROVER __________ */

  /* Abort if Hamming Weight of v is not l */
  if(hammingWeight(v, 2*l) != l) {
    Serial.println("Abort, Hamming Weight of v is not l.");
    return;
  }

  /* Choose either random key or true key. */
  boolean candidate[2*l];
  generateKey(&candidate[0]);

  /* Compute R and R^T */
  boolean r[l][n];

  for(int i=0; i<l; i++) {
    for(int j=0; j<n; j++) {
      r[i][j] = random(2);
    }
  }

  boolean rt[n][l];

  transposeMatrix(rt, r);

  /* Compute candidate_v in {0,1}^l which is derived from the key candidate by
    deleting all entries candidate[i] where v[i] = 0.*/

  boolean candidate_v[l];
  delete_v(&candidate_v[0], candidate, v);

  /* Compute z = R^T * candidate_v XOR e in {0,1}^n with R^T in {0,1}^(nxl) and
    candidate_v in {0,1}^l. */
  boolean z[n] = {0};

  matrixVectorProduct(&z[0], rt, candidate_v);

  for(int i=0; i<n; i++) {
    boolean e = generateNoiseBit();
    z[i] ^= e;
  }

  /* __________ VERIFIER __________ */

  /* Compute s_v in {0,1}^l which is derived from the key by deleting all
    entries s[i] where v[i] = 0.*/

  boolean s_v[l];
  delete_v(&s_v[0], s, v);

  /* Verifier rejects if rank(R) != n or wt(z XOR R^T * s_v) > n*eps_threshold) */
  int checksum;
  boolean check[n] = {0};

  matrixVectorProduct(&check[0], rt, s_v);

  for(int i=0; i<n; i++) {
    check[i] ^= z[i];
  }

  checksum = hammingWeight(check, n);

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

/*
 * Initializes a random secret key s.
 */
void initializeKey(){
  for(int i=0; i<2*l; i++) {
    s[i] = (boolean) random(2);
  }
}

/*
 * Generates either a random key or true key (=s) with probability of 0.5 for each.
 */
void generateKey(boolean *x){

  long r = random(2);

  if(r == 0) {
    Serial.println("Candidate key (RANDOM): ");
    for(int i=0; i<2*l; i++) {
      x[i] = (boolean) random(2);
      Serial.print(x[i]);
    }
  }
  else {
    Serial.println("Candidate key (TRUE): ");
    for(int i=0; i<2*l; i++) {
      x[i] = s[i];
      Serial.print(x[i]);
    }
  }
  Serial.println();
}

/*
 * Fills a boolean array x with random entries with hamming weight k.
 */
void generateFixedHamNumber(boolean *x, int k){
  for(int i = 0; i<k; i++) {
    int r = random(2*l);
    while(x[r] == 1) {
      r = random(2*l);
    }
    x[r] = 1;
  }
}

/*
 * Returns Hamming Weight of boolean array x.
 */
int hammingWeight(boolean x[], int length) {
  int sum = 0;
  for(int i=0; i<length; i++) {
    sum += x[i];
  }
  return sum;
}

/*
 * Transposes boolean matrix mat and stores result in boolean matrix result.
 */
void transposeMatrix(boolean result[C][R], boolean mat[R][C]) {
  for(int i=0; i<R; i++) {
    for(int j=0; j< C; j++) {
      result[j][i] = mat[i][j];
    }
  }
}

/*
 * Computes Matrix-Vector-Product of boolean matrix mat and boolean vector R
 * and stores it in boolean vector result.
 */
void matrixVectorProduct(boolean *result, boolean mat[C][R], boolean vect[R]) {
  for(int i = 0; i<C; i++) {
    for(int j = 0; j<R; j++) {
      result[i] ^= mat[i][j] & vect[j];
    }
  }
}

/*
 * Compute x_v in {0,1}^l which is derived from x by deleting all entries x[i]
 * where v[i] = 0.
 */
void delete_v(boolean *x_v, boolean x[], boolean v[]) {
  int p = 0;
  for(int i = 0; i<2*l; i++) {
    if(p>l) {
      Serial.println("ERROR (size)");
    }
    if(v[i] != 0) {
      x_v[p] = x[i];
      p++;
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

/*
 * Exchanges two rows of a matrix. Courtesy of
 * https://www.geeksforgeeks.org/program-for-rank-of-matrix/
 */
void swap(boolean mat[R][C], int row1, int row2, int col)
{
    for (int i = 0; i < col; i++)
    {
        int temp = mat[row1][i];
        mat[row1][i] = mat[row2][i];
        mat[row2][i] = temp;
    }
}

/*
 *Returns rank of a matrix. Courtesy of
 * https://www.geeksforgeeks.org/program-for-rank-of-matrix/
 */
int rankOfMatrix(boolean mat[R][C])
{
    int rank = C;

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
                 double mult = (double)mat[col][row] /mat[row][row];
                 for (int i = 0; i < rank; i++)
                   mat[col][i] -= mult * mat[row][i];
              }
           }
        }

        /* Diagonal element is already zero. Two cases arise:
        1) If there is a row below it with non-zero
           entry, then swap this row with that row
           and process that row
        2) If all elements in current column below
           mat[r][row] are 0, then remvoe this column
           by swapping it with last column and
           reducing number of columns by 1. */
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
    }
    return rank;
}
