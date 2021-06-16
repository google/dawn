void fwidthFine_f1742d() {
  float res = fwidth(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fwidthFine_f1742d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fwidthFine_f1742d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthFine_f1742d();
  return;
}
