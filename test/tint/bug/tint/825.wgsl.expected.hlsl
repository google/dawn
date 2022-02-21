[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  int i = 0;
  int j = 0;
  const float2x2 m = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  const float f_1 = m[i][j];
}
