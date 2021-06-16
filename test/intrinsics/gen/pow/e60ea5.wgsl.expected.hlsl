void pow_e60ea5() {
  float2 res = pow(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  pow_e60ea5();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  pow_e60ea5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pow_e60ea5();
  return;
}
