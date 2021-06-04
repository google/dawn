struct tint_symbol {
  float4 value : SV_Position;
};

void asin_c0c272() {
  float res = asin(1.0f);
}

tint_symbol vertex_main() {
  asin_c0c272();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  asin_c0c272();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_c0c272();
  return;
}

