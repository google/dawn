void select_266aff() {
  float2 res = (bool2(false, false) ? float2(0.0f, 0.0f) : float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_266aff();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_266aff();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_266aff();
  return;
}
