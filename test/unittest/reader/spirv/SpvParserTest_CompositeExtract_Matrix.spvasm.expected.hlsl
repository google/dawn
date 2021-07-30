void main_1() {
  float3x2 x_35 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const float2 x_2 = x_35[2u];
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
