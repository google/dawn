void unpack2x16unorm_7699c0() {
  uint tint_tmp_1 = 1u;
  uint2 tint_tmp = uint2(tint_tmp_1 & 0xffff, tint_tmp_1 >> 16);
  float2 res = float2(tint_tmp) / 65535.0;
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
