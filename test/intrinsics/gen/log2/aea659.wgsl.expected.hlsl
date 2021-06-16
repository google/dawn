void log2_aea659() {
  float2 res = log2(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  log2_aea659();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  log2_aea659();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log2_aea659();
  return;
}
