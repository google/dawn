void normalize_64d8c0() {
  float3 res = normalize(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  normalize_64d8c0();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  normalize_64d8c0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  normalize_64d8c0();
  return;
}
