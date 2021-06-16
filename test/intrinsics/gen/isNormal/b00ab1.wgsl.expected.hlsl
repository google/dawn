void isNormal_b00ab1() {
  uint2 tint_isnormal_exponent = asuint(float2(0.0f, 0.0f)) & 0x7f80000;
  uint2 tint_isnormal_clamped = clamp(tint_isnormal_exponent, 0x0080000, 0x7f00000);
  bool2 res = (tint_isnormal_clamped == tint_isnormal_exponent);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isNormal_b00ab1();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isNormal_b00ab1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNormal_b00ab1();
  return;
}
