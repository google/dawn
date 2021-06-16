void countOneBits_0f7980() {
  int4 res = countbits(int4(0, 0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  countOneBits_0f7980();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  countOneBits_0f7980();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_0f7980();
  return;
}
