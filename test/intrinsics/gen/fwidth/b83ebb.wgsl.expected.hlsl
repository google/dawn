struct tint_symbol {
  float4 value : SV_Position;
};

void fwidth_b83ebb() {
  float2 res = fwidth(float2(0.0f, 0.0f));
}

tint_symbol vertex_main() {
  fwidth_b83ebb();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fwidth_b83ebb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidth_b83ebb();
  return;
}

