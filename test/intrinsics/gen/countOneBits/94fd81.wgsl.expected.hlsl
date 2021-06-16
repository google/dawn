void countOneBits_94fd81() {
  uint2 res = countbits(uint2(0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  countOneBits_94fd81();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  countOneBits_94fd81();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_94fd81();
  return;
}
