void main_1() {
  float3x2 x_35 = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const float x_2 = x_35[2u].y;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
