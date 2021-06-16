void select_28a27e() {
  uint3 res = (bool3(false, false, false) ? uint3(0u, 0u, 0u) : uint3(0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_28a27e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_28a27e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_28a27e();
  return;
}
