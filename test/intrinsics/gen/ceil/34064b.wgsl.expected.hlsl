void ceil_34064b() {
  float3 res = ceil(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ceil_34064b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ceil_34064b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_34064b();
  return;
}
