void max_85e6bc() {
  int4 res = max(int4(0, 0, 0, 0), int4(0, 0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  max_85e6bc();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  max_85e6bc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_85e6bc();
  return;
}
