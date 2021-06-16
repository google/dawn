void min_c70bb7() {
  uint3 res = min(uint3(0u, 0u, 0u), uint3(0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  min_c70bb7();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  min_c70bb7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_c70bb7();
  return;
}
