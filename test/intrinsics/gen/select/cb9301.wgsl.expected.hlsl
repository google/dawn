void select_cb9301() {
  bool2 res = (bool2(false, false) ? bool2(false, false) : bool2(false, false));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_cb9301();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_cb9301();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_cb9301();
  return;
}
