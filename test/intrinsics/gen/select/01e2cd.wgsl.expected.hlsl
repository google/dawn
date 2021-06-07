struct tint_symbol {
  float4 value : SV_Position;
};

void select_01e2cd() {
  int3 res = (vector<bool, 3>(false, false, false) ? int3(0, 0, 0) : int3(0, 0, 0));
}

tint_symbol vertex_main() {
  select_01e2cd();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_01e2cd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_01e2cd();
  return;
}

