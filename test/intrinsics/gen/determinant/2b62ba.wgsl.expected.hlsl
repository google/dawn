void determinant_2b62ba() {
  float res = determinant(float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  determinant_2b62ba();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  determinant_2b62ba();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  determinant_2b62ba();
  return;
}
