void main_1() {
  const uint2 x_1 = uint2(10u, 20u);
  const int2 x_2 = int2(30, 40);
  const float2 x_3 = float2(50.0f, 60.0f);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
