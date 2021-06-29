void select_78be5f() {
  float3 res = (false ? float3(0.0f, 0.0f, 0.0f) : float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_78be5f();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_78be5f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_78be5f();
  return;
}
