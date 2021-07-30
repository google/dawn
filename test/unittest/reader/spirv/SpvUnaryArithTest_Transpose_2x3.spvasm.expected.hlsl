void main_1() {
  const float2x3 x_1 = float2x3(float3(50.0f, 60.0f, 70.0f), float3(60.0f, 70.0f, 50.0f));
  const float3x2 x_2 = transpose(x_1);
  return;
}

void main() {
  main_1();
  return;
}
