void length_056071() {
  float res = length(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  length_056071();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  length_056071();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_056071();
  return;
}
