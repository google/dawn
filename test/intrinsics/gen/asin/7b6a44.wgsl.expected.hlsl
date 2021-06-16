void asin_7b6a44() {
  float2 res = asin(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  asin_7b6a44();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  asin_7b6a44();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_7b6a44();
  return;
}
