void max_0c0aae() {
  uint res = max(1u, 1u);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  max_0c0aae();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  max_0c0aae();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_0c0aae();
  return;
}
