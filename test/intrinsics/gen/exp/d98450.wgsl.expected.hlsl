void exp_d98450() {
  float3 res = exp(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  exp_d98450();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  exp_d98450();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_d98450();
  return;
}
