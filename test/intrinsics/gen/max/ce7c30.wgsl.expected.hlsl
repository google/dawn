void max_ce7c30() {
  int res = max(1, 1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  max_ce7c30();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  max_ce7c30();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_ce7c30();
  return;
}
