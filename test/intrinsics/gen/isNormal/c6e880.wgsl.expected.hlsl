bool tint_isNormal(float param_0) {
  uint exponent = asuint(param_0) & 0x7f80000;
  uint clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

void isNormal_c6e880() {
  bool res = tint_isNormal(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isNormal_c6e880();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isNormal_c6e880();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNormal_c6e880();
  return;
}
