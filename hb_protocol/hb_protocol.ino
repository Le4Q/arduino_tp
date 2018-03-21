//#include "./printf"

/** key length k (80 or 128) **/
const unsigned int secPar = 128;

/** key in {0,1}^k **/
boolean key[secPar];

/** epsilon in (0, 0.5), supports up to two decimals **/
float eps = 0.1;

/** iterations n **/
int n = 100;

void setup() {

  initializeKey();
  Serial.begin (9600) ;
  //printf_begin();
}

void loop() {
  Serial.print("Key of length ");
  Serial.print(secPar);
  Serial.println(": ");
  for(int i=0; i<secPar; i++) {
    Serial.print(key[i]);
  }
  Serial.println();

  hbTest();
  delay(2000);
}

/**
  HB protocol using either a random key or the true key with probability of 0.5 each.
**/
void hbTest() {

  /* counter for unsuccessful iteration */
  int counter=0;

  /* generate candidate key */
  boolean candidate[secPar];
  generateKey(&candidate[0]); //prints TRUE KEY or RANDOM KEY

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    /* random challenge a in {0,1}^k */
    boolean a[secPar];

    /* response z in {0, 1} */
    boolean z;

    /* choose random challenge a */
    for(int i=0; i<secPar; i++) {
      a[i] = (boolean) random(2);
    }

    /* get z as a*candidate XOR v */
    z = getZ(a, candidate, secPar);

    /* check if z = a*candidate XOR v ?= a*key */
    if(z != dotProduct(a, key, secPar)) {
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

/**
  Initializes the key during setup.
**/
void initializeKey(){
  for(int i=0; i<secPar; i++) {
    key[i] = (boolean) random(2);
  }
}

/**
  Computes the dot product of two vectors x and y in {0,1}^k mod 2.
**/
boolean dotProduct (boolean x[], boolean y[], unsigned int k) {
  boolean sum=0;

  for(int i=0; i<k; i++) {
    sum ^= x[i] & y[i];
  }

  return sum;
}

/**
  Computes a noisy dot product of two vectors x and y in {0,1}^k mod 2 with probability eps of the result bit being flipped.
**/
int getZ(boolean x[], boolean y[], unsigned int k){
  long rand = random (1, 100);
  boolean v = rand < eps*100;

  boolean product = dotProduct(x, y, k);

  return product^v;
}

/**
  Generates either a random key or true key with probability of 0.5 for each.
**/
void generateKey(boolean *x){

  long r = random(2);
  //static boolean x[secPar];
  if(r == 0) {
    Serial.println("RANDOM KEY: ");
    for(int i=0; i<secPar; i++) {
      x[i] = (boolean) random(2);
      Serial.print(x[i]);
    }
  }
  else {
    Serial.println("TRUE KEY: ");
    for(int i=0; i<secPar; i++) {
      x[i] = key[i];
      Serial.print(x[i]);
    }
  }
  Serial.println();
}
