void sqrt_f8c59a() {
  float3 res = sqrt(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  sqrt_f8c59a();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sqrt_f8c59a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_f8c59a();
  return;
}
