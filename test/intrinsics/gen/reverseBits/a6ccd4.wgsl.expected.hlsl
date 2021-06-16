void reverseBits_a6ccd4() {
  uint3 res = reversebits(uint3(0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reverseBits_a6ccd4();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reverseBits_a6ccd4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_a6ccd4();
  return;
}
