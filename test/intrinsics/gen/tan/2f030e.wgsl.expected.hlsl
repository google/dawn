void tan_2f030e() {
  float res = tan(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  tan_2f030e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  tan_2f030e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_2f030e();
  return;
}
