void select_1e960b() {
  uint2 res = (bool2(false, false) ? uint2(0u, 0u) : uint2(0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_1e960b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_1e960b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_1e960b();
  return;
}
