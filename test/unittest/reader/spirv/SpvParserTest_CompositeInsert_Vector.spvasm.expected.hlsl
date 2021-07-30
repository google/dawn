void main_1() {
  float2 x_1_1 = float2(50.0f, 60.0f);
  x_1_1.y = 70.0f;
  const float2 x_1 = x_1_1;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
