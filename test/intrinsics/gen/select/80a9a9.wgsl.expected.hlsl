void select_80a9a9() {
  bool3 res = (bool3(false, false, false) ? bool3(false, false, false) : bool3(false, false, false));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_80a9a9();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_80a9a9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_80a9a9();
  return;
}
