void select_bb8aae() {
  float4 res = (bool4(false, false, false, false) ? float4(0.0f, 0.0f, 0.0f, 0.0f) : float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_bb8aae();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_bb8aae();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_bb8aae();
  return;
}
