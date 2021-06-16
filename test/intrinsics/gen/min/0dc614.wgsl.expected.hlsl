void min_0dc614() {
  uint4 res = min(uint4(0u, 0u, 0u, 0u), uint4(0u, 0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  min_0dc614();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  min_0dc614();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_0dc614();
  return;
}
