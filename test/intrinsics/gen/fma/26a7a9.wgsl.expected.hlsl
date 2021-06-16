void fma_26a7a9() {
  float2 res = mad(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fma_26a7a9();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fma_26a7a9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_26a7a9();
  return;
}
