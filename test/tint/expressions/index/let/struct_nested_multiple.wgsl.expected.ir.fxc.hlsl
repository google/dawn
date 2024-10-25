struct T {
  uint k[2];
};

struct S {
  int m;
  T n[4];
};


uint f() {
  S a = (S)0;
  return a.n[int(2)].k[int(1)];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

