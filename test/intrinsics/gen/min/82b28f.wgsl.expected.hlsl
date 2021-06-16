void min_82b28f() {
  uint2 res = min(uint2(0u, 0u), uint2(0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  min_82b28f();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  min_82b28f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_82b28f();
  return;
}
