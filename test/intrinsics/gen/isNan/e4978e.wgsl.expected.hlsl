void isNan_e4978e() {
  bool res = isnan(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isNan_e4978e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isNan_e4978e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_e4978e();
  return;
}
