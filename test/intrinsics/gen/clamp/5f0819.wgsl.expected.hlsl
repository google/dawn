void clamp_5f0819() {
  int3 res = clamp(int3(0, 0, 0), int3(0, 0, 0), int3(0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  clamp_5f0819();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  clamp_5f0819();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_5f0819();
  return;
}
