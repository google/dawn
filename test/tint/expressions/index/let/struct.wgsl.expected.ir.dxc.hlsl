struct S {
  int m;
  uint n;
};


uint f() {
  S v = (S)0;
  S a = v;
  return a.n;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

