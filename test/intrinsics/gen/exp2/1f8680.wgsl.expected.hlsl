void exp2_1f8680() {
  float3 res = exp2(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  exp2_1f8680();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  exp2_1f8680();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp2_1f8680();
  return;
}
