void select_ab069f() {
  int4 res = (false ? int4(0, 0, 0, 0) : int4(0, 0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_ab069f();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_ab069f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_ab069f();
  return;
}
