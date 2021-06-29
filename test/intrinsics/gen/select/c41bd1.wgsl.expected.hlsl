void select_c41bd1() {
  bool4 res = (false ? bool4(false, false, false, false) : bool4(false, false, false, false));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_c41bd1();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_c41bd1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_c41bd1();
  return;
}
