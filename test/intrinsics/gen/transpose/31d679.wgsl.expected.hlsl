void transpose_31d679() {
  float2x2 res = transpose(float2x2(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  transpose_31d679();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  transpose_31d679();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_31d679();
  return;
}
