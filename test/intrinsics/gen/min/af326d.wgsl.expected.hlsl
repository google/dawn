void min_af326d() {
  float res = min(1.0f, 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  min_af326d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  min_af326d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_af326d();
  return;
}
