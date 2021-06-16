void transpose_4dc9a1() {
  float3x2 res = transpose(float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  transpose_4dc9a1();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  transpose_4dc9a1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  transpose_4dc9a1();
  return;
}
