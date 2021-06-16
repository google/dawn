void max_44a39d() {
  float res = max(1.0f, 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  max_44a39d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  max_44a39d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_44a39d();
  return;
}
