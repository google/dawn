Texture2D<float4> x_20 : register(t0, space0);

void main_1() {
  int2 tint_tmp;
  x_20.GetDimensions(tint_tmp.x, tint_tmp.y);
  const uint2 x_125 = uint2(tint_tmp);
  return;
}

void main() {
  main_1();
  return;
}
