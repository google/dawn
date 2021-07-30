Texture2D<float4> x_20 : register(t0, space0);

void main_1() {
  int3 tint_tmp;
  x_20.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  const uint x_125 = uint(tint_tmp.z);
  return;
}

void main() {
  main_1();
  return;
}
