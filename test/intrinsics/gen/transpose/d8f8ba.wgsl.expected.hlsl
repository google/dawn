void transpose_d8f8ba() {
  float4x3 res = transpose(float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  transpose_d8f8ba();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  transpose_d8f8ba();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_d8f8ba();
  return;
}
