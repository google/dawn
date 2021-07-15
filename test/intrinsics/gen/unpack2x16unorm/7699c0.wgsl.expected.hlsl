float2 tint_unpack2x16unorm(uint param_0) {
  uint j = param_0;
  uint2 i = uint2(j & 0xffff, j >> 16);
  return float2(i) / 65535.0;
}

void unpack2x16unorm_7699c0() {
  float2 res = tint_unpack2x16unorm(1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  unpack2x16unorm_7699c0();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  unpack2x16unorm_7699c0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack2x16unorm_7699c0();
  return;
}
