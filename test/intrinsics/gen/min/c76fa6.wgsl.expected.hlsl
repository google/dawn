void min_c76fa6() {
  float4 res = min(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  min_c76fa6();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  min_c76fa6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_c76fa6();
  return;
}
