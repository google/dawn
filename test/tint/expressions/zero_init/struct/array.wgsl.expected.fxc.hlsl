struct S {
  float a[4];
};


[numthreads(1, 1, 1)]
void f() {
  S v = (S)0;
}

