
/* Curve parameter */
int a;
int b;



void setup() {


  /* Check EC condition*/
  if(4*a*a*a + 27*b*b == 0) {
    Serial.println("Curve parameters do not satisfy condition!");
  }
}

void loop() {

}

void ecracTest() {

}

class Point {
  int x, y;
public:
  Point();
  Point(int a, int b);
  Point add(Point a, Point b);
  Point doub(Point);
  Point mult(Point a, int x);
};

Point::Point() {
  x = 0;
  y = 0;
}

Point::Point(int a, int b) {
  x = a;
  y = b;
}

Point Point::add(Point j, Point k) {
  Point l;
  int s = (j.y  -k.y)/ (j.x - k.x);
  l.x = s*s - j.x - k.x;
  l.y = -j.y + s*(j.x - l.x);

  return l;
}

Point Point::doub(Point j) {
  Point l;
  int s = (3*j.x*j.x + a) / (2*j.y);
  l.x = s*s - 2*j.x;
  l.y = -j.y + s*(j.x - l.x);

  return l;

}

Point Point::mult(Point j, int x) {
  Point l;
  Point n = j;
  Point q;

  return l;
}
