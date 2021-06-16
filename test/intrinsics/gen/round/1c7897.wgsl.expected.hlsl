void round_1c7897() {
  float3 res = round(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  round_1c7897();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  round_1c7897();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_1c7897();
  return;
}
