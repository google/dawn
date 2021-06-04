struct tint_symbol {
  float4 value : SV_Position;
};

void fwidthFine_523fdc() {
  float3 res = fwidth(float3(0.0f, 0.0f, 0.0f));
}

tint_symbol vertex_main() {
  fwidthFine_523fdc();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fwidthFine_523fdc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthFine_523fdc();
  return;
}

