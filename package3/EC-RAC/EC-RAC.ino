#include "BigNumber.h"

/* Curve parameters y^2 = x^3 + ax^2 + x mod p (NIST P-224)*/
const BigNumber _a = 3;
const BigNumber _b = "18958286285566608000408668544493926415504680968679321075787234672564";
const BigNumber _p = "26959946667150639794667015087019630673557916260026308143510066298881";

void setup() {

  Serial.begin(9600);
  while(!Serial) {;}
  BigNumber::begin ();

}

void loop() {

  ecracTest();

  delay(100);
}

class Point {

  BigNumber _x;
  BigNumber _y;

public:

  Point()
  {
    _x = 0;
    _y = 0;
  }

  Point(BigNumber x, BigNumber y)
  {
    _x = x;
    _y = y;
  }

  Point add(Point k)
  {
    if(this->_x == 0 && this->_y == 0) {
      return Point(k._x, k._y);
    }

    if(k._x == 0 && k._y == 0) {
      return Point(this->_x, this->_y);
    }

    if(this->_y == (-k._y)) {
      return Point(0, 0);
    }

    Point l(0, 0);

    int s = (this->_y - k._y) / (this->_x - k._x);

    if(s == 0) {
      l._x = l._y = 0;
    }
    else {
      l._x = s*s - this->_x - k._x;
      l._y = -this->_y + s*(this->_x - l._x);
    }

    return l;
  }

  Point doub()
  {
    Point l;
    int s = (3*this->_x*this->_x + _a) / (2*this->_y);
    l._x = s*s - 2*this->_x;
    l._y = -this->_y + s*(this->_x - l._x);

    return l;
  }

  Point mult(BigNumber k)
  {
    Point n = *this;
    Point q = Point();
    int i = 0;

    while( k ) {

      if(k & 1) {
        q = q.add(n);
      }

      n = n.doub();
      k >>= 1;
      i++;
    }

    return q;
  }

};

void ecracTest() {

  /* Check EC condition*/
  if(4*_a.pow(3) + 27*_b*_b == 0) {
    Serial.println("Curve parameters do not satisfy condition!");
  }

  /* generator P */
  Point P_gen = Point("19277929113566293071110308034699488026831934219452440156649784352033",
  "19926808758034470970197974370888749184205991990603949537637343198772");

  /* Verifier input */
  BigNumber x1 = 1;
  BigNumber x2 = 1;
  Point Y = Point();

  /* VERIFIER */
  BigNumber r2 = random(2147483647);

  /* TAG */
  BigNumber r1 = random(2147483647);

  if(r2 == 0) {
    Serial.println("HALT");
    return;
  }

  Point T1 = P_gen.mult(r1);
  Point T2 = Y.mult(r1+x1);
  BigNumber v = r1*x1 + r2*x2;

  Serial.println("Tagging done!");
}
