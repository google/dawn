void length_602a17() {
  float res = length(1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  length_602a17();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  length_602a17();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_602a17();
  return;
}
