void main_1() {
  const float3 x_1 = float3(50.0f, 60.0f, 70.0f);
  const float3 x_2 = ddx_fine(x_1);
  return;
}

void main() {
  main_1();
  return;
}
