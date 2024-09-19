struct S {
  float3 val[3];
};


void a() {
  int4 a_1 = (int(0)).xxxx;
  a_1[0u] = int(1);
  a_1[2u] = int(2);
  S d = (S)0;
  d.val[int(2)][1u] = 3.0f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

