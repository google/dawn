void transpose_31e37e() {
  float2x4 res = transpose(float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  transpose_31e37e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  transpose_31e37e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_31e37e();
  return;
}
