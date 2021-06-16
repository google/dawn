void select_00b848() {
  int2 res = (bool2(false, false) ? int2(0, 0) : int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_00b848();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_00b848();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_00b848();
  return;
}
