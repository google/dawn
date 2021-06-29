void select_3c25ce() {
  bool3 res = (false ? bool3(false, false, false) : bool3(false, false, false));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_3c25ce();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_3c25ce();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_3c25ce();
  return;
}
