void cosh_e0c1de() {
  float4 res = cosh(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  cosh_e0c1de();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  cosh_e0c1de();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cosh_e0c1de();
  return;
}
