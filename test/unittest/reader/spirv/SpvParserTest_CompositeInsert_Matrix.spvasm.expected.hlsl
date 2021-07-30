void main_1() {
  float3x2 x_35 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x2 x_2_1 = x_35;
  x_2_1[2u] = float2(50.0f, 60.0f);
  const float3x2 x_2 = x_2_1;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
