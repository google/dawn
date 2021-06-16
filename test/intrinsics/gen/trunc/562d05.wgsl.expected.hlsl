void trunc_562d05() {
  float3 res = trunc(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  trunc_562d05();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  trunc_562d05();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_562d05();
  return;
}
