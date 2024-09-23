struct S {
  int m;
  uint3 n;
};


uint f() {
  S v = (S)0;
  S a = v;
  return a.n[int(2)];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

