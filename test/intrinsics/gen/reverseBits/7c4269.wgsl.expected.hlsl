void reverseBits_7c4269() {
  int res = reversebits(1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  reverseBits_7c4269();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  reverseBits_7c4269();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_7c4269();
  return;
}
