void determinant_e19305() {
  float res = determinant(float2x2(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  determinant_e19305();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  determinant_e19305();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  determinant_e19305();
  return;
}
