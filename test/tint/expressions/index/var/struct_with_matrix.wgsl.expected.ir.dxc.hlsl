struct S {
  int m;
  float4x4 n;
};


float f() {
  S a = (S)0;
  return a.n[int(2)].y;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

