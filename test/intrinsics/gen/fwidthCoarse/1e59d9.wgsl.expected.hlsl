struct tint_symbol {
  float4 value : SV_Position;
};

void fwidthCoarse_1e59d9() {
  float3 res = fwidth(float3(0.0f, 0.0f, 0.0f));
}

tint_symbol vertex_main() {
  fwidthCoarse_1e59d9();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fwidthCoarse_1e59d9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthCoarse_1e59d9();
  return;
}

