struct S {
  uint3 v;
};


static S P = (S)0;
void f() {
  P.v = uint3(1u, 2u, 3u);
  P.v[0u] = 1u;
  P.v[1u] = 2u;
  P.v[2u] = 3u;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

