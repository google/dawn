void isFinite_34d32b() {
  bool2 res = isfinite(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isFinite_34d32b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isFinite_34d32b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_34d32b();
  return;
}
