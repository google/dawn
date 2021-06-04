struct tint_symbol {
  float4 value : SV_Position;
};

void fwidthCoarse_e653f7() {
  float2 res = fwidth(float2(0.0f, 0.0f));
}

tint_symbol vertex_main() {
  fwidthCoarse_e653f7();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fwidthCoarse_e653f7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthCoarse_e653f7();
  return;
}

