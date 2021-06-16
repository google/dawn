void fract_8bc1e9() {
  float4 res = frac(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fract_8bc1e9();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fract_8bc1e9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_8bc1e9();
  return;
}
