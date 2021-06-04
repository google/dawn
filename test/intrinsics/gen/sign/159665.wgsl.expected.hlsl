struct tint_symbol {
  float4 value : SV_Position;
};

void sign_159665() {
  float3 res = sign(float3(0.0f, 0.0f, 0.0f));
}

tint_symbol vertex_main() {
  sign_159665();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sign_159665();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_159665();
  return;
}

