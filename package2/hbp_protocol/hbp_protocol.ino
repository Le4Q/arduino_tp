//#include "./printf"

/** key length k (80 or 128) **/
const unsigned int secPar = 128;

/** keys in {0,1}^k **/
boolean key1[secPar], key2[secPar];

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
  Serial.print("Key 1 of length ");
  Serial.print(secPar);
  Serial.println(": ");
  for(int i=0; i<secPar; i++) {
    Serial.print(key1[i]);
  }

  Serial.println();

  Serial.print("Key 2 of length ");
  Serial.print(secPar);
  Serial.println(": ");
  for(int i=0; i<secPar; i++) {
    Serial.print(key2[i]);
  }

  Serial.println();

  hbTest();
  delay(1000);
}

/**
  HB protocol using either a random key or the true key with probability of 0.5 each.
**/
void hbTest() {

  /* counter for unsuccessful iteration */
  int counter=0;

  /* generate candidate key */
  boolean candidate1[secPar], candidate2[secPar];
  generateKey1(&candidate1[0]); //prints TRUE KEY or RANDOM KEY
  generateKey2(&candidate2[0]);

  /* n iterations of HB */
  for(int i=0; i<n; i++) {

    /* random blinding factor b in {0,1}^k */
    boolean b[secPar];

    /* random challenge a in {0,1}^k */
    boolean a[secPar];

    /* response z in {0, 1} */
    boolean z;

    /* choose random blinding factor b */
    for(int i=0; i<secPar; i++) {
      b[i] = (boolean) random(2);
    }

    /* choose random challenge a */
    for(int i=0; i<secPar; i++) {
      a[i] = (boolean) random(2);
    }

    /* get z as candidate1*b XOR candidate2*a XOR v */
    z = dotProduct(candidate1, b, secPar)^dotProduct(candidate2, a,secPar)^generateNoiseBit();

    /* check if z = candidate1*b XOR candidate2*a XOR v ?= key1*b XOR key2*a */
    if(z != dotProduct(key1, b, secPar)^dotProduct(key2, a, secPar)) {
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
    key1[i] = (boolean) random(2);
    key2[i] = (boolean) random(2);
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

boolean generateNoiseBit() {
  long rand = random(100);
  boolean v = rand < eps*100;

  return v;
}

/**
  Generates either a random key1 or true key1 with probability of 0.5 for each.
**/
void generateKey1(boolean *x){

  long r = random(2);
  //static boolean x[secPar];
  if(r == 0) {
    Serial.println("RANDOM KEY 1: ");
    for(int i=0; i<secPar; i++) {
      x[i] = (boolean) random(2);
      Serial.print(x[i]);
    }
  }
  else {
    Serial.println("TRUE KEY 1: ");
    for(int i=0; i<secPar; i++) {
      x[i] = key1[i];
      Serial.print(x[i]);
    }
  }
  Serial.println();
}

/**
  Generates either a random key2 or true key2 with probability of 0.5 for each.
**/
void generateKey2(boolean *x){

  long r = random(2);
  //static boolean x[secPar];
  if(r == 0) {
    Serial.println("RANDOM KEY 2: ");
    for(int i=0; i<secPar; i++) {
      x[i] = (boolean) random(2);
      Serial.print(x[i]);
    }
  }
  else {
    Serial.println("TRUE KEY 2: ");
    for(int i=0; i<secPar; i++) {
      x[i] = key2[i];
      Serial.print(x[i]);
    }
  }
  Serial.println();
}
