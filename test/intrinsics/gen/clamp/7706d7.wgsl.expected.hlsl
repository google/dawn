void clamp_7706d7() {
  uint2 res = clamp(uint2(0u, 0u), uint2(0u, 0u), uint2(0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  clamp_7706d7();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  clamp_7706d7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_7706d7();
  return;
}
