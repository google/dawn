void sinh_b9860e() {
  float2 res = sinh(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  sinh_b9860e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  sinh_b9860e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sinh_b9860e();
  return;
}
