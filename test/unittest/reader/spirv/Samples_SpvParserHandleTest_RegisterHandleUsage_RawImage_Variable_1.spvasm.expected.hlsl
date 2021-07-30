RWTexture1D<float4> x_20 : register(u0, space0);

void main_1() {
  x_20[int(1u)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
  return;
}

void main() {
  main_1();
  return;
}
