void fract_943cb1() {
  float2 res = frac(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fract_943cb1();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fract_943cb1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_943cb1();
  return;
}
