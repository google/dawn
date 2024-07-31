struct S {
  int m;
  uint n[4];
};


uint f() {
  S v[2] = (S[2])0;
  S a[2] = v;
  return a[1].n[1];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

