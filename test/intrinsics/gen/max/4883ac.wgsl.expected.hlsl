void max_4883ac() {
  float3 res = max(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  max_4883ac();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  max_4883ac();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_4883ac();
  return;
}
