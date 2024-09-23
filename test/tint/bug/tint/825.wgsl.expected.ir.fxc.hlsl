
void f() {
  int i = int(0);
  int j = int(0);
  float2x2 m = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  float f_1 = m[i][j];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

