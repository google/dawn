void mix_4f0b5e() {
  float res = lerp(1.0f, 1.0f, 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  mix_4f0b5e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  mix_4f0b5e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_4f0b5e();
  return;
}
