void reverseBits_222177() {
  int2 res = reversebits(int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reverseBits_222177();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reverseBits_222177();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_222177();
  return;
}
