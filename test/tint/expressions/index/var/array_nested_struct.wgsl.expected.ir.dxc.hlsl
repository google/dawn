struct S {
  int m;
  uint n[4];
};


uint f() {
  S a[2] = (S[2])0;
  return a[int(1)].n[int(1)];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

