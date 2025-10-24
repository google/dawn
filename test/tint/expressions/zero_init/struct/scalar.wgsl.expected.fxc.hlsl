struct S {
  int i;
  uint u;
  float f;
  bool b;
};


[numthreads(1, 1, 1)]
void f() {
  S v = (S)0;
}

