void log2_902988() {
  float4 res = log2(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  log2_902988();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  log2_902988();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log2_902988();
  return;
}
