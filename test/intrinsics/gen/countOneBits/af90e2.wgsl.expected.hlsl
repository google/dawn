void countOneBits_af90e2() {
  int2 res = countbits(int2(0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  countOneBits_af90e2();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  countOneBits_af90e2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_af90e2();
  return;
}
