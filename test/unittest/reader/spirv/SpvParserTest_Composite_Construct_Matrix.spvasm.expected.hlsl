void main_1() {
  const float3x2 x_1 = float3x2(float2(50.0f, 60.0f), float2(60.0f, 50.0f), float2(70.0f, 70.0f));
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
