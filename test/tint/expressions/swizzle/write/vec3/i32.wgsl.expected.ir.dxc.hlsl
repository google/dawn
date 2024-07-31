struct S {
  int3 v;
};


static S P = (S)0;
void f() {
  P.v = int3(1, 2, 3);
  P.v[0u] = 1;
  P.v[1u] = 2;
  P.v[2u] = 3;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

