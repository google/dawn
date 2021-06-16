void sqrt_20c74e() {
  float res = sqrt(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  sqrt_20c74e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sqrt_20c74e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_20c74e();
  return;
}
