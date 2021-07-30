void main_1() {
  const float2x2 x_1 = float2x2(float2(50.0f, 60.0f), float2(60.0f, 50.0f));
  const float2x2 x_2 = transpose(x_1);
  return;
}

void main() {
  main_1();
  return;
}
