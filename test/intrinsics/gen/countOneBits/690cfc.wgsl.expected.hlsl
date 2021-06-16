void countOneBits_690cfc() {
  uint3 res = countbits(uint3(0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  countOneBits_690cfc();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  countOneBits_690cfc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_690cfc();
  return;
}
