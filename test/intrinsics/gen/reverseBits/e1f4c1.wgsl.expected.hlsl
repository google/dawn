void reverseBits_e1f4c1() {
  uint2 res = reversebits(uint2(0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reverseBits_e1f4c1();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reverseBits_e1f4c1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_e1f4c1();
  return;
}
