float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

void unpack4x8unorm_750c74() {
  float4 res = tint_unpack4x8unorm(1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  unpack4x8unorm_750c74();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  unpack4x8unorm_750c74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4x8unorm_750c74();
  return;
}
