warning: use of deprecated intrinsic
Texture1D<float4> x_20 : register(t0, space0);

void main_1() {
  const float4 x_125 = x_20.Load(int2(int(1u), 0));
  return;
}

void main() {
  main_1();
  return;
}
