/*
 * Not a working version. Just here to show that we actually tried :)
 */

#include "BigNumber.h"

/* Curve parameters y^2 = x^3 + ax^2 + x mod p (NIST P-224)*/
const BigNumber _a = -3;
const BigNumber _b = "18958286285566608000408668544493926415504680968679321075787234672564";
const BigNumber _p = "26959946667150639794667015087019630673557916260026308143510066298881";


BigNumber modInverse(BigNumber a, BigNumber m)
{
  a = a%m;
  for (BigNumber x=1; x<m; x++)
    if ((a*x) % m == 1)
      return x;
}

class Point
{
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

  BigNumber getX()
  {
    return this->_x;
  }

  BigNumber getY()
  {
    return this->_y;
  }

  Point add(Point k)
  {
    if(this->_x == 0 && this->_y == 0) {
      return Point(k._x, k._y);
    }

    if(k._x == 0 && k._y == 0) {
      return Point(this->_x, this->_y);
    }

    if(this->_y == (-(k._y))) {
      return Point(0, 0);
    }

    Point l = Point(0, 0);

    BigNumber s = (this->_y - k._y) * modInverse(this->_x - k._x, _p);
    Serial.println(s);

    if(s == 0) {
      l._x = l._y = 0;
    }
    else {
      l._x = s*s - this->_x - k._x;
      l._y = s*(this->_x - l._x) - this->_y;
    }

    //return l;
    BigNumber x_red = (l._x % _p + _p) % _p;
    BigNumber y_red = (l._y % _p + _p) % _p;
    return Point(x_red, y_red);
  }

  Point doub()
  {
    Point l;
    BigNumber s = (3*this->_x*this->_x + _a) / (2*this->_y);
    l._x = s*s - BigNumber(2)*this->_x;
    l._y = -this->_y + s*(this->_x - l._x);

    return l; //Point(l._x % _p, l._y % _p);
  }

  Point mult(BigNumber k)
  {
    Point n = *this;
    Point q = Point(0,0);

    BigNumber d = k;

    while(d != 0)
    {
      BigNumber quo, rem;
      d.divMod(2, quo, rem);

      Serial.println(quo);
      Serial.println(rem);

      if( d % BigNumber(2) == 1) {
        q = q.add(n);
      }

      Point n = n.doub();
      d /= 2;
    }

    return q; //Point(q._x % _p, q._y % _p);
  }

};

void setup()
{
  BigNumber::begin ();
  Serial.begin(9600);
  while(!Serial) {;}

  randomSeed(analogRead(0));
}

void loop()
{
  //ecracTest();

  Point aa = Point(BigNumber("19277929113566293071110308034699488026831934219452440156649784352033"),
  BigNumber("19926808758034470970197974370888749184205991990603949537637343198772"));
  Point bb = Point(BigNumber(2), BigNumber(2));
  Point cc = aa.add(bb);
  Serial.println("OK3");
  Serial.println(cc.getX());
  Serial.println(cc.getY());

  delay(100);
}

/* generator P */
const Point P_gen = Point(BigNumber("19277929113566293071110308034699488026831934219452440156649784352033"),
BigNumber("19926808758034470970197974370888749184205991990603949537637343198772"));

void ecracTest() {

  /* Check EC condition*/
  if(4*_a.pow(3) + 27*_b*_b == 0) {
    Serial.println("Curve parameters do not satisfy condition!");
  }

  /* Verifier input */
  BigNumber x1 = 1;
  BigNumber x2 = 1;
  Point Y = Point();

  /* VERIFIER */
  //long rand2 = random(1, 2147483647);
  BigNumber r2 = BigNumber("164");

  /* TAG */
  //long rand1 = random(1, 2147483647);
  BigNumber r1 = BigNumber("11243416227976667897643");

  if(r2 == 0) {
    Serial.println("HALT");
    return;
  }

  Serial.println(r1);

  Point T1 = P_gen.mult(r1);
  Point T2 = Y.mult(r1+x1);
  BigNumber v = r1*x1 + r2*x2;

  Serial.println("Tagging done!");
}
