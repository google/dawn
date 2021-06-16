void transpose_4ce359() {
  float4x2 res = transpose(float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  transpose_4ce359();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  transpose_4ce359();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_4ce359();
  return;
}
