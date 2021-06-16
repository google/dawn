void cos_c3b486() {
  float2 res = cos(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  cos_c3b486();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  cos_c3b486();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cos_c3b486();
  return;
}
