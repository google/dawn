struct tint_symbol {
  float4 value : SV_Position;
};

void select_e3e028() {
  vector<bool, 4> res = (vector<bool, 4>(false, false, false, false) ? vector<bool, 4>(false, false, false, false) : vector<bool, 4>(false, false, false, false));
}

tint_symbol vertex_main() {
  select_e3e028();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_e3e028();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_e3e028();
  return;
}

