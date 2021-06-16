void atan_331e6d() {
  float3 res = atan(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  atan_331e6d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  atan_331e6d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan_331e6d();
  return;
}
