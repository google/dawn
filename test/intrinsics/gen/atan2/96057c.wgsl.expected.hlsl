void atan2_96057c() {
  float res = atan2(1.0f, 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  atan2_96057c();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  atan2_96057c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_96057c();
  return;
}
