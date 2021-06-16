void countOneBits_65d2ae() {
  int3 res = countbits(int3(0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  countOneBits_65d2ae();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  countOneBits_65d2ae();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_65d2ae();
  return;
}
