void fract_fa5c71() {
  float res = frac(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fract_fa5c71();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fract_fa5c71();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_fa5c71();
  return;
}
