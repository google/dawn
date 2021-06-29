void select_bf3d29() {
  float2 res = (false ? float2(0.0f, 0.0f) : float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  select_bf3d29();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  select_bf3d29();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_bf3d29();
  return;
}
