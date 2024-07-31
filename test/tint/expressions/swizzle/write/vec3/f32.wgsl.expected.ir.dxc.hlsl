struct S {
  float3 v;
};


static S P = (S)0;
void f() {
  P.v = float3(1.0f, 2.0f, 3.0f);
  P.v[0u] = 1.0f;
  P.v[1u] = 2.0f;
  P.v[2u] = 3.0f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

