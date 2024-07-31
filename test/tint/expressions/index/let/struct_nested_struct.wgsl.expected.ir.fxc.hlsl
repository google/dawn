struct T {
  float o;
  uint p;
};

struct S {
  int m;
  T n;
};


uint f() {
  S v = (S)0;
  S a = v;
  return a.n.p;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

