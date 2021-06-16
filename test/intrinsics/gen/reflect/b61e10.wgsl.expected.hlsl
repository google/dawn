void reflect_b61e10() {
  float2 res = reflect(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reflect_b61e10();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reflect_b61e10();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_b61e10();
  return;
}
