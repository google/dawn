void main_1() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  const float3 x_15 = m[1];
  const float x_16 = x_15.y;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
