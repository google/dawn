void any_0e3e58() {
  bool res = any(bool2(false, false));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  any_0e3e58();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  any_0e3e58();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  any_0e3e58();
  return;
}
