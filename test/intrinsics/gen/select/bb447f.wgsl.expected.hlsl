void select_bb447f() {
  int2 res = (false ? int2(0, 0) : int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_bb447f();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_bb447f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_bb447f();
  return;
}
