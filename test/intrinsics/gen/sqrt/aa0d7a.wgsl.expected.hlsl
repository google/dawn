void sqrt_aa0d7a() {
  float4 res = sqrt(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  sqrt_aa0d7a();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sqrt_aa0d7a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_aa0d7a();
  return;
}
