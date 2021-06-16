void clamp_2bd567() {
  float res = clamp(1.0f, 1.0f, 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  clamp_2bd567();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  clamp_2bd567();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_2bd567();
  return;
}
