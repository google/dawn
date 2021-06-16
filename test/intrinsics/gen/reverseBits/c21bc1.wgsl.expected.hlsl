void reverseBits_c21bc1() {
  int3 res = reversebits(int3(0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reverseBits_c21bc1();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reverseBits_c21bc1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_c21bc1();
  return;
}
