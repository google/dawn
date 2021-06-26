void unpack2x16snorm_b4aea6() {
  int tint_tmp_1 = int(1u);
  int2 tint_tmp = int2(tint_tmp_1 << 16, tint_tmp_1) >> 16;
  float2 res = clamp(float2(tint_tmp) / 32767.0, -1.0, 1.0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  unpack2x16snorm_b4aea6();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  unpack2x16snorm_b4aea6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack2x16snorm_b4aea6();
  return;
}
