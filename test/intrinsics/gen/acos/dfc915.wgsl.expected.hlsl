void acos_dfc915() {
  float2 res = acos(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  acos_dfc915();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  acos_dfc915();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_dfc915();
  return;
}
