Texture2D<float4> x_20 : register(t0, space0);

void main_1() {
  int3 tint_tmp;
  x_20.GetDimensions(int(1u), tint_tmp.x, tint_tmp.y, tint_tmp.z);
  const uint2 x_125 = uint2(tint_tmp.xy);
  return;
}

void main() {
  main_1();
  return;
}
