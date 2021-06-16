void clamp_6c1749() {
  int2 res = clamp(int2(0, 0), int2(0, 0), int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  clamp_6c1749();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  clamp_6c1749();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_6c1749();
  return;
}
