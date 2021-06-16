void isFinite_8a23ad() {
  bool3 res = isfinite(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isFinite_8a23ad();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isFinite_8a23ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_8a23ad();
  return;
}
